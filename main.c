/*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
 */

/*
© [2024] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
 */
#include "mcc_generated_files/system/system.h"

/*
    Main application
 */

#define SW1 RC4
#define SW2 RC3
#define F0 RC0
#define F1 RC1

unsigned char state, count_3h;
unsigned char Check, T_byte1, T_byte2, RH_byte1, RH_byte2, Ch;
unsigned Temp, RH, RH_ref, Sum;

void send_start_signal() {
    IO_RC2_SetDigitalOutput();
    IO_RC2_SetLow();
    DELAY_milliseconds(18);
    IO_RC2_SetHigh();
    DELAY_microseconds(10);
    IO_RC2_SetDigitalInput();
}

void is_sensor_start_detected() {
    Check = 0;
    DELAY_microseconds(40);
    if (IO_RC2_GetValue() == 0) {
        DELAY_microseconds(30);
        if (IO_RC2_GetValue() == 1) {
            Check = 1;
            DELAY_microseconds(40);
        }
    }
}

char ReadData() {
    char i, j;
    for (j = 0; j < 8; j++) {
        while (!IO_RC2_GetValue()); //Wait until input goes high
        DELAY_microseconds(30);
        
        //If input is low after 30us then the bit is a 0
        if (IO_RC2_GetValue() == 0)
            i &= ~(1 << (7 - j)); //Clear bit (7-b)
        else {
            i |= (1 << (7 - j)); //Set bit (7-b)
            while (IO_RC2_GetValue());
        } //Wait until PORTD.F0 goes LOW necessary since if a 1 still high after 30us
    }
    return i;
}

void sensor_read(void) {
    send_start_signal();
    is_sensor_start_detected();
    if (Check == 1) {
        RH_byte1 = ReadData();
        RH_byte2 = ReadData();
        T_byte1 = ReadData();
        T_byte2 = ReadData();
        Sum = ReadData();
        //Checking check_sum
        if (Sum == ((RH_byte1 + RH_byte2 + T_byte1 + T_byte2) & 0XFF)) {
            Temp = T_byte1;
            RH = RH_byte1;
        }
    }
}

void myTimer2ISR(void) {
    if (state == 2) {
        //Read sensor
        sensor_read();
        //Compare to reference value
        if (RH > (RH_ref + 5)) {
            F0 = 1;
            F1 = 1;
        } else {
            F0 = 0;
            F1 = 0;
        }
    } else {
        if (++count_3h >= 18) {
            //After 3 hours timer2 and fans are turned off
            count_3h = 0;
            F0 = 0;
            F1 = 0;
            Timer2_Stop();
        }
    }
}

void button_press(void) {
    Timer2_Stop();
    if (SW2 == 0 && SW1 == 0) {
        // Always on Code 1
        state = 1;
        IO_RC0_Toggle();
        IO_RC1_Toggle();
    } else if (SW2 == 0 && SW1 == 1) {
        // Sensor control Code 2
        //Set reference humidity value and start timer2
        state = 2;
        sensor_read();
        RH_ref = RH;
        //Turn on fan as the first timer2ISR won't be done until 10min 
        F0 = 1;
        F1 = 1;
        Timer2_Start();
    } else if (SW2 == 1 && SW1 == 0) {
        // Undefine Code 3
        state = 3;
        F0 = 0;
        F1 = 0;
    } else {
        //Time control Code 4
        state = 4;
        count_3h = 0;
        F0 = 1;
        F1 = 1;
        Timer2_Start();
    }
}

int main(void) {
    SYSTEM_Initialize();
    Timer2_OverflowCallbackRegister(myTimer2ISR);
    IO_RC5_SetInterruptHandler(button_press);
    F0 = 0;
    F1 = 0;
    count_3h = 0;

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts 
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts 
    // Use the following macros to: 

    // Enable the Global Interrupts 
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts 
    //INTERRUPT_GlobalInterruptDisable(); 

    // Enable the Peripheral Interrupts 
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts 
    //INTERRUPT_PeripheralInterruptDisable(); 

    while (1) {
        //Nothing to do here
    }
}