#include "../all_drivers.h"
#include "mouse.h"
unsigned long timer_ticks;
extern terminal_start;





void (*tasks[32]) (void);

int taskInterval[32];


void timer_handler(struct regs *r)
{
    timer_ticks++;
    maxrand(rtcGetUnixTimestamp(), INT32_MAX);
    read_rtc();
    for(int i = 0; i<32; i++){
        if(tasks[i] != 0){
            if(timer_ticks % taskInterval[i] == 0){
                tasks[i]();
            }
        }
    }

    if(terminalmousecursor && !terminalScrolling){
      if(mouseX != oldmouseX || mouseY != oldmouseY || terminalScrolls != oldscrolls){
        terminal_putrawentryat(oldentry, oldmouseX, oldmouseY - (terminalScrolls - oldscrolls));
        oldmouseX = mouseX; 
        oldmouseY = mouseY; 
        oldscrolls = terminalScrolls;
        oldentry = terminal_getentryat(mouseX, mouseY);
        
      }
      if(mouseDown)
          terminal_putentryat(219, 0x07, mouseX, mouseY);
        else
          terminal_putentryat(219, 0x08, mouseX, mouseY);
    }
    


}

int newTask(void (*func)(void), int interval){
    for(int i = 0; i < 32; i++){
        if(tasks[i] == 0){
            tasks[i] = func;
            taskInterval[i] = interval;
            return i;
        }
    }
    return -1;

}

void newTaskAt(void (*func)(), int interval, int pos){
    tasks[pos] = func;
    taskInterval[pos] = interval;

}

void removeTask(int pos){
    tasks[pos] = 0;
    taskInterval[pos] = 0;

}


void timer_wait(int ms)
{
    unsigned long eticks;
    eticks = timer_ticks + ms;
    while(timer_ticks <= eticks){
        itoa(20, 10); // dummy function call. needed for some reason
    }
}

unsigned long get_timer_ticks(){
    return timer_ticks;
}


void timer_install(uint32_t frequency)
{
   // Firstly, register our timer callback.
   irq_install_handler(0, timer_handler);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32_t divisor = 1193180 / frequency;

   // Send the command byte.
   outportb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outportb(0x40, l);
   outportb(0x40, h);
}