/*
 * File:   pdfusb_4550.h
 * Author: vsk hs-ulm
 *
 * Created on 9. Mai 2014, 10:39
 */

#ifndef PDFUSB_2550_H
#define	PDFUSB_2550_H

//#define OLD_USB_DEMO_BOARD    //for use with the 1. USB DEMO BOARD DM163010

// ------------------------------------------------------ fixed_address_momory.h
#define FIXED_ADDRESS_MEMORY

#define HID_CUSTOM_OUT_DATA_BUFFER_ADDRESS  0x500
#define HID_CUSTOM_IN_DATA_BUFFER_ADDRESS   0x540

#define IN_DATA_BUFFER_ADDRESS_TAG      @0x500    // for CDC device (XC8)
#define OUT_DATA_BUFFER_ADDRESS_TAG     @0x540
#define CONTROL_BUFFER_ADDRESS_TAG      @0x580

void initADC(void);
void initSpecialEventTrigger(void);

#endif	/* PDFUSB_2550_H */
