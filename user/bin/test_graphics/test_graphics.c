#include <unistd.h>
#include <stdio.h>
#include <sys/fb.h>
#include <sys/input.h>

#define CURSOR_W 14
#define CURSOR_H 20

static const char cursor_shape[CURSOR_H][CURSOR_W + 1] = {
    "#.............",
    "##............",
    "#+#...........",
    "#++#..........",
    "#+++#.........",
    "#++++#........",
    "#+++++#.......",
    "#++++++#......",
    "#+++++++#.....",
    "#++++++++#....",
    "#++++#####....",
    "#++#+#........",
    "#+#.+#........",
    "##..+#........",
    "#....+#.......",
    ".....+#.......",
    ".....+#.......",
    "......+#......",
    "......##......",
    "..............",
};

static void clear_screen(uint32_t *fb, uint32_t stride, uint32_t width, uint32_t height)
{
    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
            fb[y * stride + x] = 0xFF000000;
}

static void draw_rect(
    uint32_t *fb, uint32_t stride, int x, int y, int width, int height, uint32_t color
) {
    for (int dy = 0; dy < height; dy++)
        for (int dx = 0; dx < width; dx++)
            fb[(y + dy) * stride + x + dx] = color;
}

static void cursor_restore(
    uint32_t *fb,
    uint32_t stride,
    int x,
    int y,
    const uint32_t *saved
) {
    for (int py = 0; py < CURSOR_H; py++)
        for (int px = 0; px < CURSOR_W; px++)
            fb[(y + py) * stride + x + px] = saved[py * CURSOR_W + px];
}

static void cursor_draw(
    uint32_t *fb,
    uint32_t stride,
    int x,
    int y,
    uint32_t *saved
) {
    for (int py = 0; py < CURSOR_H; py++) {
        for (int px = 0; px < CURSOR_W; px++) {
            uint32_t *pixel = &fb[(y + py) * stride + x + px];
            saved[py * CURSOR_W + px] = *pixel;
            if (cursor_shape[py][px] == (char)35) *pixel = 0xFF000000;
            if (cursor_shape[py][px] == (char)43) *pixel = 0xFFFFFFFF;
        }
    }
}

int main(void)
{
    long fb_fd = open("/dev/fb0", 0);
    if (fb_fd < 0) {
        printf(" cursor: framebuffer open failed\n");
        _exit(1);
    }

    fb_info_t info;
    if (ioctl((int)fb_fd, FB_IOCTL_GET_INFO, &info) < 0) {
        printf(" cursor: framebuffer info failed\n");
        _exit(1);
    }

    unsigned long vaddr = 0;
    if (ioctl((int)fb_fd, FB_IOCTL_MAP, &vaddr) < 0) {
        printf(" cursor: framebuffer map failed\n");
        _exit(1);
    }

    long mouse_fd = open("/dev/mouse", 0);
    if (mouse_fd < 0) {
        printf(" cursor: mouse open failed\n");
        _exit(1);
    }

    uint32_t *fb = (uint32_t *)vaddr;
    uint32_t stride = info.pitch / 4;
    clear_screen(fb, stride, info.width, info.height);

    draw_rect(fb, stride, 40, 40, 200, 150, 0xFFFF0000);
    draw_rect(fb, stride, 260, 40, 200, 150, 0xFF00FF00);
    draw_rect(fb, stride, 480, 40, 200, 150, 0xFF0000FF);

    for (uint32_t x = 0; x < info.width && x < 256; x++) {
        uint32_t shade = (x * 255) / 256;
        uint32_t color = 0xFF000000 | (shade << 16) | (shade << 8) | shade;
        for (uint32_t y = 220; y < 260; y++)
            fb[y * stride + x + 40] = color;
    }

    fb[300 * stride + 40] = 0xFFFFFFFF;
    fb[300 * stride + 41] = 0xFFFF00FF;
    fb[300 * stride + 42] = 0xFF00FFFF;
    fb[300 * stride + 43] = 0xFFFFFF00;

    int mouse_x = (int)info.width / 2;
    int mouse_y = (int)info.height / 2;
    uint32_t cursor_saved[CURSOR_W * CURSOR_H];
    cursor_draw(fb, stride, mouse_x, mouse_y, cursor_saved);
    ioctl((int)fb_fd, FB_IOCTL_FLUSH, NULL);

    for (;;) {
        input_event_t events[16];
        long count = read((int)mouse_fd, events, sizeof(events));
        int changed = 0;
        int old_x = mouse_x;
        int old_y = mouse_y;

        for (long i = 0; i < count / (long)sizeof(input_event_t); i++) {
            input_event_t *ev = &events[i];
            if (ev->type != INPUT_EV_REL) continue;
            if (ev->code == INPUT_REL_X) mouse_x += ev->value;
            if (ev->code == INPUT_REL_Y) mouse_y += ev->value;
            changed = 1;
        }

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > (int)info.width - CURSOR_W)
            mouse_x = (int)info.width - CURSOR_W;
        if (mouse_y > (int)info.height - CURSOR_H)
            mouse_y = (int)info.height - CURSOR_H;

        if (changed) {
            cursor_restore(fb, stride, old_x, old_y, cursor_saved);
            cursor_draw(fb, stride, mouse_x, mouse_y, cursor_saved);
            ioctl((int)fb_fd, FB_IOCTL_FLUSH, NULL);
        }
        yield();
    }
}
