/* jtag.h
* Copyright (C) 2007  Benedikt Sauter, sauter@ixbat.de
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

#include "jtag.h"
#include "bit.h"
#include "jtagice2.h"
#include "uart.h"

uint8_t z[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

volatile TAP_STATE tapstate;

void jtag_init(void)
{
	// use as output
	JTAG_PORT_INIT |= (1<<TCK)|(1<<TMS)|(1<<TDI);
	// use as input
	JTAG_PORT_INIT &=~(1<<TDO);

	// pullup
	JTAG_PORT_WRITE |= (1<<TDO);
	//JTAG_PORT_WRITE |= (1<<TDO);

	// use as input
	JTAG_CLEAR_TCK();
	JTAG_CLEAR_TMS();
	JTAG_CLEAR_TDI();
	JTAG_CLEAR_TCK();

	jtag_reset();
}

uint8_t jtag_read_tdo(void)
{
	if(JTAG_IS_TDO_SET())
		return 1;
	else
		return 0;
}

void jtag_send_slice(uint8_t tck, uint8_t tms, uint8_t tdi)
{

	if(tms)
		JTAG_PORT_WRITE |= (1<<TMS);
	else
		JTAG_PORT_WRITE &= ~(1<<TMS);

	if(tdi)
		JTAG_PORT_WRITE |= (1<<TDI);
	else
		JTAG_PORT_WRITE &= ~(1<<TDI);

	if(tck)
		JTAG_PORT_WRITE |= (1<<TCK);
	else
		JTAG_PORT_WRITE &= ~(1<<TCK);

}

int jtag_reset(void)
{
	int i;
	JTAG_SET_TMS();
	for(i=0;i<5;i++) {
		JTAG_CLK();
	}
	JTAG_CLEAR_TMS();
	tapstate = TEST_LOGIC_RESET;

	return 1;
}

uint8_t jtag_read(uint8_t numberofbits, unsigned char * buf)
{
  int receivedbits = 0;
	JTAG_CLEAR_TMS();				// last one with tms
	JTAG_CLK();

  while(numberofbits--) {
		if(numberofbits==0){
	 		JTAG_SET_TMS();				// last one with tms
			if(tapstate==SHIFT_IR){
				tapstate = EXIT1_IR;
			} else {
				tapstate = EXIT1_DR;
			}
		}

		//if(numberofbits==1)
	  //	JTAG_SET_TMS();			// last one with tms

		if(JTAG_IS_TDO_SET())
			buf[receivedbits/8] |= (1 << (receivedbits & 7));
		else
			buf[receivedbits/8] &= ~(1 << (receivedbits & 7));

		receivedbits++;
		JTAG_CLK();
  }

	return receivedbits;
}


/* 
 * NOTE: This function hard-optimized for working into
 * "funcions avr_sequence1, avr_sequence2" context. And IT NOT UNIVERSAL.
 * Function send 15 bits to jtag from "buf" and put bits from TDO to
 * "readbuf", when TAP state is SHIFT_DR. But you can call it from
 * SHIFT_IR TAP state, when you need send and read 15 bits. It ~3 times faster
 * than jtag_write_and_read, but not universal. I apply it in "time critical"
 * operations such as read flash/eeprom, write flash/eeprom. For example:
 * with functions jtag_write + jtag_write_and_read write/read programming
 * cycle on my AT90CAN64 take about 60 sec., and with jtag_write_sequence +
 * jtag_write_and_read_sequence it take about 20 sec.
 *
 */

