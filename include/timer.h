#ifndef TIMER_H
#define TIMER_H

void timer_init();
void timer_handler();
void delay(int ticks);
unsigned int timer_ticks();

#endif