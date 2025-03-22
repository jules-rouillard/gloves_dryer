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

unsigned char  Check, T_byte1, T_byte2, RH_byte1, RH_byte2, Ch ;
unsigned Temp, RH, RH_ref, Sum ;

void sensor_trigger(){
    IO_RC2_SetDigitalOutput();
    IO_RC2_SetLow();
    DELAY_milliseconds(18);
    IO_RC2_SetHigh();
    DELAY_microseconds(10);
    IO_RC2_SetDigitalInput();
  }

void is_trigger_detected() {
    Check=0;
    DELAY_microseconds(40);
    if(IO_RC2_GetValue()==0){
        DELAY_microseconds(30);
        if(IO_RC2_GetValue()==1){
            Check=1;
            DELAY_microseconds(40);
        }
    }
}

char ReadData(){
    char i, j;
    for(j = 0; j < 8; j++){
        while(!IO_RC2_GetValue()); //Wait until input goes High since start of a bit is always 50us of low
        //If input high for more than 30us then it's a 1 else a 0
        DELAY_microseconds(30); 
        if(IO_RC2_GetValue() == 0)
              i&= ~(1<<(7 - j));  //Clear bit (7-b)
        else {i|= (1 << (7 - j));  //Set bit (7-b)
        while(IO_RC2_GetValue());}  //Wait until PORTD.F0 goes LOW necessary since if a 1 still high after 30us
    }
    return i;
}

void sensor_read(void){
    sensor_trigger();    
    is_trigger_detected();
    if(Check == 1){
        RH_byte1 = ReadData();
        RH_byte2 = ReadData();
        T_byte1 = ReadData();
        T_byte2 = ReadData();
        Sum = ReadData();
        if(Sum == ((RH_byte1+RH_byte2+T_byte1+T_byte2) & 0XFF)){
            Temp = T_byte1;
            RH = RH_byte1;
        }
    }
}

void myTimer2ISR(void){
    LATCbits.LATC3 = ~LATCbits.LATC3;
    sensor_read();
    if(RH>(RH_ref+10)){
        IO_RC1_SetHigh();
        IO_RC0_SetHigh();
    }
    else{
        IO_RC1_SetLow();
        IO_RC0_SetLow();
    }
}

void get_ref_rh_and_start_timer(void){
    sensor_read();
    RH_ref = RH;
    Timer2_Start();
}




int main(void)
{
    SYSTEM_Initialize();
    Timer2_OverflowCallbackRegister(myTimer2ISR);
    IO_RC5_SetInterruptHandler(get_ref_rh_and_start_timer);
    
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
    // IO_RC3_GetValue()  
    // IO_RC4_GetValue()  
    while(1){

    }
    

}