void jtag_write_and_read_sequence(uint8_t * buf, uint8_t *readbuf)
{
    uint8_t i, x, y;
    /*unsigned short m;*/

#if 0
    /* 
     * NOTE!!! Uncomment if you want to use this funcion in SHIFT_IR TAP
     * state, REPLACE jtagchain.units_after -> after,
     * jtagchain.units_before -> before.
     *
     */

    uint8_t before, after;

    /* units/bits before/after target (JTAG chain) depends on TAP state */
    before = tapstate == SHIFT_IR? jtagchain.bits_before: jtagchain.units_before;
    after = tapstate == SHIFT_IR? jtagchain.bits_after: jtagchain.units_after;

#endif

    readbuf[0] = readbuf[1] = 0;
	JTAG_CLEAR_TMS();				// last one with tms

    for (i = 0; i < jtagchain.units_after; i++) {
        JTAG_SET_TDI();
        JTAG_CLK();
    }
    for (i = 0; i < 2; i++) {
        x = i == 0? 8: 7;
        for (y = 0; y < x; y++) {
            if (jtagchain.units_before == 0 && i == 1 && y == 6)
                JTAG_SET_TMS(); // last one with tms
            if (buf[i] & z[y])
                JTAG_SET_TDI();
            else
                JTAG_CLEAR_TDI();
            JTAG_CLK();

            if(JTAG_IS_TDO_SET())
                readbuf[i] |= z[y];
        }
    }
    for (i = 0; i < jtagchain.units_before; i++) {
        if (i == jtagchain.units_before - 1)
            JTAG_SET_TMS();				// last one with tms
        JTAG_SET_TDI();
        JTAG_CLK();
    }
    tapstate = tapstate == SHIFT_IR? EXIT1_IR: EXIT1_DR;
}


/* 
 * NOTE: This function hard-optimized for working into
 * "funcions avr_sequence1, avr_sequence2" context. And IT NOT UNIVERSAL.
 * Function send 15 bits to jtag from "buf", when TAP state is SHIFT_DR.
 * But you can call it from SHIFT_IR TAP state, when you need send 15 bits.
 * It ~3 times faster than jtag_write, but not universal. I apply it in
 * "time critical" operations such as read flash/eeprom, write flash/eeprom.
 * For example: with functions jtag_write + jtag_write_and_read write/read
 * programming cycle on my AT90CAN64 take about 60 sec., and with
 * jtag_write_sequence + jtag_write_and_read_sequence it take about 20 sec.
 *
 */

void jtag_write_sequence(uint8_t * buf)
{
    uint8_t i;

#if 0
    /* 
     * NOTE!!! Uncomment if you want to use this funcion in SHIFT_IR TAP
     * state, REPLACE jtagchain.units_after -> after,
     * jtagchain.units_before -> before.
     *
     */

    uint8_t before, after;

    /* units/bits before/after target (JTAG chain) depends on TAP state */
    before = tapstate == SHIFT_IR? jtagchain.bits_before: jtagchain.units_before;
    after = tapstate == SHIFT_IR? jtagchain.bits_after: jtagchain.units_after;

#endif

	JTAG_CLEAR_TMS();				// last one with tms

    for (i = 0; i < jtagchain.units_after; i++) {
        JTAG_SET_TDI();
        JTAG_CLK();
    }

    for (i = 0; i < 8; i++) {
        if (buf[0] & z[i])
            JTAG_SET_TDI();
        else
            JTAG_CLEAR_TDI();
        JTAG_CLK();
    }
    for (i = 0; i < 6; i++) {
        if (buf[1] & z[i])
            JTAG_SET_TDI();
        else
            JTAG_CLEAR_TDI();
        JTAG_CLK();
    }
    if (jtagchain.units_before == 0)
        JTAG_SET_TMS(); // last one with tms
   
    if (buf[1] & 0x40)
        JTAG_SET_TDI();
    else
        JTAG_CLEAR_TDI();
    JTAG_CLK();

    for (i = 0; i < jtagchain.units_before; i++) {
        if (i == jtagchain.units_before - 1)
            JTAG_SET_TMS();				// last one with tms
        JTAG_SET_TDI();
        JTAG_CLK();
    }
    tapstate = tapstate == SHIFT_IR? EXIT1_IR: EXIT1_DR;
}

