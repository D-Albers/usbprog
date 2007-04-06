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

#include "jtagice2.h" 
#include "jtag_avr_prg.h" 
#include "uart.h"
//#include "jtag.h"

#include "../usbn2mc/fifo.h"


// represent acutal state of state machine
static JTAGICE_STATE jtagicestate;

// actuall message
volatile struct message_t msg;

int cmd_get_sign_on(char *msg, char * answer)
{
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x1c;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= RSP_SELFTEST;		// page 57 datasheet
	answer[9]	= 0x01;	// communication protocoll version
	answer[10] = 0xff;	
	answer[11] = 0x07;
	answer[12] = 0x04;
	answer[13] = 0x00;
	answer[14] = 0xff;
	answer[15] = 0x14;
	answer[16] = 0x04;
	answer[17] = 0x00;
	answer[18] = 0x00;

	answer[19] = 0xa0;	// serial number
	answer[20] = 0x00;
	answer[21] = 0x00;
	answer[22] = 0x0d;
	answer[23] = 0x3f;	// end of serial number

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
	crc16_append(answer, (unsigned long)36);
	return 38;
}

int cmd_sign_off(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}


int cmd_set_parameter(char *msg, char * answer)
{
	switch(msg[9]) {

		case EMULATOR_MODE:
			jtagice.emulatormode = answer[10];
			answer[0] = MESSAGE_START;
			answer[1] = jtagice.seq1;
			answer[2] = jtagice.seq2;
			answer[3] = 0x01;					// length of body
			answer[4] = 0;
			answer[5] = 0;
			answer[6] = 0;
			answer[7] = TOKEN;

			answer[8]	= 0x80;		// page 57 datasheet 0xab = no target power (0x80 = ok)
			crc16_append(answer,(unsigned long)9);
			return 11;

		break;
		default:
			;
	}

}


int cmd_get_parameter(char *msg, char * answer)
{

	char jtagbuf[4];
	// get id from target controller over jtag connection
	idcode(jtagbuf);
	
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x05;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x81;		// (0x80 = ok)

	answer[9] = jtagbuf[0];
	answer[10] = jtagbuf[1];
	answer[11] = jtagbuf[2];
	answer[12] = jtagbuf[3];
	crc16_append(answer,(unsigned long)13);
	return 15;
}


int cmd_read_pc(char *msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x05;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x84;		// (0x80 = ok)
	
	answer[9] = 0x00;
	answer[10]= 0x00;
	answer[11]= 0x00;
	answer[12]= 0x00;

	crc16_append(answer,(unsigned long)13);
	return 15;
}

int cmd_set_break(char *msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;

}


int cmd_single_step(char *msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;

}


int cmd_forced_stop(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_set_device_descriptor(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_go(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_restore_target(char * msg, char * answer)
{
	avr_reset(0);	// clear reset mode
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_enter_progmode(char * msg, char * answer)
{
	avr_reset(1);
	avr_prog_enable();
	avr_prog_cmd();

	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_leave_progmode(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}


int cmd_reset(char * msg, char * answer)
{
	avr_reset(1); // force target controller into reset state

	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}


int cmd_read_memory(char * msg, char * answer)
{
	int length=8;
	int msglen=0;
	unsigned long len,startaddr;	// length
	
	// byte 2,3,4 of length = default value
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;


	char jtagbuf[6];
	//SendHex(msg[15]);
	switch(msg[15]) {
		case LOCK_BITS:
			//SendHex(0xff);
			msglen=2;
			answer[3] = 2;					// length of body with ok
			answer[9]	= rd_lock_avr();		// (lock bits)
		break;

		case FUSE_BITS:
			msglen=3;
			answer[3] = 4;					// length of body with ok
						
			answer[9]		= rd_lfuse_avr();			// (lfuse)
			answer[10]	= rd_hfuse_avr();		// (hfuse)
			answer[11]	= rd_efuse_avr();		// (efuse)
		break;


		case SRAM:
			// flash lesen 
			len = msg[16]+(255*msg[17])+(512*msg[18])+(1024*msg[19]);	// length
			//startaddress
			startaddr = msg[20]+(255*msg[21])+(512*msg[22])+(1024*msg[23]);	// length
			answer[3] = (char)msg[16]+1;					// length of body with ok
			msglen=(int)msg[16]+1;
			//SendHex(msg[16]);

		break;

		case SPM:
			// flash lesen 
			len = msg[16]+(255*msg[17])+(512*msg[18])+(1024*msg[19]);	// length
			//startaddress
			startaddr = msg[20]+(255*msg[21])+(512*msg[22])+(1024*msg[23]);	// length
			
			// read data from flash and send back
			//answer[3] = (char)(startaddr+1);					// length of body with ok
			answer[3] = (char)msg[16]+1;					// length of body with ok
			msglen=(int)msg[16]+1;

			//answer[9] - ...

		break;

		default:
			//SendHex(0x88);
	}
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	
	answer[7] = TOKEN;

	answer[8]	= 0x82;		// (0x80 = ok)

	length = length+msglen;
	crc16_append(answer,(unsigned long)length);
	return length+2;
}

