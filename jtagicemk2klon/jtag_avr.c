#include "jtag_avr.h"
#include "jtag_avr_defines.h"
#include "jtag.h"


void idcode(unsigned char *buf)
{
	// READ IDCODE
	jtag_goto_state(SHIFT_IR);
	unsigned char jtagbuf[1];
	jtagbuf[0]=AVR_IDCODE;
	jtag_write(4,jtagbuf);

	jtag_goto_state(SHIFT_DR);
	jtag_read(32,buf);
}

void bypass(void)
{

}

void avr_reset(int true)
{
	// RESET
	jtag_goto_state(SHIFT_IR);
	unsigned char jtagbuf[1];
	jtagbuf[0]=AVR_RESET;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
	if(true==1)
		jtagbuf[0]=0x1;
	else
		jtagbuf[0]=0x0;
	jtag_write(1,jtagbuf);
}

void avr_prog_enable(void)
{
  // ENABLE PROG
	unsigned char jtagbuf[2];
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_PRG_ENABLE;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
	jtagbuf[0]=0x70;
	jtagbuf[1]=0xA3;
	jtag_write(16,jtagbuf);
}

void avr_prog_cmd(void)
{
	unsigned char jtagbuf[1];
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_PRG_CMDS;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
}


int avr_jtag_instr(unsigned char instr, int delay)
{
	unsigned char jtagbuf[1];
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=instr;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
	return 1;
}




//tdi2 = 7 bit
////tdi1 = 8 bit (write it like it is in the datasheet page ...278...)

void avr_sequence(char tdi2, char tdi1, unsigned char * tdo)
{
	unsigned char jtagbuf[2];
	jtag_goto_state(SHIFT_DR);
	jtagbuf[0]=tdi1;  //  select fuse
	jtagbuf[1]=tdi2;
	jtag_write_and_read(15,jtagbuf,tdo);
	jtag_goto_state(RUN_TEST_IDLE);
}

