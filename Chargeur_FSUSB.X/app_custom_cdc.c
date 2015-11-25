/*********************************************************************
 *
 *                CDC  Firmware
 *
 *********************************************************************
 * FileName:        app_custom_cdc.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18F14K50 - 4550 - 27J53
 * Compiler:        C18 3.40+
 * Company:         HS-Ulm
 *
 * Author       Company     Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * VSK          HS-Ulm      29.09.10
 *                          02.10.13    added MEM_MODEL define (rom strings ...)
 *                                      changed buffer types (uchar -> char)
 *                          05.05.14    major changes to MLA 2013_12_20
 *                          22.07.14    cdc version and 27J53 added
 *
 *		TODO
 *
 *		-	...
 *
 ******************************************************************************/

#include <stdio.h>
#include <usb/usb.h>
#include <usb/usb_device_cdc.h>
#include "system.h"
#include "app_custom_cdc.h"

#include "system.h"
#include "user.h"
#include "battery.h"


/** P U B L I C    P R O T O T Y P E S ****************************************/


/** P R I V A T E  P R O T O T Y P E S ****************************************/


/** P R I V A T E  C O N S T A N T  S T R I N G S *****************************/
const char strDevice[]    = "CHARGER";
const char strVersion[]   = "Version 1.0";
const char strDate[]      = "24. Juil. 2015";
const char strManu[]      = "BTN";
const char strMisc[]      = "BTN-CRG";
  
/** VARIABLES ******************************************************/
enum State cur_State;
PROGRAM cur_Program;
ACTION cur_Action;
Battery battery;

unsigned short long precondition_Time;
unsigned short long charge_Time;
tPIParams PI; // Data Structure for PI controller
unsigned short timeout;
long seconds;
char msg_info[256];


