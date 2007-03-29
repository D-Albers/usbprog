/*
 * jtagice - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006 Benedikt Sauter 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "wait.h"

#include "../usbprog_base/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

#include "jtag.h"
#include "jtagice2.h"
#include "crc.h"

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);


SIGNAL(SIG_UART_RECV)
{
 //Terminal(UARTGetChar());
 //UARTWrite("usbn>");
}


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}



/* id need for live update of firmware */
void USBNDecodeVendorRequest(DeviceRequest *req)
{
	//UARTWrite("vendor request check ");
	//SendHex(req->bRequest);
	switch(req->bRequest)
	{
		case STARTAVRUPDATE:
			avrupdate_start();
		break;
	}
}

volatile int datatogl = 0;
//volatile char answer[300];

void CommandAnswer(int length)
{
	int i;

 	USBNWrite(TXC1,FLUSH);
	for(i=0;i<length;i++)
		USBNWrite(TXD1,answer[i]);

	//SendHex(0x11);
	/* control togl bit */

	if(datatogl==0) {
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
		datatogl=0;
	} else {
		USBNWrite(TXC1,TX_LAST+TX_EN);
		datatogl=1;
	}
}

/* called after data where send to pc */
void USBSend()
{

}

/* is called when received data from pc */
void USBReceive(char *buf)
{
  USBNWrite(TXC1,FLUSH);
#if 0	
	if(jtagice.longpackage) {

		// recalculate message size
		jtagice.size = jtagice.size -64;
		if(jtagice.size==0)
			jtagice.longpackage = 0;

	} else {
#endif	
		jtagice.seq1=buf[1];		// save sequence number
		jtagice.seq2=buf[2];		// save sequence number
	
		// check if package is a cmdpackage
		if(buf[0]==MESSAGE_START)
			jtagice.cmdpackage=1;
		else
			jtagice.cmdpackage=0;

		// check if package is a longpackage
		jtagice.size = buf[3]+(255*buf[4])+(512*buf[5])+(1024*buf[6]);
		//jtagice.size = buf[3]+(buf[4]<<8)+(buf[5]<<16)+(buf[6]<<24);
		if(jtagice.size>54)
			jtagice.longpackage = 1;
			
	
		int cmdlength;
		switch(buf[8]) {

			case CMND_GET_SIGN_ON:
				//cmdlength = cmd_get_sign_on(buf,NULL);
				  answer[0] = MESSAGE_START;
					  answer[1] = jtagice.seq1;
						  answer[2] = jtagice.seq2;
							  answer[3] = 0x1c;         // length of body
								  answer[4] = 0;
									  answer[5] = 0;
										  answer[6] = 0;
											  answer[7] = TOKEN;

												  answer[8] = RSP_SELFTEST;   // page 57 datasheet
													  answer[9] = 0x01; // communication protocoll version
														  answer[10] = 0xff;
															  answer[11] = 0x07;
																  answer[12] = 0x04;
																	  answer[13] = 0x00;
																		  answer[14] = 0xff;
																			  answer[15] = 0x14;
																				  answer[16] = 0x04;
																					  answer[17] = 0x00;
																						  answer[18] = 0x00;

																							  answer[19] = 0xa0;  // serial number
																								  answer[20] = 0x00;
																									  answer[21] = 0x00;
																										  answer[22] = 0x0d;
																											  answer[23] = 0x3f;  // end of serial number

																												  answer[24] = 'J';
																													  answer[25] = 'T';
																														  answer[26] = 'A';
																															  answer[27] = 'G';
																																  answer[28] = 'I';
																																	  answer[29] = 'C';
																																		  answer[30] = 'E';
																																			  answer[31] = 'm';
																																				  answer[32] = 'k';
																																					  answer[33] = 'I';
																																						  answer[34] = 'I';
																																							  answer[35] = 0x00;
																																								  answer[36] = 0x00;
																																									  answer[37] = 0x00;
																																										  crc16_append(answer,36);

				CommandAnswer(38);

			break;

			case CMND_SET_PARAMETER:
				cmdlength = cmd_set_parameter(buf);
			break;

			case CMND_FORCED_STOP:
				cmdlength = cmd_forced_stop(buf);
			break;

			default:
				answer[0]=RSP_FAILED;
				cmdlength=1;
		}
		// recalculate size
		jtagice.size = jtagice.size -54;
		//CommandAnswer(cmdlength);
	//}
}



int main(void)
{
  int conf, interf;
	// only for testing
  UARTInit();
  
	USBNInit();   
  
  jtagice.longpackage=0;

	DDRA = (1 << DDA4);
	PORTA &= ~(1<<PA4); //off

  USBNDeviceVendorID(0x03eb);	//atmel ids
  USBNDeviceProductID(0x2103); // atmel ids
  
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("JTAGICE mkII Klon");
  USBNDeviceSerialNumber("A000000D3F");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,USBSend);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBReceive);

  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();

	// only for testing

	unsigned char jtagbuf[10];

	jtag_init();

#if 0
	jtag_goto_state(SHIFT_DR);

	char buf[4];
	jtag_read(32,buf);
	
	SendHex(buf[0]);
	SendHex(buf[1]);
	SendHex(buf[2]);
	SendHex(buf[3]);
#endif	

	// ask for new events
	// while send an event block usb receive routine
	while(1);
	// end testing
}


