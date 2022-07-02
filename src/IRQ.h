#ifndef IRQ_H
#define IRQ_H

extern "C" void irq_init_vectors();
extern "C" void irq_enable();
extern "C" void irq_disable();
void enable_interrupt_controller();
void disable_interrupt_controller();
void handle_irq();
void timer_init();
void wait_msec(unsigned int n);

#endif