extern "C" void kernel_mmu_init() __attribute__((section(".boot")));
extern "C" void kernel_main();

void kernel_mmu_init() { kernel_main(); }
