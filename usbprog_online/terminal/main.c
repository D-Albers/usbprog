#include <stdio.h>
#include <usb.h>
#include "../lib/avrupdate.h"


int main(int argc, char **argv)
{
	
  	// start with the destination of your bin file
	
 	struct usb_dev_handle* usb_handle;


avrupdate_find_usbdevice();
/*

	avrupdate_start_with_vendor_request(0x03eb,0x2104);
	
	sleep(2);

	usb_handle = avrupdate_open(0x0400,0x5dc3);
 
  	if(argc==2)
    	avrupdate_flash_bin(usb_handle,argv[1]);


  	avrupdate_startapp(usb_handle);
  	avrupdate_close(usb_handle);
*/

/*
	char *url = "http://www.ixbat.de/versions.conf";
	printf("Found %i version(s)\n",avrupdate_net_versions(url));	

	struct avrupdate_info *tmp = avrupdate_net_get_version_info(url,1);

	printf("titel %s\n",tmp->title);
	printf("version %s\n",tmp->version);
	printf("file %s\n",tmp->file);
	printf("description %s\n",tmp->description);


	avrupdate_net_flash_version(url,0);
*/	

  	return 0;
}
