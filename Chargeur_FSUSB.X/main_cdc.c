/********************************************************************
 FileName:      main_cdc.c
 Dependencies:  See INCLUDES section
 Processor:     PIC18 USB Microcontrollers
 
 Hardware:	This firmware is made based on Microchip USB demo
 		board board code supported by the MCHPFSUSB stack.
 		(*** Device - CDC - Custom Demos ***)
 		
 Complier:      Microchip C18 / XC8 (for PIC18)
 Company:	HS-Ulm

********************************************************************
 File Description:

 Change History:
  Rev   Date    Description
  ----  ------- -----------------------------------------------------
  1.0   ?       Initial release
  2.0   May 14  major changes (added XC8 / MLA 2013_12_20)

 ********************************************************************/

/** INCLUDES *******************************************************/

#include "system.h"
#include <usb/usb.h>
#include <usb/usb_device_cdc.h>
#include "app_custom_cdc.h"


/** DECLARATIONS ***************************************************/


/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
MAIN_RETURN main(void)
{   
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);

    USBDeviceInit();            //usb_device.c
    #if defined(USB_INTERRUPT)
      USBDeviceAttach();        //usb_device.c
    #endif

    while(1)
    {
        SYSTEM_Tasks();

        // Application-specific tasks.
        APP_CustomCDCTasks();

        //CDCTxService();
    }//end while
}//end main

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        USB_EVENT event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           USB_EVENT event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch(event)
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            APP_USBCB_SOF_Handler();
            /* We are using the SOF as a timer to time the LED indicator.  Call
             * the LED update function here. */
//new MLA           APP_LEDUpdateUSBStatus();
            break;
        case EVENT_SUSPEND:
            APP_USBCBSuspend();
            /* Update the LED status for the suspend event. */
//new MLA            APP_LEDUpdateUSBStatus();
            break;
        case EVENT_RESUME:
            APP_USBCBWakeFromSuspend();
            /* Update the LED status for the resume event. */
//new MLA            APP_LEDUpdateUSBStatus();
            break;
        case EVENT_CONFIGURED:
            /* When the device is configured, we can (re)initialize the demo
             * code. */
//old MLA            USBCBInitEP();
            APP_CustomCDCInitialize();
            break;
        case EVENT_SET_DESCRIPTOR:
//            APP_USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            /* We have received a non-standard USB request.  The CDC driver
             * needs to check to see if the request was for it. */
//            APP_USBCBCheckOtherReq();
            USBCheckCDCRequest();
            break;
        case EVENT_BUS_ERROR:
            APP_USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT
            //      endpoints).
            break;
        default:
            break;
    }
    return true;
}

/** EOF main_hid.c ************************************************************/
