/* Speedtest functions for USBprog GNU/GPL 2 (C) Benedikt Sauter 2008 */

#include <stdio.h>
#include <usb.h>

#include "client.h"

int main (int argc,char **argv)
{
	struct usb_dev_handle *usbprog_handle;
	struct usb_device *usbprog_device;
	int send_status;
	int open_status;
	unsigned char send_data[10000];
	unsigned char receive_data[64];

	usb_init();
	usb_set_debug(2);

 	if ((usbprog_handle = usbprog_locate())==0) 
	{
		printf("Could not open the USBprog device\n");
		return (-1);
	}  

	open_status = usb_set_configuration(usbprog_handle,1);
	printf("conf_stat=%d\n",open_status);
	
	open_status = usb_claim_interface(usbprog_handle,0);
	printf("claim_stat=%d\n",open_status);
	
	open_status = usb_set_altinterface(usbprog_handle,0);
	printf("alt_stat=%d\n",open_status);


	int k;
	for(k=0;k<10000;k++)
		send_data[k]=(unsigned char)k;


	send_data[0] = 0x77;
	send_data[1] = 0x88;

	usb_bulk_write(usbprog_handle,2,send_data,320,500);
	usb_bulk_read(usbprog_handle,2,receive_data,320,500);	

	usb_close(usbprog_handle);
}	


/* find and open first usbprog on usb (bus) */
usb_dev_handle *usbprog_locate(void) 
{
	unsigned char located = 0;
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *device_handle = 0;
 		
	usb_find_busses();
	usb_find_devices();
 
 	for (bus = usb_busses; bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)	
		{
			if (dev->descriptor.idVendor == 0x1786) 
			{	
				located++;
				device_handle = usb_open(dev);
				printf("USBprog Device Found @ Address %s \n", dev->filename);
				printf("USBprog Vendor ID 0x0%x\n",dev->descriptor.idVendor);
				printf("USBprog Product ID 0x0%x\n",dev->descriptor.idProduct);
			}
			else printf("** usb device %s found **\n", dev->filename);			
		}	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}

int usbprog_close(usb_dev_handle * usbprog_handle) 
{
	if( usb_close(usbprog_handle) )
		return 1;
	else
		return -1;
}

/* transmit and receive command buffer */
int usbprog_command_buffer(usb_dev_handle * usbprog_handle, char *read_buffer, int read_length, char *write_buffer, int write_length)
{
	// usb_bulk_write
	// usb_bulk_read
	return -1;
}


/* control trst signal 0:low, 1:high */
int usbprog_trst(usb_dev_handle * usbprog_handle, int value) 
{
	if (value == 1) {
		BUFFER_ADD = SET_TRST(1);
		return 1;
	} else if (value == 0) {
		BUFFER_ADD = SET_TRST(0);
		return 1;
	} else {
		return -1;
	}
}


/* control trst signal 0:low, 1:high */
int usbprog_srst(usb_dev_handle * usbprog_handle, int value) 
{
	if (value == 1) {
		BUFFER_ADD = SET_SRST(1);
		return 1;
	} else if (value == 0) {
		BUFFER_ADD = SET_SRST(0);
		return 1;
	} else {
		return -1;
	}
}



/* control led 0:off, 1:on */
int usbprog_led(usb_dev_handle * usbprog_handle, int value) 
{
	if (value == 1) {
		usb_control_msg(usbprog_handle, USB_VENDOR_REQUEST, USER_INTERFACE, LED_ON, 0, NULL, 8, 1000);
		return 1;
	} else if (value == 0) {
		usb_control_msg(usbprog_handle, USB_VENDOR_REQUEST, USER_INTERFACE, LED_OFF, 0, NULL, 8, 1000);
		return 1;
	} else {
		/* unkown value */
		return -1;
	}
}


/* value = kHz (6 = kHz, 5000 = 5 MHz) */
int usbprog_speed(usb_dev_handle * usbprog_handle, short value) 
{
	if( value > 0 && value < 0x1388 ) {
		usb_control_msg(usbprog_handle, USB_VENDOR_REQUEST, SET_SPEED, value, 0, NULL, 8, 1000);
		return 1;
	} else {
		/* speed to slow or to large */
		return -1;	    
	}
}


   

// vim: set sw=8 ts=8 fdm=marker et:
