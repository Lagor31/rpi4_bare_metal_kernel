#ifndef MMU_H
#define MMU_H

#define LINEAR_MAP_BASE 0xffff000000000000
/* translation table descriptors */
#define TD_VALID                   (1 << 0)
#define TD_BLOCK                   (0 << 1)
#define TD_PAGE                    (1 << 1)
#define TD_TABLE                   (1 << 1)
/* 
 * we can check ID_AA64MMFR1_EL1.HAFDBS to see if the hardware supports hw management
 * of the access flag and dirty state, but we'll just set the access flag to 1 by sw. 
 */
#define TD_ACCESS                  (1 << 10)
/* D5-2739 */
/* EL1 rwx, EL0 - */
#define TD_KERNEL_PERMS            (1 << 54)
/* EL1 rw, EL0 rwx */
#define TD_USER_PERMS              (1 << 6)
/*
 * memory region shared by all cores, but this has no effect now because non-cachable and
 * device memory are always outer sharable
 * */
#define TD_OUTER_SHARABLE          (3 << 8)

#define TD_KERNEL_TABLE_FLAGS      (TD_TABLE | TD_VALID)
#define TD_KERNEL_BLOCK_FLAGS      (TD_ACCESS | TD_OUTER_SHARABLE | TD_KERNEL_PERMS | (MATTR_NORMAL_NC_INDEX << 2) | TD_BLOCK | TD_VALID)
#define TD_DEVICE_BLOCK_FLAGS      (TD_ACCESS | TD_OUTER_SHARABLE | TD_KERNEL_PERMS | (MATTR_DEVICE_nGnRnE_INDEX << 2) | TD_BLOCK | TD_VALID)
#define TD_USER_TABLE_FLAGS        (TD_TABLE | TD_VALID)
#define TD_USER_PAGE_FLAGS         (TD_ACCESS | TD_OUTER_SHARABLE | TD_USER_PERMS | (MATTR_NORMAL_NC_INDEX << 2) | TD_PAGE | TD_VALID)

/* memory attribute indirect register */
#define MATTR_DEVICE_nGnRnE        0x0
#define MATTR_NORMAL_NC            0xFF
#define MATTR_DEVICE_nGnRnE_INDEX  0
#define MATTR_NORMAL_NC_INDEX      1
#define MAIR_EL1_VAL               ((MATTR_NORMAL_NC << (8 * MATTR_NORMAL_NC_INDEX)) | MATTR_DEVICE_nGnRnE << (8 * MATTR_DEVICE_nGnRnE_INDEX))

/* translation control register */
#define TCR_TG1_4K     (2 << 30)
#define TCR_T1SZ       ((64 - 48) << 16)
#define TCR_TG0_4K     (0 << 14)
#define TCR_T0SZ       (64 - 48)
#define TCR_EL1_VAL    (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ)


#define PAGE_MASK  0xfffffffffffff000
#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define ID_MAP_PAGES           3
#define HIGH_MAP_PAGES         6
#define ID_MAP_TABLE_SIZE      (ID_MAP_PAGES * PAGE_SIZE)
#define HIGH_MAP_TABLE_SIZE    (HIGH_MAP_PAGES * PAGE_SIZE)
#define ENTRIES_PER_TABLE      512
#define PGD_SHIFT              (PAGE_SHIFT + 3 * TABLE_SHIFT)
#define PUD_SHIFT              (PAGE_SHIFT + 2 * TABLE_SHIFT)
#define PMD_SHIFT              (PAGE_SHIFT + TABLE_SHIFT)
#define PUD_ENTRY_MAP_SIZE     (1 << PUD_SHIFT)
#define ID_MAP_SIZE            (8 * SECTION_SIZE)

#define HIGH_MAP_FIRST_START   (0x0 + LINEAR_MAP_BASE)
#define HIGH_MAP_FIRST_END     (0x3B400000 + LINEAR_MAP_BASE)
#define HIGH_MAP_SECOND_START  (0x40000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_SECOND_END    (0x80000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_THIRD_START   (0x80000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_THIRD_END     (0xC0000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_FOURTH_START  (0xC0000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_FOURTH_END    (0xFC000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_DEVICE_START  (0xFC000000 + LINEAR_MAP_BASE)
#define HIGH_MAP_DEVICE_END    (0x100000000 + LINEAR_MAP_BASE)

#define FIRST_START   (0x0)
#define FIRST_END     (0x3B400000)
#define SECOND_START  (0x40000000)
#define SECOND_END    (0x80000000)
#define THIRD_START   (0x80000000)
#define THIRD_END     (0xC0000000)
#define FOURTH_START  (0xC0000000)
#define FOURTH_END    (0xFC000000)
#define DEVICE_START  (0xFC000000)
#define DEVICE_END    (0x100000000)

/*
  Descriptor format
`+------------------------------------------------------------------------------------------+
 | Upper attributes | Address (bits 47:12) | Lower attributes | Block/table bit | Valid bit |
 +------------------------------------------------------------------------------------------+
 63                 47                     11                 2                 1           0


*/

#endif