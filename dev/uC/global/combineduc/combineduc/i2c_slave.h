/*proto types for i2c_slave.c
Jacob Mickiewicz
*/


#include <inttypes.h>

void Demo_Function_Select();
void WDT_on();
void requestEvent_i2c();
void receiveEvent_i2c(uint8_t Howmany);
void I2C_setup(uint8_t slave_add);
void setup_i2c(uint8_t slave_add);
int loop_i2c();