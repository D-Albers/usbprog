#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"


#define LED_PIN     PA4
#define LED_PORT    PORTA


#define LED_on     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_off    (LED_PORT   &= ~(1 << LED_PIN))


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  if(req->bRequest == STARTAVRUPDATE)
      avrupdate_start();
}

volatile int togl;

SIGNAL(SIG_OUTPUT_COMPARE1A)
{
  if(togl==1)
  {
    PORTA = 0xFF;
    togl=0;
  }
  else {
    PORTA = 0x00;
    togl=1;
  }
}

void Commands(char * buf)
{


}


int main(void)
{
  int conf, interf;
  
  //UARTInit();
  
  USBNInit();   

  DDRA = (1 << PA4); // status led


  // setup your usbn device

  USBNDeviceVendorID(0x1786);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0007);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceManufacture ("EmbeddedProjects");
  USBNDeviceProduct	("usbprogLogic 250");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&Commands);

  
  USBNInitMC();
  sei();
  USBNStart();

  DDRA=0xff; // testing
  //LED_on;

  //DDRB=0x00; //in port
  //PORTB = 0xff; //internal pull up resistors

  //int datatogl=0;

  while(1);
}