uint8_t jtag_write(uint8_t numberofbits, unsigned char * buf)
{
	int sendbits=0;
    uint8_t i = 0;
    uint8_t before, after;
    
	// if numbers is not vaild
	if(numberofbits<=0)
		return -1;

    /* units/bits before/after target (JTAG chain) depends on TAP state */
    before = tapstate == SHIFT_IR? jtagchain.bits_before: jtagchain.units_before;
    after = tapstate == SHIFT_IR? jtagchain.bits_after: jtagchain.units_after;

	JTAG_CLEAR_TMS();				// last one with tms

    for (i = 0; i < after; i++) {
        JTAG_SET_TDI();
        JTAG_CLK();
    }

	while(numberofbits--)
	{
        if(before == 0 && numberofbits == 0)
             JTAG_SET_TMS();				// last one with tms
		if(buf[sendbits/8] >> (sendbits & 7) & 1)
		{
			JTAG_SET_TDI();
		}
		else
		{
			JTAG_CLEAR_TDI();
		}

		sendbits++;
		JTAG_CLK();
	}
    for (i = 0; i < before; i++) {
        if (i == before - 1)
            JTAG_SET_TMS();				// last one with tms
        JTAG_SET_TDI();
        JTAG_CLK();
    }
    tapstate = tapstate == SHIFT_IR? EXIT1_IR: EXIT1_DR;
	return sendbits;
}

uint8_t jtag_write_and_read(	uint8_t numberofbits,
															unsigned char * buf,
															unsigned char * readbuf)
{
	int bits=0;
    uint8_t i = 0;
    uint8_t before, after;
    
	// if numbers is not vaild
	if(numberofbits<=0)
		return -1;

    /* units/bits before/after target (JTAG chain) depends on TAP state */
    before = tapstate == SHIFT_IR? jtagchain.bits_before: jtagchain.units_before;
    after = tapstate == SHIFT_IR? jtagchain.bits_after: jtagchain.units_after;

	JTAG_CLEAR_TMS();				// last one with tms

    for (i = 0; i < after; i++) {
        JTAG_SET_TDI();
        JTAG_CLK();
    }

  while(numberofbits--) {
        if(before == 0 && numberofbits == 0)
             JTAG_SET_TMS();				// last one with tms
		if(buf[bits/8] >> (bits & 7) & 1)
			JTAG_SET_TDI();
		else
			JTAG_CLEAR_TDI();

		JTAG_CLK();

		if(JTAG_IS_TDO_SET())
			readbuf[bits/8] |= (1 << (bits & 7));
		else
			readbuf[bits/8] &= ~(1 << (bits & 7));


	  bits++;
	}
    for (i = 0; i < before; i++) {
        if (i == before - 1)
            JTAG_SET_TMS();				// last one with tms
        JTAG_SET_TDI();
        JTAG_CLK();
    }
    tapstate = tapstate == SHIFT_IR? EXIT1_IR: EXIT1_DR;
	return bits;
}

uint8_t jtag_bit (uint8_t tms, uint8_t tdi, uint8_t read_tdo)
{
    uint8_t tdo = 0;
    if (tms) {
        JTAG_SET_TMS();
        tapstate = tapstate == SHIFT_IR? EXIT1_IR: EXIT1_DR;
    }
    else
        JTAG_CLEAR_TMS();
    if (tdi)
        JTAG_SET_TDI();
    else
        JTAG_CLEAR_TDI();

    JTAG_CLK();

    if (read_tdo) {
        if (JTAG_IS_TDO_SET())
            tdo = 1;
    }
    return tdo;
}

