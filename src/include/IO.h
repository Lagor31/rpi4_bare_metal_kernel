void uart_init();
void uart_writeText(char *buffer);
void uart_loadOutputFifo();
unsigned char uart_readByte();
unsigned int uart_isReadByteReady();
void uart_writeByteBlocking(unsigned char ch);
void uart_update();
void mmio_write(long reg, unsigned int val);
unsigned int mmio_read(long reg);

#define RPIQ_MEM_BARRIER() asm volatile("dsb sy" ::: "memory")
#define RPIQ_INVAL_DCACHE(_va) asm volatile("dc ivac, %0" ::"r"(_va))
#define RPIQ_DMA_CLEAN_DCACHE(_va) asm volatile("dc civac, %0" ::"r"(_va))
