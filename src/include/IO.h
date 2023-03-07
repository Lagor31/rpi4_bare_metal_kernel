void mmio_write(long reg, unsigned int val);
unsigned int mmio_read(long reg);

#define RPIQ_MEM_BARRIER() asm volatile("dsb sy" ::: "memory")
#define RPIQ_INVAL_DCACHE(_va) asm volatile("dc ivac, %0" ::"r"(_va))
#define RPIQ_DMA_CLEAN_DCACHE(_va) asm volatile("dc civac, %0" ::"r"(_va))