void jtag_goto_state1(TAP_STATE state)
{
  /* If 'state' is invalid, simply ignore it */
  if( state > UPDATE_IR ) return;
  while( tapstate != state ) {
      switch( tapstate ) {
          case RUN_TEST_IDLE:
              JTAG_SET_TMS();
              JTAG_CLK();
              tapstate = SELECT_DR_SCAN;
              break;

          case SELECT_DR_SCAN:
              JTAG_CLEAR_TMS();
              JTAG_CLK();
              tapstate = CAPTURE_DR;
              break;

          case CAPTURE_DR:
              JTAG_CLEAR_TMS();
              JTAG_CLK();
              tapstate = SHIFT_DR;
              break;

          default:
              return;
      }
  }
}

void jtag_goto_state2(TAP_STATE state)
{

/*case EXIT1_DR:*/
    JTAG_SET_TMS();
    JTAG_CLK();
    /*tapstate = UPDATE_DR;*/
/*break;*/

/*case UPDATE_DR:*/
    JTAG_CLEAR_TMS();
    JTAG_CLK();
    tapstate = RUN_TEST_IDLE;
/*break;*/
}

void jtag_goto_state(TAP_STATE state)
{
  /* If 'state' is invalid, simply ignore it */
  if( state > UPDATE_IR ) return;
	//SendHex(tapstate);
	//SendHex(state);
  while( tapstate != state ) {
  	//if (debug_verbose != 0)
			//SendHex(tapstate);
		switch( tapstate ) {
			case TEST_LOGIC_RESET:
			  JTAG_CLEAR_TMS();
	  		JTAG_CLK();
	  		tapstate = RUN_TEST_IDLE;
	  	break;

			case RUN_TEST_IDLE:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = SELECT_DR_SCAN;
	  	break;

			case SELECT_DR_SCAN:
	  		if( state < TEST_LOGIC_RESET ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = CAPTURE_DR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = SELECT_IR_SCAN;
	    	}
	  	break;

			case CAPTURE_DR:
	  		if( state == SHIFT_DR ){
      		JTAG_CLEAR_TMS();
      		JTAG_CLK();
      		tapstate = SHIFT_DR;
    		} else {
	    		JTAG_SET_TMS();
	    		JTAG_CLK();
	    		tapstate = EXIT1_DR;
	  		}
			break;

			case SHIFT_DR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT1_DR;
	  	break;

			case EXIT1_DR:
	  		if( state == PAUSE_DR || state == EXIT2_DR ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = PAUSE_DR;
	   	 	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_DR;
	    	}
	  	break;

			case PAUSE_DR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT2_DR;
	  	break;

			case EXIT2_DR:
	  		if( state == SHIFT_DR || state == EXIT1_DR || state == PAUSE_DR ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = SHIFT_DR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_DR;
	    	}
	  	break;

			case UPDATE_DR:
	  		if( state == RUN_TEST_IDLE ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = RUN_TEST_IDLE;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = SELECT_DR_SCAN;
	    	}
	  	break;

			case SELECT_IR_SCAN:
	  		if( state != TEST_LOGIC_RESET ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = CAPTURE_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = TEST_LOGIC_RESET;
	    	}
	  	break;

			case CAPTURE_IR:
	  		if( state == SHIFT_IR ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = SHIFT_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = EXIT1_IR;
	    	}
	  	break;

			case SHIFT_IR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT1_IR;
	  	break;

			case EXIT1_IR:
	  		if( state == PAUSE_IR || state == EXIT2_IR ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = PAUSE_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_IR;
	    	}
	  	break;

			case PAUSE_IR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT2_IR;
	  	break;

			case EXIT2_IR:
	  		if( state == SHIFT_IR  || state == EXIT1_IR  || state == PAUSE_IR ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = SHIFT_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_IR;
	    	}
	  	break;

			case UPDATE_IR:
	  		if( state == RUN_TEST_IDLE ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = RUN_TEST_IDLE;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = SELECT_DR_SCAN;
	    	}
	  	break;
			default:
				return;
		}
  }
	return;
}

void jtag_clock_cycles(uint8_t num)
{
	JTAG_CLEAR_TMS();

	while (num--) {
		JTAG_CLK();
	}
}
