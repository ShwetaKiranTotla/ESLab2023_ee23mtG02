#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

//A 100kHz PWM varying duty cycle waveform on Pin PF2; controlled by Module 1 PWM Generator 3 A and SW2

//Interrupt Handlers
void GPIOFHandler(void);
void SysTickHandler(void){}
void UART5Handler(void);
char UART5_Receiver(void);
void UART5_Transmitter(unsigned char data);

/* SysTick memory-mapped registers */
#define STCTRL *((volatile long *) 0xE000E010)    // control and status
#define STRELOAD *((volatile long *) 0xE000E014)    // reload value
#define STCURRENT *((volatile long *) 0xE000E018)    // current value

#define COUNT_FLAG  (1 << 16)   // bit 16 of CSR automatically set to 1
                                //   when timer expires
#define ENABLE      (1 << 0)    // bit 0 of CSR to enable the timer
#define CLKINT      (1 << 2)    // bit 2 of CSR to specify CPU clock

#define CLOCK_MHZ 16

void Delay(float ms)
{
    STCURRENT = 0;
    STRELOAD = 16000*ms; //1000us    // reload value for 'ms' milliseconds
    STCTRL |= (CLKINT | ENABLE);        // set internal clock, enable the timer

    while ((STCTRL & COUNT_FLAG) == 0)  // wait until flag is set
    {
        ;   // do nothing
    }
    STCTRL = 0;                // stop the timer

}


#define RED_LED (1 << 1)
#define BLUE_LED (1 << 2)
#define GREEN_LED (1 << 3)
#define MASK_BITS 0x11 //SW1 and SW2

void UARTInit(void){
    /* UART5 initialization */
    UART5_CTL_R = 0;         /* UART5 module disbable */
    UART5_IBRD_R = 104;      /* for 9600 baud rate, integer = 104 */
    UART5_FBRD_R = 11;       /* for 9600 baud rate, fractional = 11*/
    UART5_CC_R = 0;          /*select system clock*/
    UART5_LCRH_R = 0x62;     /* data lenght 8-bit, odd parity bit, no FIFO */
    UART5_CTL_R = 0x301;     /* Enable UART5 module, Rx and Tx */

    /* UART5 TX5 and RX5 use PE4 and PE5. Configure them digital and enable alternate function */
    GPIO_PORTE_DEN_R = 0x30;      /* set PE4 and PE5 as digital */
    GPIO_PORTE_AFSEL_R = 0x30;    /* Use PE4,PE5 alternate function */
    GPIO_PORTE_AMSEL_R = 0;    /* Turn off analog function*/
    GPIO_PORTE_PCTL_R  = 0x00110000;     /* configure PE4 and PE5 for UART */

      // enable interrupt
    UART5_ICR_R &= ~(0x0780);; // Clear receive interrupt
    UART5_IM_R = 0x0010; //Enable UART5 Rx Interrupt
    NVIC_EN1_R |= 0x20000000; /* enable IRQ61 for UART5 */


}

void GPIO_PORTF_setup(void)
{
    SYSCTL_RCGCGPIO_R |= (1<<5);        /* enable clock to GPIOF */
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x1F;             /* make PORTF0 configurable */
    GPIO_PORTF_PUR_R = 0x11;            /* enable pull up for pin 4 and 0 */
    GPIO_PORTF_DEN_R = 0x1F;            /* set PORTF pins 4-3-2-1 as digital pins */
    GPIO_PORTF_DIR_R = 0x0E;            /* set PORTF3+PORTF2+PORTF1 pin as output (LED) pin */
    /* and PORTF4 and PORTF0 as input, SW1 is on PORTF4  and SW2 is PORTF0*/
    //GPIO PortF Interrupt Configurations
    GPIO_PORTF_IM_R &= ~MASK_BITS; // mask interrupt by clearing bits
    GPIO_PORTF_IS_R &= ~MASK_BITS; // edge sensitive interrupts
    GPIO_PORTF_IBE_R &= ~MASK_BITS; // interrupt NOT on both edges
    GPIO_PORTF_IEV_R &= ~MASK_BITS; // rising edge trigger

    /* Enable interrupt generation in GPIO */
    GPIO_PORTF_ICR_R |= MASK_BITS; // clear any prior interrupt
    GPIO_PORTF_IM_R |= MASK_BITS; // unmask interrupt by setting bits

    /* Prioritize and enable interrupts in NVIC */
    NVIC_PRI7_R &= 0xFF3FFFFF;
     // interrupt priority register 7
     // bits 21-23 for interrupt 30 (port F)
    NVIC_EN0_R |= 1 << 30; // enable interrupt 30 (port F)

}

int main(void)
{
    SYSCTL_RCGCUART_R |= 0x20;  /* enable clock to UART5 */
    SYSCTL_RCGCGPIO_R |= 0x10;  /* enable clock to PORTE for PE4/Rx and PE5/Tx */
    UARTInit();
    GPIO_PORTF_setup();
    while (1){
        UART5_DR_R = 0xF0;
        Delay(100);
    }
}

void GPIOFHandler(void){
    GPIO_PORTF_IM_R &= ~MASK_BITS;//clear interrupt

        if(GPIO_PORTF_RIS_R & 0x10)    //SW 2
        {
            UART5_DR_R = 0xAA;//transmit "aa"

        }

        if(GPIO_PORTF_RIS_R & 0x01)    //SW 1
        {
            UART5_DR_R = 0xF0';//transmit "f0"

        }
}

void UART5Handler(void){
    unsigned char rx_data = 0;
    UART5_ICR_R &= ~(0x010); // Clear receive interrupt
    rx_data = UART5_DR_R ; // get the received data byte
        if(rx_data == 0xAA){/*AA*/
            GPIO_PORTF_DATA_R |= GREEN_LED;
        }
        else if(rx_data == 0xF0){/*F0*/
            GPIO_PORTF_DATA_R |= BLUE_LED;
        }
}
