/* 
Jacob Mickiewicz 3/25/15
a program for an attiny85 to be an I2C slave device

On read returns function.
On write changes the current funtion or i2c adress.
This is based off the https://github.com/rambo/TinyWire examples
*/

#include <TinyWireS.h> // from https://github.com/rambo/TinyWire

#define I2C_SLAVE_ADDRESS 2 //in use it'll be set by another program (local bus code) can also be set over the i2c bus
#define BLOCK_FUNCTION 0x11 //in phase 2 will be set by another program (adc ->function code) can also be set over the i2c bus
/* list of functions by number
0x11 this test
*/

volatile uint8_t State; //this keeps track of the state or mode of the block
volatile uint8_t Block_Function = BLOCK_FUNCTION; //this is the lexical function 

void requestEvent()  //this runs when a read is detected for address
{  
  switch (State)
  {
  case 0: //reading the function, default
  default:
    TinyWireS.send(Block_Function);
    break; 
  }
}

void receiveEvent(uint8_t HowMany) //this runs when a write is detected for address 
{                                  //flow is slave_write then state_# then value. 3 bytes to changes omething
    switch (State)
    {
      case 2: //changing slave address
        TinyWireS.begin(TinyWireS.receive());
        State = 0;
        break;
      case 1: //setting a new function
        Block_Function = TinyWireS.receive();
        State = 0;
        break;
      case 0: //setting a new state, default
      default:       
        State = TinyWireS.receive();
        break;
    }
}

void I2C_setup(uint8_t slave_add)
{
     TinyWireS.begin(slave_add);
}


void setup() 
{
     I2C_setup(I2C_SLAVE_ADDRESS);
     TinyWireS.onReceive(receiveEvent);
     TinyWireS.onRequest(requestEvent);

}

void loop() 
{
    TinyWireS_stop_check();

}
