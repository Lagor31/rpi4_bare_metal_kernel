#ifndef SMP_H
#define SMP_H

void store32(unsigned long address, unsigned long value);
void store64(unsigned long address, unsigned long long value);

unsigned long load32(unsigned long address);
unsigned long load64(unsigned long address);

extern "C" unsigned long spin_cpu0;
extern "C" unsigned long spin_cpu1;
extern "C" unsigned long spin_cpu2;
extern "C" unsigned long spin_cpu3; 

extern "C" void _start_core1();
void start_core1(void (*func)(void));
void start_core2(void (*func)(void));
void start_core3(void (*func)(void));
void clear_core1(void);
void clear_core2(void);
void clear_core3(void);
void print_core_id();
#endif