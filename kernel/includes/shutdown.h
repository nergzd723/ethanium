//nasm shutdown: disable interrupts, paging, a20, far jump to real mode, use bios 0x15 interrupt to shutdown
extern void shutdownasm();
void shutdown();
void reboot();
void shutdown_qemu_vm();
