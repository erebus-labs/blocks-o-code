/*proto types for i2c_slave.c
Jacob Mickiewicz
*/


#include <inttypes.h>

typedef uint8_t (*GetData_ptr)(uint8_t);

void Demo_Function_Select(void);
void WDT_on(void);
void requestEvent_i2c(void);
void receiveEvent_i2c(uint8_t Howmany);
void I2C_setup(uint8_t slave_add);
void setup_i2c(uint8_t slave_add, GetData_ptr getDataFunc);
int loop_i2c(void);

