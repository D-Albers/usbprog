/* 
   usbprog - Change easy the firmware on the usbprog adapter.

   Copyright (C) 2007 Benedikt Sauter

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "usbprog.h"
#include "xmlParser.h"

extern "C" int http_fetch(const char *url, char **fileBuf);


#define usbprog_error_return(code, str) do {  \
	  usbprog->error_str = str;             \
	  return code;                       \
	} while(0);

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_init(struct usbprog_context *usbprog)
{
  if(usbprog!=NULL) {
    usbprog->error_str	= NULL; 
    usbprog->url	= NULL; 
  } 
  
  usb_init();
}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_close(struct usbprog_context *usbprog)
{

}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_get_numberof_devices(struct usbprog_context *usbprog)
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();
  int i=0;
  
  char vendor[255];
  char product[255];
  int vendorlen=0, productlen=0;

  for (bus = busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next){

	#ifndef _WIN32
	if(dev->descriptor.bDeviceClass==0x09) // hub devices
	  continue;
	#endif

	usb_dev_handle * tmp_handle = usb_open(dev);

	vendor[0]=0x00; product[0]=0x00;

	if(dev->descriptor.idVendor==0x1781 && dev->descriptor.idProduct==0x0c62){
	  sprintf(vendor,"usbprog");
	  sprintf(product,"update mode");
	  vendorlen = strlen(vendor);
	  productlen = strlen(product);
	} else {
	  usb_dev_handle * tmp_handle = usb_open(dev);
	  vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
	  productlen = usb_get_string_simple(tmp_handle, 2, product, 255);
	  usb_close(tmp_handle);
	
	  if(vendorlen<=0) sprintf(vendor,"unkown vendor");
	  if(productlen<=0) sprintf(product,"unkown product");
	}
      

	if(vendorlen<=0 && productlen<=0){
	  continue;
	}

	i++;

	usb_close(tmp_handle);
    }
  }
  return i;
}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_print_devices(struct usbprog_context *usbprog, char** buf)
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();
  int i=0;
  
  char vendor[255];
  char product[255];
  char serial[255];
  int vendorlen=0, productlen=0, seriallen=0;

  for (bus = busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next){

	#ifndef _WIN32
	if(dev->descriptor.bDeviceClass==0x09) // hub devices
	  continue;
	#endif

	if(dev->descriptor.bDescriptorType !=1)
	  continue;

	usb_dev_handle * tmp_handle = usb_open(dev);

	vendor[0]=0x00; product[0]=0x00;serial[0]=0x00;
      
	if(dev->descriptor.idVendor==0x1781 && dev->descriptor.idProduct==0x0c62){
	  sprintf(vendor,"usbprog");
	  sprintf(product,"update mode");
	  vendorlen = strlen(vendor);
	  productlen = strlen(product);
	} else {
	  usb_dev_handle * tmp_handle = usb_open(dev);
	  vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
	  productlen = usb_get_string_simple(tmp_handle, 2, product, 255);
	  seriallen = usb_get_string_simple(tmp_handle, 3, serial, 255);
	  usb_close(tmp_handle);
	
	  if(vendorlen<=0) sprintf(vendor,"unkown vendor");
	  if(productlen<=0) sprintf(product,"unkown product");
	  if(seriallen<=0) sprintf(serial,"none");

	}
      

	if(vendorlen<=0 && productlen<=0){
	  continue;
	}


	char * complete = (char*)malloc(sizeof(char)*(strlen(vendor)+strlen(product)+strlen(serial)+30)); 
	//sprintf(complete,"(%i) %s from %s (Serial: %s)%i:%i",i,product,vendor,serial, \
	  //dev->descriptor.idVendor,dev->descriptor.idProduct);
	sprintf(complete,"%s %s %i",vendor,product,i);
	buf[i++]=complete;
	usb_close(tmp_handle);
    }
  }

}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_online_get_netlist(struct usbprog_context *usbprog,char *url)
{
  int result =  http_fetch(url, &(usbprog->versions_xml));
  if(result >=0) {
    usbprog->xMainNode=XMLNode::parseString(usbprog->versions_xml,"usbprog");
    return result;
  }
  return -1;
}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_online_numberof_firmwares(struct usbprog_context* usbprog)
{
  XMLNode xNode=usbprog->xMainNode.getChildNode("pool");
  int n = xNode.nChildNode("firmware");
  if(n)
      return n;
  return -1;
}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_online_print_netlist(struct usbprog_context* usbprog, char** buf, int numberof_firmwares)
{
  XMLNode xNode=usbprog->xMainNode.getChildNode("pool");
  for (int i=0; i<numberof_firmwares; i++){
    char * complete = (char*) malloc(sizeof(char)*255);
    sprintf(complete,"%s",xNode.getChildNode("firmware",i).getAttribute("label"));
    buf[i]=complete;
  }
}





/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *         \param number index of device_list arrar (from usbprog_print_devicelist)
 *
 *         \retval Pointer to error string 
 */
