/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: pci.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "pci.h"
#include "device.h"
#include "config.h"
#include "express.h"
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/print.h>


void pci_init(void)
{
    char buf[64];
    buf[0] = '\0'; // prevents it from random character drawing
// before pci_get_device_count();
// because in the kernel its used before this causes random character drawing

    log("[PCI]", "Init PCI/PCIe\n", d);

    pci_device_init();


    int total_count = pci_device_get_count();
    int pcie_count = 0;

    // count PCIe devices
    for (int i = 0; i < total_count; i++)
    {
        pci_device_t *dev = pci_device_get(i);
        if (dev && pcie_is_device(dev->bus, dev->device, dev->function)) {
            pcie_count++;
        }
    }

    int pci_count = total_count - pcie_count;

    str_copy(buf, "found ");
    str_append_uint(buf, total_count);
    str_append(buf, " total device(s)\n");
    log("[PCI]", buf); // first log with tag

    str_copy(buf, "found ");
    str_append_uint(buf, pcie_count);
    str_append(buf, " PCIe device(s)\n");
    log_continue(buf);

    str_copy(buf, "found ");
    str_append_uint(buf, pci_count);
    str_append(buf, " PCI device(s)\n");
    log_continue(buf);

}

int pci_get_device_count(void) {
    return pci_device_get_count();
}

pci_device_t* pci_get_device(int index) {
    return pci_device_get(index);
}
