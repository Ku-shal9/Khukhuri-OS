global idt_load
global irq0
global irq1

extern idtp
extern isr_handler

section .text
idt_load:
    lidt [idtp]
    ret

irq0:
    push dword 32
    call isr_handler
    add esp, 4
    iretd

irq1:
    push dword 33
    call isr_handler
    add esp, 4
    iretd
