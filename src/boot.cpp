#include "include/Mem.h"
#include "include/Mmu.h"
#include "include/Stdlib.h"

extern "C" void kernel_mmu_init() __attribute__((section(".boot")));
extern "C" void kernel_main();
extern "C" void setup_page_directory();


typedef struct mmu_desc_t {
    unsigned int upper_attr: 16;
    unsigned long address: 36;
    unsigned int lower_attr: 9;
    unsigned char block_table: 1;
    unsigned char valid: 1;
} __attribute__((packed)) MMUDescriptor;

/* 
// Reserved in assembly in .boot section so they have PHYSICAL ADDRESSES
__attribute__((section(".boot_data"))) MMUDescriptor page_global_dir[512];
__attribute__((section(".boot_data"))) MMUDescriptor page_upper_dir[512];
__attribute__((section(".boot_data"))) MMUDescriptor page_middle_dir[512];
__attribute__((section(".boot_data"))) MMUDescriptor pud_entry_0;
 */

// Working with PHYSYCAL Addresses
void kernel_mmu_init() {
  kernel_main();
}
