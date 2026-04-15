#ifndef IDT_H
#define IDT_H

void idt_init();
void idt_set_gate(int num, unsigned long base);

#endif