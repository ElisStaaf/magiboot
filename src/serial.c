#include "regs_imx233.h"
#include "serial.h"

static UARTDBG * const uartdbg = (UARTDBG *) REGS_UARTDBG_BASE_PHYS;
#define hw_uartdbg (*uartdbg)
static PINCTRL * const pinctrl = (PINCTRL*) REGS_PINCTRL_BASE_PHYS;
#define hw_pinctrl (*pinctrl) 


/*
 * Set baud rate. The settings are always 8n1:
 * 8 data bits, no parity, 1 stop bit 
 */
static void serial_setbrg (void)
{
    unsigned int cr, lcr_h;
    unsigned int quot;

    // Disable everything
    cr = hw_uartdbg.cr; 
    hw_uartdbg.cr = 0;

    // Calculate and set baudrate
    quot = CONFIG_DBGUART_CLK * 4 / CONFIG_BAUDRATE;
    hw_uartdbg.fbrd = quot & 0x3f;
    hw_uartdbg.ibrd = quot >> 6;

    // Set 8n1 mode, enable FIFOs
    lcr_h = WLEN8 | FEN;
    hw_uartdbg.lcr_h = lcr_h;

    // Enable Debug UART
    hw_uartdbg.cr = cr;
}

void serial_init (void)
{
    unsigned int cr;

    // Set the uart_tx and uart_rx pins to be for the uart, and not e.g.
    // for GPIO.
    /*	hw_pinctrl.muxsel[3].clr = (3<<20)|(3<<22);
    hw_pinctrl.muxsel[3].set =(2<<20)|(2<<22);
	*/

    // Disable UART
    hw_uartdbg.cr = 0;

    // Mask interrupts
    hw_uartdbg.imsc = 0;

    // Set default baudrate
    serial_setbrg();

    // Disable FIFOs.
    cr = hw_uartdbg.lcr_h;
    cr &= ~0x00000010;
    hw_uartdbg.lcr_h = cr;

    // Enable UART
//    cr = DTR | RXE | TXE | RXE | UARTEN;
    cr = hw_uartdbg.cr;
    cr |= TXE | RXE | UARTEN;
    hw_uartdbg.cr = cr;

    return;
}

/* Send a character */
void serial_putc (const char c) {
    // Wait for room in TX FIFO.
    while (hw_uartdbg.fr & TXFF)
        ;

    // Write the data byte.
    hw_uartdbg.dr = c;

    if (c == '\n')
        serial_putc('\r');	//pretty good!
}

void serial_puts (const char *s) {
    while (*s)
        serial_putc(*s++);
}

// Test whether a character is in RX buffer
int serial_tstc (void) {
    int ready = (!(hw_uartdbg.fr & RXFE));

    // Check if RX FIFO is not empty
    if(ready) {
        int data = hw_uartdbg.dr;
        if( data & 0xFFFFFF00 ) {
            // Clear error.
            serial_puts("Clearing serial error...\n");
	    hw_uartdbg.ecr = 0;
            serial_init();
            return serial_tstc();
        }
    }
    return ready;
}

/* Receive character */
int serial_getc (void)
{
    int data = 0;
    while(!data) {
        // Wait while RX FIFO is empty
        while (hw_uartdbg.fr & RXFE)
            ;
	data = hw_uartdbg.dr;

        if( data & 0xFFFFFF00 ) {
            // Clear error.
            serial_puts("Clearing serial error...\n");
	    hw_uartdbg.ecr = 0;
            serial_init();
            data = 0;
        }
    }

    /* Read data byte */
    return data;
}

static char hex[] = "0123456789abcdef";

void serial_puthex(unsigned int c) {
    int i;
    serial_puts("0x");
    for(i=7; i>=0; i--)
        serial_putc(hex[(c>>(4*i))&0x0f]);
}



