[VFS] seems like this is the end :)
[DEV] init device system
[FB] fb0dev is ready
[USER] found, load '/bin/hello.elf' <8392 bytes>
[ELF] loading 'hello.elf': type=2 machine=0x0x0000003e entry=0x40000000 phdrs=1
[VMM] new space PML4 frame=0x4fa000
[ELF] seg 0: vaddr=0x40000000 filesz=2644 memsz=2644 flags=RWX
[PAGING] new PDPT frame=0x4fc000
[PAGING] new PD frame=0x4fd000
[PAGING] new PT frame=0x4fe000
[PAGING] mapping virt=0x40000000 -> phys=0x4fb000
[ELF] seg 0 mapped va=0x40000000 pages=1
[PAGING] new PDPT frame=0x500000
[PAGING] new PD frame=0x501000
[PAGING] new PT frame=0x502000
[PAGING] mapping virt=0x7ffffffee000 -> phys=0x4ff000
...
[PAGING] mapping virt=0x7fffffffd000 -> phys=0x511000
[ELF] stack mapped: 0x7ffffffee000 - 0x7fffffffe000
[ELF] launched 'hello.elf' entry=0x40000000 stack_top=0x7fffffffe000
[ELF] process scheduled
[USER] loading was a success!
[USER] found, load '/bin/test_graphics.elf' <9560 bytes>
[ELF] loading 'test_graphics.elf': type=2 machine=0x0x0000003e entry=0x40000000 phdrs=1
[VMM] new space PML4 frame=0x512000
[ELF] seg 0: vaddr=0x40000000 filesz=3774 memsz=3774 flags=RWX
[PAGING] new PDPT frame=0x514000
[PAGING] new PD frame=0x515000
[PAGING] new PT frame=0x516000
[PAGING] mapping virt=0x40000000 -> phys=0x513000
[ELF] seg 0 mapped va=0x40000000 pages=1
[PAGING] new PDPT frame=0x518000
[PAGING] new PD frame=0x519000
[PAGING] new PT frame=0x51a000
[PAGING] mapping virt=0x7ffffffee000 -> phys=0x517000
...
[PAGING] mapping virt=0x7fffffffd000 -> phys=0x529000
[ELF] stack mapped: 0x7ffffffee000 - 0x7fffffffe000
[ELF] launched 'test_graphics.elf' entry=0x40000000 stack_top=0x7fffffffe000
[ELF] process scheduled
[USER] loading was a success!

hello, world!, call me userspace cuz its what i am!!
 running test_graphics
 test_graphics: width=1280 height=800 pitch=5120 bpp=32 size=4096000
[PAGING] new PDPT frame=0x4fa000
[PAGING] new PD frame=0x4fb000
[PAGING] new PT frame=0x4fc000
[PAGING] mapping virt=0x600000000000 -> phys=0x112000
[PAGING] mapping virt=0x600000001000 -> phys=0x113000
...
[PAGING] mapping virt=0x6000001ff000 -> phys=0x311000
[PAGING] new PT frame=0x4fd000
[PAGING] mapping virt=0x600000200000 -> phys=0x312000
[PAGING] mapping virt=0x600000201000 -> phys=0x313000
...
[PAGING] mapping virt=0x6000003e6000 -> phys=0x4f8000
[PAGING] mapping virt=0x6000003e7000 -> phys=0x4f9000
 framebuffer mapped at 0x600000000000
 frame drawn, sit back and enjoy the awesome graphic x3

!!! PANIC !!!
Exception: Page Fault
INT: 14 ERR: 0
CR2 (faulting addr): 0x9D0
  present: no (unmapped), write: no
RIP: 0xFFFFFFFF80001679
RSP: 0xFFFF880000020A48

System halted.