#include <unistd.h>
#include <stdio.h>
#include <sys/fb.h>

static void draw_rect(
	uint32_t *fb,
	uint32_t stride,
	int x,
	int y,
	int w,
	int h,
	uint32_t color
) {
    for (int dy = 0; dy < h; dy++)
    {
        for (int dx = 0; dx < w; dx++)
        {
            fb[(y + dy) * stride + (x + dx)] = color;
        }
    }
}

int main(void)
{
    printf(" running test_graphics\n");

    long fd = open("/dev/fb0", 0);
    if (fd < 0)
    {
        printf(" test_graphics: open failed, no CAP_FRAMEBUFFER or fb0 not ready\n");
        _exit(1);
    }

    fb_info_t info;
    if (ioctl((int)fd, FB_IOCTL_GET_INFO, &info) < 0)
    {
        printf(" FB_IOCTL_GET_INFO failed\n");
        _exit(1);
    }

    printf(
        " test_graphics: width=%d height=%d pitch=%d bpp=%d size=%u\n",
        (int)info.width,
        (int)info.height,
        (int)info.pitch,
        (int)info.bpp,
        (unsigned int)info.size
    );

    unsigned long vaddr = 0;
    if (ioctl((int)fd, FB_IOCTL_MAP, &vaddr) < 0)
    {
        printf(" FB_IOCTL_MAP failed\n");
        _exit(1);
    }

    printf(" framebuffer mapped at %p\n", (void *)vaddr);

    uint32_t *fb = (uint32_t *)vaddr;
    uint32_t  stride = info.pitch / 4;

    draw_rect(fb, stride, 40,  40, 200, 150, 0xFFFF0000);
    draw_rect(fb, stride, 260, 40, 200, 150, 0xFF00FF00);
    draw_rect(fb, stride, 480, 40, 200, 150, 0xFF0000FF);

    // little gradient strip under the rectangles, nothing fancy
    // TODO:
    // add it in eXui
    for (uint32_t x = 0; x < info.width && x < 256; x++)
    {
        uint32_t shade = (x * 255) / 256;
        uint32_t color = 0xFF000000 | (shade << 16) | (shade << 8) | shade;

        for (uint32_t y = 220; y < 260; y++)
        {
            fb[y * stride + x + 40] = color;
        }
    }

    fb[300 * stride + 40]     = 0xFFFFFFFF;
    fb[300 * stride + 41]     = 0xFFFF00FF;
    fb[300 * stride + 42]     = 0xFF00FFFF;
    fb[300 * stride + 43]     = 0xFFFFFF00;

    if (ioctl((int)fd, FB_IOCTL_FLUSH, NULL) < 0)
    {
        printf(" FB_IOCTL_FLUSH failed\n");
    }

    printf(" frame drawn, sit back and enjoy the awesome graphic x3\n");

    //return 0;
    //_exit(0);
    for (;;)
    {
        yield();
    }
}