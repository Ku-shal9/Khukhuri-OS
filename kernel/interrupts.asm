global idt_load
global irq0
global irq1
global irq12

extern idtp
extern isr_handler

section .text
idt_load:
    lidt [idtp]
    ret

irq0:
    pushad
    push dword 32
    call isr_handler
    add esp, 4
    popad
    iretd

irq1:
    pushad
    push dword 33
    call isr_handler
    add esp, 4
    popad
    iretd

irq12:
    pushad
    push dword 44
    call isr_handler
    add esp, 4
    popad
    iretd
