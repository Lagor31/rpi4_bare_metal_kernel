#ifndef IRQ_H
#define IRQ_H

extern "C" void enable_irq();
extern "C" void disable_irq();
void enable_interrupt_controller();
void disable_interrupt_controller();
void handle_irq();
void timer_init();
void wait_msec(unsigned int n);
void timer_init();

#endif