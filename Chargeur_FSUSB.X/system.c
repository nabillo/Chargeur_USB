
#include "system.h"
#include "system_config.h"
#include "app_custom_cdc.h"
#include <usb/usb.h>
#include "user.h"

extern enum State cur_State;

void interrupt SYS_InterruptHigh() {
    //Check which interrupt flag caused the interrupt.
    //Service the interrupt
    //Clear the interrupt flag
    //Etc.
/*
    if (TMR2IE && TMR2IF)
    {
#asm
        BSF TMR1H, 7
#endasm
        TMR2IF=0;
        seconds++;
        if (timeout > 0)
        {
            timeout--;
        }
    }
*/
#if defined(USB_INTERRUPT)
    USBDeviceTasks();
#endif
} //This return will be a "retfie fast"
void interrupt low_priority SYS_InterruptLow() {
    //Check which interrupt flag caused the interrupt.
    //Service the interrupt
    //Clear the interrupt flag
    //Etc.
}


/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:
               InitCharger();
            break;

        case SYSTEM_STATE_USB_SUSPEND:
            break;

        case SYSTEM_STATE_USB_RESUME:
            break;
    }
}

