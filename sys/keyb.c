#include <sys/system.h>
#include <sys/kprintf.h>
#include <sys/inout.h>
#include <sys/memcpy.h>

static int shift = 0;
volatile char buffer[1024];
volatile int buff_len=0;
volatile int readflag = 0;

unsigned char kbdus[128] =
{   
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',         /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     /* Enter key */
    0,          /* 29   - Control */ 
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,        /* Left shift */        
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
  'm', ',', '.', '/',   0,                  /* Right shift */
  '*',  
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */   
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',  
    0,  /* Left Arrow */
    0,  
    0,  /* Right Arrow */
  '+',  
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

unsigned char special[13] = {0 , 0, '!' , '@' , '#' , '$', '%' , '^' , '&' , '*' , '(' , ')' , '_'};


/* Handles the keyboard interrupt */
void keyboard_handler()
{
    unsigned char scancode;

    /* Read from the keyboard's data buffer */
    scancode = inb(0x60);

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
    }
    else
    {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */

        /* Just to show you how this works, we simply translate
        *  the keyboard scancode into an ASCII value, and then
        *  display it to the screen. You can get creative and
        *  use some flags to see if a shift is pressed and use a
        *  different layout, or you can add another 128 entries
        *  to the above layout to correspond to 'shift' being
        *  held. If shift is held using the larger lookup table,
        *  you would add 128 to the scancode when you look for it */
        if(scancode == 42) {  //Shift
            shift = 1;            
        }
        else if(scancode == 29) { //Ctrl
            kprintf("^");
            if(readflag == 1)
                buffer[buff_len++] = '^';
        }
        else {
            if(shift == 1 && (kbdus[scancode]>=97 && kbdus[scancode]<=122)) { //Shift was pressed before - current press= alphabet
                kprintf("%c", kbdus[scancode]-32);
                shift = 0;
                if(readflag == 1)
                    buffer[buff_len++] = kbdus[scancode]-32;
            }
            else if(shift == 1 && (scancode >=1 && scancode <=12)) { //Shift was pressed before - current press=top row
                kprintf("%c", special[scancode]);
                shift = 0;
                if(readflag == 1)
                    buffer[buff_len++] = special[scancode];
            }
            else {
                if( kbdus[scancode] == '\n') {
                    readflag = 0;
                }else {
                    kprintf("%c", kbdus[scancode]);
                    if(readflag == 1) {
                        buffer[buff_len++] = kbdus[scancode];
                    }
                }
            }
        }
    }
    outb(0x20, 0x20);
}

void update_buffer(void *buff_addr, int count) {
    readflag = 1;
    __asm__ volatile ( "sti" );

    while(readflag);

    __asm__ volatile ( "cli" );

    readflag = 0;
    memcpy((void *)buff_addr, (void *)buffer, (buff_len));
    ((char*)buff_addr)[buff_len] = '\0';

    //empty buffer
    while(buff_len >= 0) {
        buffer[buff_len--] = '\0';
    }
    buff_len = 0;
    kprintf("\n");
}
