/* 
 * Small application to dim the backlichgt of a GLCD2USB to zero
 * Till Harbaum
 * Philipp Klaus
 * Licensed under GPL
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "usbcalls.h"

#define IDENT_VENDOR_NUM        0x1c40
#define IDENT_PRODUCT_NUM       0x0525

#define IDENT_VENDOR_STRING     "www.harbaum.org/till/glcd2usb"
#define IDENT_PRODUCT_STRING    "GLCD2USB"

#define IDENT_VENDOR_NUM_OLD     0x0403
#define IDENT_PRODUCT_NUM_OLD    0xc634

/* ------------------------------------------------------------------------- */

char    *usbErrorMessage(int errCode) {
  static char buffer[80];
  
  switch(errCode){
  case USB_ERROR_ACCESS:      return "Access to device denied";
  case USB_ERROR_NOTFOUND:    return "The specified device was not found";
  case USB_ERROR_BUSY:        return "The device is used by another application";
  case USB_ERROR_IO:          return "Communication error with device";
  default:
    sprintf(buffer, "Unknown USB error %d", errCode);
    return buffer;
  }
  return NULL;    /* not reached */
}

/* ------------------------------------------------------------------------- */

/* this is placed in the lcd4linux directory to make sure lcd4linux can */
/* still be compiled without the glcd2usb firmware being present */
#include "../lcd4linux/glcd2usb.h"

int main(int argc, char **argv)
{
  usbDevice_t *dev = NULL;
  int         err = 0, len;

  /* message buffer for messages going forth and back between PC and */
  /* GLCD2USB unit */
  union {
    char bytes[132];
    display_info_t display_info;
  } buffer;

  /* open a connection to the device */
  if((err = usbOpenDevice(&dev, IDENT_VENDOR_NUM, IDENT_VENDOR_STRING, 
			  IDENT_PRODUCT_NUM, IDENT_PRODUCT_STRING, 1)) != 0){
    if((err = usbOpenDevice(&dev, IDENT_VENDOR_NUM_OLD, IDENT_VENDOR_STRING, 
			    IDENT_PRODUCT_NUM_OLD, IDENT_PRODUCT_STRING, 1)) != 0){
      fprintf(stderr, "Error opening GLCD2USB device: %s\n", usbErrorMessage(err));
      goto errorOccurred;
    }
  }

  /* request the buffer to be filled with display info */
  memset(&buffer, 0, sizeof(buffer));
  len = sizeof(display_info_t);
  if((err = usbGetReport(dev, USB_HID_REPORT_TYPE_FEATURE, 
			 GLCD2USB_RID_GET_INFO, buffer.bytes, &len)) != 0){
    fprintf(stderr, "Error getting display info: %s\n", usbErrorMessage(err));
    goto errorOccurred;
  }

  if(len < sizeof(buffer.display_info)){
    fprintf(stderr, "Not enough bytes in display info report (%d instead of %d)\n", 
	    len, (int)sizeof(buffer.display_info));
    err = -1;
    goto errorOccurred;
  }

  printf("Display name: %s\n", buffer.display_info.name);
  printf("Display resolution: %d * %d\n", 
	 buffer.display_info.width, buffer.display_info.height);
  printf("Display flags: %x\n", buffer.display_info.flags);

  /* this driver currently does not support all display memory arrangements */
  if(buffer.display_info.flags & FLAG_SIX_BIT) {
    fprintf(stderr, "Error: Six bit displays are not supported yet\n");
    err = -1;
    goto errorOccurred;
  }

  if(buffer.display_info.flags & FLAG_VERTICAL_INC) {
    fprintf(stderr, "Error: Vertical increase is not supported yet\n");
    err = -1;
    goto errorOccurred;
  }

  if(!(buffer.display_info.flags & FLAG_VERTICAL_UNITS)) {
    fprintf(stderr, "Error: Horizontal units are not supported yet\n");
    err = -1;
    goto errorOccurred;
  }

  /* get access to display */
  buffer.bytes[0] = GLCD2USB_RID_SET_ALLOC;
  buffer.bytes[1] = 1;  /* 1 -> alloc display, 0 -> free it */
  if((err = usbSetReport(dev, USB_HID_REPORT_TYPE_FEATURE, 
			 buffer.bytes, 2)) != 0) {
    fprintf(stderr, "Error allocating display: %s\n", usbErrorMessage(err));
    goto errorOccurred;
  }
  printf("Display allocated\n");


  /* Clear Display and dim down to zero */
  int bright = 0;
  buffer.bytes[0] = GLCD2USB_RID_SET_BL;
  buffer.bytes[1] = bright & 0xff;
  if((err = usbSetReport(dev, USB_HID_REPORT_TYPE_FEATURE, buffer.bytes, 2)) != 0) {
    fprintf(stderr, "Error freeing display: %s\n", usbErrorMessage(err));
    goto errorOccurred;
  }
  
  printf("Botton map on exit: 0x%02x\n", buffer.bytes[1]);

  /* release access to display */
  buffer.bytes[0] = GLCD2USB_RID_SET_ALLOC;
  buffer.bytes[1] = 0;  /* 1->alloc, 0->free */ 
  if((err = usbSetReport(dev, USB_HID_REPORT_TYPE_FEATURE, 
			 buffer.bytes, 2)) != 0) {
    fprintf(stderr, "Error freeing display: %s\n", usbErrorMessage(err));
    goto errorOccurred;
  }
  printf("Display freed\n");

errorOccurred:
    if(dev != NULL)
        usbCloseDevice(dev);

#ifdef WIN32
    printf("Press key\n");
    getchar();
#endif

    return 0;
}

/* ------------------------------------------------------------------------- */