/*********************************************************************
* Function:     void APP_CustomHIDInitialize(void);
*               (legacy USBCBInitEP();)
*
* Overview:     Initializes the Custom HID demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_CustomCDCInitialize()
{
    CDCInitEP();
}

/*********************************************************************
* Function:         void APP_USB_send(const char *format, ...);
*
* Overview:         Dend string to host.
*
* PreCondition:     The demo should have been initialized and started via
*                   the APP_DeviceCustomHIDInitialize()
*                   and APP_DeviceCustomHIDStart() demos
*                   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_USB_send(char * msg)
{
    static uint8_t  usbOutBuffer[CDC_DATA_OUT_EP_SIZE];
    int n;
    
    memset(usbOutBuffer,'\0',sizeof(usbOutBuffer));
    n = sprintf(usbOutBuffer,"%s",msg);
    
    if (( USBGetDeviceState() < CONFIGURED_STATE ) || ( USBIsDeviceSuspended() == true ))
    {
        return;
    }
    else
    {
        if (mUSBUSARTIsTxTrfReady())
        {
            putUSBUSART(usbOutBuffer,n);
        }
    }
    CDCTxService();
}

/*********************************************************************
* Function:         void APP_CustomHIDTasks(void);
*
* Overview:         Keeps the Custom HID demo running.
*
* PreCondition:     The demo should have been initialized and started via
*                   the APP_DeviceCustomHIDInitialize()
*                   and APP_DeviceCustomHIDStart() demos
*                   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_CustomCDCTasks()
{

    short res;
    short precondition;
    char end[10];
    char msg_Error[100];

    memset(msg_Error, '\0', sizeof(msg_Error));
    
    memset(msg_info, '\0', sizeof(msg_info));
    sprintf(msg_info,"Application initialized\n");
    APP_USB_send(msg_info);
    memset(msg_info, '\0', sizeof(msg_info));
    sprintf(msg_info,"cur_State <%d>\n",cur_State);
    APP_USB_send(msg_info);
    
    res = INDEFINED;
    switch (cur_State)
    {
        case POWERUP :
            res = initialize();
            if (res != OK)
            {
                strcpy(msg_Error,"Charger Error");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = INITIALIZERD;
            }
            break;

        case INITIALIZERD :
            res = list_Battery();
            if (res != OK)
            {
                strcpy(msg_Error,"Battery list Error");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = BATTERIES_LISTED;
            }
            break;

        case BATTERIES_LISTED :
            select_Battery();
            cur_State = BATTERY_SELECTED;
            break;

        case BATTERY_SELECTED :
            res = list_Action();
            if (res != OK)
            {
                strcpy(msg_Error,"Action list Error");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = ACTION_LISTED;
            }
            break;

        case ACTION_LISTED :
            select_Action();
            cur_State = ACTION_SELECTED;
            break;

        case ACTION_SELECTED :
            switch (cur_Action)
            {
                case RUN :
                    cur_State = RUN_SELECTED;
                break;

                case EDIT :
                    cur_State = EDIT_SELECTED;
                break;

                case DELETE :
                    cur_State = DELETE_SELECTED;
                break;
            }
            break;

        case RUN_SELECTED :
            res = list_Program();
            if (res != OK)
            {
                strcpy(msg_Error,"Program list Error");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = PROGRAM_LISTED;
            }
            break;

        case PROGRAM_LISTED :
            select_Program();
            cur_State = PROGRAM_SELECTED;
            break;

        case PROGRAM_SELECTED :
            switch (cur_Program)
            {
                case CHARGE :
                        /* LIPO */
                        if (strncmp(battery.battery_type, algo_Name[0], 5))
                        {
                            res = check_Lipo();
                            if (res != OK)
                            {
                                strcpy(msg_Error,"Liop check failure");
                                cur_State = CHARGE_ERROR;
                            }
                            else
                            {
                                cur_State = LIPO_ALGO_STARTED;
                            }
                        }
                        /* NIMH */
                        else if (strncmp(battery.battery_type, algo_Name[1], 5))
                        {

                        }
                        /* PB */
                        else if (strncmp(battery.battery_type, algo_Name[2], 5))
                        {

                        }
                    break;

                case STORAGE :

                    break;
                case DISCHARGE :

                    break;
                case CYCLE :

                    break;
                case BALANCE :

                    break;

                default :

                    break;
            }
            break;

        case LIPO_ALGO_STARTED :
            res = check_Precondition(&precondition);
            if (res != OK)
            {
                strcpy(msg_Error,"Precondition check failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = PRECONDITION_SELECTED;
            }
            break;

        case PRECONDITION_SELECTED :
            if (precondition)
            {
                res = start_Precondition();
                if (res != OK)
                {
                    strcpy(msg_Error,"Precondition starting failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = PRECONDITION_STARTED;
                }
            }
            else
            {
                res = start_CC();
                if (res != OK)
                {
                    strcpy(msg_Error,"CC starting failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = CC_PHASE_STARTED;
                }
            }
            break;

        case PRECONDITION_STARTED :
        case PRECONDITION_VERIFICATION :
            res = verifie_Precondition(end);
            if (res != OK)
            {
                strcpy(msg_Error,"precondition verification failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                if (strncmp(end,"END",3))
                {
                    cur_State = PRECONDITION_ENDED;
                }
                else if (strncmp(end,"TIMER",5))
                {
                    strcpy(msg_Error,"Precondition safety timer ended");
                    cur_State = CHARGE_ERROR;
                }
                else if (strncmp(end,"PROGRESS",8))
                {
                    cur_State = PRECONDITION_VERIFICATION;
                }
            }
            break;

        case PRECONDITION_ENDED :
            res = start_CC();
            if (res != OK)
            {
                strcpy(msg_Error,"CC starting failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = CC_PHASE_STARTED;
            }
            break;

        case CC_PHASE_STARTED :
        case VBAT_VERIFICATION :
            res = verifie_Vbat(end);
            if (res != OK)
            {
                strcpy(msg_Error,"CC verification failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                if (strncmp(end,"END",3))
                {
                    cur_State = CC_PHASE_ENDED;
                }
                else if (strncmp(end,"TIMER",5))
                {
                    strcpy(msg_Error,"CC safety timer ended\n");
                    cur_State = CHARGE_ERROR;
                }
                else if (strncmp(end,"PROGRESS",8))
                {
                    cur_State = VBAT_VERIFICATION;
                }
            }
            break;

        case CC_PHASE_ENDED :
            res = start_CV();
            if (res != OK)
            {
                strcpy(msg_Error,"CV starting failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = CV_PHASE_STARTED;
            }
            break;

        case CV_PHASE_STARTED :
        case IBAT_VERIFICATION :
            res = verifie_Ibat(end);
            if (res != OK)
            {
                strcpy(msg_Error,"CV verification failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                if (strncmp(end,"END",3))
                {
                    cur_State = CV_PHASE_ENDED;
                }
                else if (strncmp(end,"TIMER",5))
                {
                    strcpy(msg_Error,"CV safety timer ended");
                    cur_State = CHARGE_ERROR;
                }
                else if (strncmp(end,"PROGRESS",8))
                {
                    cur_State = IBAT_VERIFICATION;
                }
            }
            break;

        case CV_PHASE_ENDED :
            res = stop_CV();
            if (res != OK)
            {
                strcpy(msg_Error,"CV stoping failure");
                cur_State = CHARGE_ERROR;
            }
            else
            {
                cur_State = CHARGE_ENDED;
            }
            break;

        case CHARGE_ENDED :
            end_Charge();
            break;

        case CHARGE_ERROR :

            break;
        default :

            break;
    }
    APP_USB_send(msg_Error);
    
}//end APP_CustomCDCTasks

void utoa(uint8_t * buf, int val, int base)
{
	unsigned	v;
	char		c;

	v = val;
	do {
		v /= base;
		buf++;
	} while(v != 0);
	*buf-- = 0;
	do {
		c = val % base;
		val /= base;
		if(c >= 10)
			c += 'A'-'0'-10;
		c += '0';
		*buf-- = c;
	} while(val != 0);

}

// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA* each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

// Note *: The "usb_20.pdf" specs indicate 500uA or 2.5mA, depending upon device classification. However,
// the USB-IF has officially issued an ECN (engineering change notice) changing this to 2.5mA for all
// devices.  Make sure to re-download the latest specifications to get all of the newest ECNs.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/
void APP_USBCBSuspend(void)
{
	//Example power saving code.  Insert appropriate code here for the desired
	//application behavior.  If the microcontroller will be put to sleep, a
	//process similar to that shown below may be used:

	//ConfigureIOPinsForLowPower();
	//SaveStateOfAllInterruptEnableBits();
	//DisableAllInterruptEnableBits();
	//EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
	//Sleep();
	//RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
	//RestoreIOPinsToNormal();									//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

	//IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is
	//cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause
	//things to not work as intended.

}


/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *
 *					This call back is invoked when a wakeup from USB suspend
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/
void APP_USBCBWakeFromSuspend(void)
{
	// If clock switching or other power savings measures were taken when
	// executing the USBCBSuspend() function, now would be a good time to
	// switch back to normal full power run mode conditions.  The host allows
	// 10+ milliseconds of wakeup time, after which the device must be
	// fully back to normal, and capable of receiving and processing USB
	// packets.  In order to do this, the USB module must receive proper
	// clocking (IE: 48MHz clock must be available to SIE for full speed USB
	// operation).
	// Make sure the selected oscillator settings are consistent with USB
    // operation before returning from this function.
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void APP_USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void APP_USBCBErrorHandler(void)
{
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

	// Typically, user firmware does not need to do anything special
	// if a USB error occurs.  For example, if the host sends an OUT
	// packet to your device, but the packet gets corrupted (ex:
	// because of a bad connection, or the user unplugs the
	// USB cable during the transmission) this will typically set
	// one or more USB error interrupt flags.  Nothing specific
	// needs to be done however, since the SIE will automatically
	// send a "NAK" packet to the host.  In response to this, the
	// host will normally retry to send the packet again, and no
	// data loss occurs.  The system will typically recover
	// automatically, without the need for application firmware
	// intervention.

	// Nevertheless, this callback function is provided, such as
	// for debugging purposes.
}


/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 *                  firmware must process the request and respond
 *                  appropriately to fulfill the request.  Some of
 *                  the SETUP packets will be for standard
 *                  USB "chapter 9" (as in, fulfilling chapter 9 of
 *                  the official USB specifications) requests, while
 *                  others may be specific to the USB device class
 *                  that is being implemented.  For example, a HID
 *                  class device needs to be able to respond to
 *                  "GET REPORT" type of requests.  This
 *                  is not a standard USB chapter 9 request, and
 *                  therefore not handled by usb_device.c.  Instead
 *                  this request should be handled by class specific
 *                  firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/
void APP_USBCBCheckOtherReq(void)
{
    USBCheckCDCRequest();
}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/
void APP_USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end


/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 *                  peripheral devices to wake up a host PC (such
 *                  as if it is in a low power suspend to RAM state).
 *                  This can be a very useful feature in some
 *                  USB applications, such as an Infrared remote
 *                  control receiver.
 *                  If a user presses the "power"
 *                  button on a remote control, it is nice that the
 *                  IR receiver can detect this signalling, and then
 *                  send a USB "command" to the PC to wake up.
 *
 *                  The USBCBSendResume() "callback" function is used
 *                  to send this special USB signalling which wakes
 *                  up the PC.  This function may be called by
 *                  application firmware to wake up the PC.  This
 *                  function will only be able to wake up the host if
 *                  all of the below are true:
 *
 *                  1.  The USB driver used on the host PC supports
 *                      the remote wakeup capability.
 *                  2.  The USB configuration descriptor indicates
 *                      the device is remote wakeup capable in the
 *                      bmAttributes field.
 *                  3.  The USB host PC is currently sleeping,
 *                      and has previously sent your device a SET
 *                      FEATURE setup packet which "armed" the
 *                      remote wakeup capability.
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior,
 *                  as a USB device that has not been armed to perform remote
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *
 *                  This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex:
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup.
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void)
{
    static uint16_t delay_count;

    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager
    //properties page for the USB device, power management tab, the
    //"Allow this device to bring the computer out of standby." checkbox
    //should be checked).
    if(USBGetRemoteWakeupStatus() == true)
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if(USBIsBusSuspended() == true)
        {
            USBMaskInterrupts();

            //Clock switch to settings consistent with normal USB operation.
            APP_USBCBWakeFromSuspend();
            USBSuspendControl = 0;
            USBBusIsSuspended = false;  //So we don't execute this code again,
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            delay_count = 3600U;
            do
            {
                delay_count--;
            }while(delay_count);

            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling
            delay_count = 1800U;        // Set RESUME line for 1-13 ms
            do
            {
                delay_count--;
            }while(delay_count);
            USBResumeControl = 0;       //Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}

/** EOF app_custom_cdc.c ************************************************************/
