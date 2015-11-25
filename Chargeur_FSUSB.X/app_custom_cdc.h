/* 
 * File:   app_custom_cdc.h
 * Author: vsk hs-ulm
 *
 * Created on 22. July 2014, 13:30
 */
#ifndef CUSTOM_CDC_H
#define	CUSTOM_CDC_H

//------------------------------------------- P U B L I C    P R O T O T Y P E S
void APP_CustomCDCInitialize(void);     // legacy
void APP_USB_send(char *msg);
void APP_CustomCDCTasks(void);          // legacy ProcessIO()

void APP_USBCBSuspend(void);            // empty ;-)
void APP_USBCBWakeFromSuspend(void);    //
void APP_USBCB_SOF_Handler(void);       //
void APP_USBCBErrorHandler(void);       //
void APP_USBCBStdSetDscHandler(void);   //


#endif	/* CUSTOM_CDC_H */

