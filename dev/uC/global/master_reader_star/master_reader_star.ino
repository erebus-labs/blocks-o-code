/*
Jacob Mickiewicz 3/25/15
a program for an arduino to test my attiny85 i2c slave code

a modification of the "wire" examples to take simple serial input for i2c debuging
made to work with my funct_slave code. takes serial comands, and sends i2c bytes.
*/


#include <Wire.h>

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  Serial.print("enter slave address for read, or three numbers for write: 1st the slave adress. Then the option number (2 for change i2c address, 1 for change block value, 0 or anything else for change state). Then number you want that thing to be.");
}

void loop()
{
  delay(500);
  Serial.print("...\n");
}

void serialEvent() {
  uint8_t adr; // the 1 digit i2c slave address
  uint8_t state; // the state to be modified (there may be a better name, but it should be changed in the slave code first)
  uint8_t val; // the new value for the state
  uint8_t ret; // what is serial printed, this is used for different jobs
  Serial.flush();//wait for the whole input
  ret = Serial.peek();//what am I sending? 
  Serial.println(ret);//note: this is only the first byte and it is the dec value of the ascii
  switch(Serial.available())
  {
    case 1://read
      adr= Serial.read();
      Wire.requestFrom((adr-48), 1);    // request 1 byte from slave named (ascii to num trick)
      while (Wire.available())  
      {
        ret = Wire.read(); // receive a byte as number
        Serial.println(ret,HEX);         // print the hex because I used hex in the default function assignment.
        //note: read after right will look fine because 1 digit dec looks that same in hex
      }
      break;
    case 3://write
        adr = Serial.read();
        state = Serial.read();
        val = Serial.read();
        Wire.beginTransmission((adr-48)); // transmit to device named
        Wire.write(state-48);
        Wire.write(val-48);
        ret = Wire.endTransmission();
    /*0:success
    1:data too long to fit in transmit buffer
    2:received NACK on transmit of address
    3:received NACK on transmit of data
    4:other error */
        Serial.println(ret); //did the write work?
        break;
    default://error
        Serial.println("input must be numbers");
        Serial.println("1 digit slave address to read, or");
        Serial.println("1 digit each; slave address then state then value for write");
        Serial.println("be sure line endings are off, no spaces or anything else");
        Serial.println("5 sec wait so you can read this");
        while(Serial.available()){
          Serial.read();//clear the buffer
        }
        delay(5000);
        break;
  }
}