int usbprog_update_mode_number(struct usbprog_context* usbprog, int number)
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();
  int i=0;
  
  char vendor[255];
  char product[255];
  int vendorlen=0, productlen=0;

  for (bus = busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next){

      #ifndef _WIN32
      if(dev->descriptor.bDeviceClass==0x09) // hub devices
	continue;
      #endif

      if(dev->descriptor.bDescriptorType !=1)
	continue;


      vendor[0]=0x00; product[0]=0x00;
           
      if(dev->descriptor.idVendor==0x1781 && dev->descriptor.idProduct==0x0c62){
        sprintf(vendor,"usbprog");
	sprintf(product,"update mode");
	vendorlen = strlen(vendor);
	productlen = strlen(product);
      } else {
	usb_dev_handle * tmp_handle = usb_open(dev);
	vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
	productlen = usb_get_string_simple(tmp_handle, 2, product, 255);
	usb_close(tmp_handle);
      }
      

      if(vendorlen<=0 && productlen<=0){
	continue;
      }

      //printf("open %i %i %i %i\n",i,number,dev->descriptor.idVendor, dev->descriptor.idProduct);

      if(i==number){

	if(!(dev->descriptor.idVendor==0x1781 && dev->descriptor.idProduct==0x0c62))
	{
	  usb_dev_handle * tmp_handle = usb_open(dev);
	  usb_set_configuration(tmp_handle,dev->config[0].bConfigurationValue);
	  usb_claim_interface(tmp_handle,0);

	  usb_control_msg(tmp_handle, 0xC0, 0x01, 0, 0, NULL,8, 10);
	  usb_close(tmp_handle);
	  #if _WIN32
	  Sleep(7000);
	  #else
	  usleep(3000);
	  #endif


	  // find update and open
	  int timeout = 30;
	  while(1){
	    usb_find_busses();
	    usb_find_devices();
	    busses = usb_get_busses();
	    for (bus = busses; bus; bus = bus->next) {
	      for (dev = bus->devices; dev; dev = dev->next){
		if(dev->descriptor.idVendor==0x1781 && dev->descriptor.idProduct==0x0c62){
		  usbprog->usb_handle = usb_open(dev);
		  return 1;
		}
	      }
	    }
	    timeout++;
	    if(timeout>30){
	      printf("timeout");
	      return -1;
	    }
	  }
	} else {
	  usbprog->usb_handle = usb_open(dev);
	  return 0;
	}
      }
      i++;
    }
  }
  return 0;
}

	
/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *         \param number index of device_list arrar (from usbprog_print_devicelist)
 *
 *         \retval Pointer to error string 
 */
int usbprog_flash_firmware(struct usbprog_context* usbprog, char *file)
{
  FILE *fd;
  // open bin file
  fd = fopen(file, "r+b");
  if(!fd) {
    printf("Unable to open file %s, ignoring.\n", file);
  } else {

    struct stat buf;
    stat(file,&buf);
    long filesize = buf.st_size;
    printf("filesize %i\n",filesize);
    
    char * buffer = (char*)malloc((sizeof(char)*filesize)+sizeof(char));
    int i=0;

    while(!feof(fd)) {
      buffer[i++] = (char)fgetc(fd);
    }
    usbprog_flash_buffer(usbprog,buffer,i);
  }
  fclose(fd);
}

int usbprog_flash_buffer(struct usbprog_context* usbprog, char *buffer, int len)
{
  int offset = 0;
  int index=0;
  int page=0;

  char buf[64];
  char cmd[64];
  
  for(index=0;index<len;index++)
  {
    buf[offset]=buffer[index];

    offset++;
    if(offset == 64)
    {
        //printf("send package\n");
        // command message
        cmd[0]=WRITEPAGE;
        cmd[1]=(char)page; // page number
        usb_bulk_write(usbprog->usb_handle,2,cmd,64,100);

        // data message
        usb_bulk_write(usbprog->usb_handle,2,buf,64,100);
        offset = 0;
        page++;
    }
  }
  if(offset > 0)
  {
    //printf("rest\n");
    // command message
    cmd[0]=WRITEPAGE;
    cmd[1]=(char)page; // page number
    usb_bulk_write(usbprog->usb_handle,2,cmd,64,100);

    // data message
    usb_bulk_write(usbprog->usb_handle,2,buf,64,100);
  }
}



/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_stop_updatemode(struct usbprog_context* usbprog)
{
  char buf[64];
  char *ptr = buf;

  buf[0]=STARTAPP;
  usb_bulk_write(usbprog->usb_handle,2,ptr,64,100);
}




/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
char *usbprog_get_error_string (struct usbprog_context *usbprog)
{     
  return usbprog->error_str;
}

