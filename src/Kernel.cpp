#include "include/BootAllocator.h"
#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/Gpio.h"
#include "include/IRQ.h"
#include "include/KernelHeapAllocator.h"
#include "include/Lists/ArrayList.hpp"
#include "include/Mem.h"
#include "include/Mmu.h"
#include "include/RedFS.h"
#include "include/SMP.h"
#include "include/Spinlock.h"
#include "include/Stdlib.h"
#include "include/String.h"
#include "include/SystemTimer.h"
#include "include/Task.h"
#include "include/Uart.h"
#include "include/Vector.h"
#include "include/buddy_alloc.h"
#include "include/emmc.h"

#define PACKED __attribute((__packed__))

#define BUDDY_ALLOC_IMPLEMENTATION

using SD::Lists::SinglyLinkedList;

extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;

extern "C" void init_core();

#define BOOT_SIGNATURE 0xAA55

typedef struct PACKED {
  uint8_t head;
  uint8_t sector : 6;
  uint8_t cylinder_hi : 2;
  uint8_t cylinder_lo;
} chs_address;

typedef struct PACKED {
  uint8_t status;
  chs_address first_sector;
  uint8_t type;
  chs_address last_sector;
  uint32_t first_lba_sector;
  uint32_t num_sectors;
} partition_entry;

typedef struct PACKED {
  uint8_t bootCode[0x1BE];
  partition_entry partitions[4];
  uint16_t bootSignature;
} master_boot_record;

extern "C" void kernel_main() {
  Core::disableIRQ();

  BootAllocator boot_allocator = BootAllocator(
      (unsigned char *)&_boot_alloc_start, (unsigned char *)&_boot_alloc_end);

  GlobalKernelAlloc::setAllocator(&boot_allocator);

  // We can use new with the boot allocator
  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  GIC400 *gic = new GIC400();
  SystemTimer *timer = new SystemTimer();
  DriverManager::load(gpio);
  DriverManager::load(uart);
  DriverManager::load(gic);
  DriverManager::load(timer);

  DriverManager::startAll();

  Console::setKernelConsole(uart);
  Console::print("\n\n\n\n\n\n############################################\n");
  Console::print("Current EL: %u\n", Std::getCurrentEL());

  Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                 &_boot_alloc_start, &_boot_alloc_end);
  Console::print("Heap Start: 0x%x Heap end: 0x%x\n",
                 (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  GlobalKernelAlloc::setAllocator(kha);
  Console::print("List of loaded drivers:\n");
  for (auto d : *DriverManager::getAll())
    Console::print("Driver %s\n", d->getName());

  initSchedLock();

  Console::print("Timer init on core: %d\n", get_core());
  Console::print("############################################\n");

  if (!emmc_init(gpio)) {
    Console::print("FAILED TO INIT EMMC\n");
    Core::panic("FAILED TO INIT EMMC\n");
  }

  master_boot_record mbr;
  emmc_read((uint8_t *)&mbr, sizeof(mbr));

  if (mbr.bootSignature != BOOT_SIGNATURE) {
    Console::print("BAD BOOT SIGNATURE: %X\n", mbr.bootSignature);
  }
  int lagorPLBAStart = 0;
  for (int i = 0; i < 4; i++) {
    if (mbr.partitions[i].type == 0) {
      break;
    }

    Console::print("Partition %d:\n", i);
    Console::print("\t Type: %x\n", mbr.partitions[i].type);
    Console::print("\t NumSecs: %d\n", mbr.partitions[i].num_sectors);
    Console::print("\t Status: %d\n", mbr.partitions[i].status);
    Console::print("\t Start: %d\n", mbr.partitions[i].first_lba_sector);
    if (mbr.partitions[i].type == 0x31) {
      Console::print("Found Lagor 0x31 partition starting at LBA: %d\n",
                     mbr.partitions[i].first_lba_sector);
      lagorPLBAStart = mbr.partitions[i].first_lba_sector;
      emmc_seek(lagorPLBAStart * 512);
    }
  }

  emmc_seek(lagorPLBAStart * 512);
  SuperBlock *sb = (SuperBlock *)GlobalKernelAlloc::alloc(512);
  emmc_read((uint8_t *)sb, 512);
  if (sb->signature[0] != 'L' && sb->signature[1] != 'a' &&
      sb->signature[2] != 'g' && sb->signature[3] != 'o' &&
      sb->signature[4] != 'r')
    Core::panic("Invalid RedFS Partition signature!");

  Console::print(
      "Tot Pools: %d\nINodes Tot: %d, INode Size: %d, INodes in Pool: %d, "
      "INodes Free: %d\n",
      sb->total_pools, sb->inodes_total, sb->inode_size, sb->inodes_in_pool,
      sb->inodes_free);

  Console::print(
      "Blocks Tot: %d, Block Size: %d, Blocks in Pool: %d, "
      "Blocks Free: %d\n",
      sb->blocks_total, sb->block_size, sb->blocks_in_pool, sb->blocks_free);
  // emmc_write((uint8_t *)&mbr, sizeof(mbr));
  emmc_seek(lagorPLBAStart * 512 + 512);

  Console::print("Inode struct size: %d\n", sizeof(INode));
  // Reading INode 0 (root)
  INode *rootI = (INode *)GlobalKernelAlloc::alloc(sizeof(INode) * 4);
  emmc_read((uint8_t *)rootI, 512);

  if (rootI->signature != 0xEEEE) Core::panic("Wrong root INode signature!");
  uint32_t inode_table_offset = lagorPLBAStart * 512 + 512;

  emmc_seek(inode_table_offset);
  for (int p = 0; p < sb->total_pools; ++p) {
    for (int i = 0; i < sb->inodes_in_pool / 4; ++i) {
      emmc_read((uint8_t *)rootI, 512);
      for (int c = 0; c < 4; ++c) {
        if (rootI[c].signature == 0xEEEE)
          Console::print("Found INode %d Perm: 0x%x Block0: %d\n", (i * 4) + c,
                         rootI[c].permissions, rootI[c].block0);
      }
      inode_table_offset += 512;
      emmc_seek(inode_table_offset);
    }
    inode_table_offset = lagorPLBAStart * 512 + 512 +
                         ((p + 1) * (sb->inodes_in_pool * sb->inode_size +
                                     sb->blocks_in_pool * sb->block_size));
    emmc_seek(inode_table_offset);
  }

  Core::scheduler = new Spinlock();
  Core::runningQ[get_core()] = new SinglyLinkedList<Task *>();
  Core::sleepingQ[get_core()] = new SinglyLinkedList<Task *>();
  Task *idle = Task::createKernelTask((uint64_t)&idleTask);
  Core::runningQ[get_core()]->insert(idle);

  Task *n;
  for (int i = 0; i < THREAD_N; ++i) {
    Task *t = Task::createKernelTask((uint64_t)&kernelTask);
    Core::runningQ[get_core()]->insert(t);
    if (i == 0) n = t;
  }

  // Core::spinms(1000);
  Core::start(1, &init_core);
  // Core::spinms(1000);
  Core::start(2, &init_core);
  // Core::spinms(1000);
  Core::start(3, &init_core);
  // Core::spinms(1000);

  Core::current[get_core()] = new Task();
  //  SystemTimer::WaitMicroT3(300000);
  Core::enableIRQ();
  SystemTimer::WaitMicroT1(100000);
  //  Core::switchTo(n);

  _hang_forever();
}
