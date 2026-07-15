/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: fb0.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "fb0.h"
#include "../names.h"

#include <kernel/screen/graphics.h>
#include <kernel/screen/bootscreen/boot.h>
#include <kernel/screen/lib/log.h>
#include <kernel/proc/process.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/mem/kheap/kheap.h>
#include <kernel/mem/lib.h>
#include <kernel/mem/phys/physmem.h>

#define FB_MAP_VADDR 0x0000600000000000ULL

typedef struct fb_handle
{
    proc_t *owner;
    u8     mapped;
    u64    vaddr;
} fb_handle_t;

static int fb0_dev_init(void)
{
    if (
    	!bs.Screens[BS3].pixels ||
     	!bs.Screens[BS3].pixels_phys
    ){
        log(
        	"[FB]",
         	"fb0 init skipped, BS3 backbuffer not ready yet\n",
          	warning
        );
        return -1;
    }

    log("[FB]", "fb0dev is ready\n");
    return 0;
}

static void *fb0_open(const char *path)
{
    (void)path;

    // no backbuffer, no device,
    //
    if (!bs.Screens[BS3].pixels || !bs.Screens[BS3].pixels_phys)
    {
        log("[FB]", "fb0 open rejected, BS3 backbuffer not ready\n", warning);
        return NULL;
    }

    proc_t *p = process_get_current();
    if (!p || !process_has_cap(p, CAP_FRAMEBUFFER))
    {
        log("[FB]", "fb0 open rejected, process lacks CAP_FRAMEBUFFER\n", warning);
        return NULL;
    }

    fb_handle_t *h = (fb_handle_t *)kmalloc(sizeof(fb_handle_t));
    if (!h) return NULL;

    h->owner      = p;
    h->mapped     = 0;
    h->vaddr      = 0;

    return h;
}

static void fb0_close(void *handle)
{
    fb_handle_t *h = (fb_handle_t *)handle;
    if (!h) return;

    // dont leave the mapping aftr close
    if (h->mapped  && h->owner && h->owner->space)
    {
        vmm_unmap_phys(h->owner->space, h->vaddr);
    }

    kfree((u64 *)h);
}

static i64 fb0_ioctl(void *handle, u64 request, void *arg)
{
    fb_handle_t *h = (fb_handle_t *)handle;

    if (!h) return -1;

    switch (request)
    {
        case FB_IOCTL_GET_WIDTH:    return (i64)get_fb_width();
        case FB_IOCTL_GET_HEIGHT:   return (i64)get_fb_height();
        case FB_IOCTL_GET_PITCH:    return (i64)get_fb_pitch();
        case FB_IOCTL_GET_BPP:      return (i64)get_fb_bpp();
        case FB_IOCTL_GET_SIZE:     return (i64)get_fb_size();
        case FB_IOCTL_GET_INFO:
        {
            if (!arg) return -1;

            fb_info_t info;
            info.width     = get_fb_width();
            info.height    = get_fb_height();
            info.pitch     = get_fb_pitch();
            info.bpp  = get_fb_bpp();
            info.size = get_fb_size();

            memcpy(arg, &info, sizeof(fb_info_t));

            return 0;
        }

        case FB_IOCTL_MAP:
        {
            if (!arg) return -1;
            if (!h->owner || !h->owner->space) return -1;

            //if already mapped
            // just hand back the same vaddr instead of mapping twive
            if (h->mapped)
            {
                *(u64 *)arg = h->vaddr;

                return 0;
            }

            u64 phys = bs.Screens[BS3].pixels_phys;
            if (!phys) return -1;

            u64 size       = get_fb_pitch() * get_fb_height();
            u64 page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

            u64 vaddr = vmm_map_phys(
                h->owner->space,
                FB_MAP_VADDR,
                phys,
                page_count,
                VMM_REGION_USER | VMM_REGION_READ | VMM_REGION_WRITE
            );

            if (!vaddr) return -1;

            h->mapped = 1;
            h->vaddr  = vaddr;

            *(u64 *)arg = vaddr;
            return 0;
        }

        case FB_IOCTL_UNMAP:
        {
            if (!h->mapped) return 0;
            if (!h->owner || !h->owner->space) return -1;

            vmm_unmap_phys(h->owner->space, h->vaddr);

            h->mapped = 0;
            h->vaddr  = 0;
            return 0;
        }

        case FB_IOCTL_FLUSH:
            bs.Flush(BS3);
            return 0;

        case FB_IOCTL_FLUSH_RECT:
        {
            if (!arg) return -1;
            fb_rect_t rect = *(fb_rect_t *)arg;
            bs_screen_t *scr = &bs.Screens[BS3];
            u32 *dst = get_framebuffer();
            if (!dst || !scr->pixels || rect.x >= scr->width || rect.y >= scr->height)
                return -1;
            if (rect.width > scr->width - rect.x) rect.width = scr->width - rect.x;
            if (rect.height > scr->height - rect.y) rect.height = scr->height - rect.y;
            u32 dst_stride = get_fb_pitch() / sizeof(u32);
            for (u32 y = 0; y < rect.height; y++)
                memcpy(&dst[(rect.y + y) * dst_stride + rect.x],
                       &scr->pixels[(rect.y + y) * scr->width + rect.x],
                       rect.width * sizeof(u32));
            return 0;
        }

        default:
            return -1;
    }
}

device_handler fb0_device =
{
    .name    = FB0_NAME,
    .mount   = FB0_MOUNT,
    .version = FB0_VERSION,
    .init    = fb0_dev_init,
    .fini    = NULL,
    .open    = fb0_open,
    .close   = fb0_close,
    .read    = NULL,
    .write   = NULL,
    .ioctl   = fb0_ioctl,
};
