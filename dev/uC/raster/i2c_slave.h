/*proto types for i2c_slave.c
Jacob Mickiewicz
*/


#include <inttypes.h>

typedef uint8_t (*getData_ptr)(void);
typedef uint8_t (*sendVertical_ptr)(void);
typedef uint8_t (*sendHorizontal_ptr)(void);

void Demo_Function_Select(void);
void WDT_on(void);
void requestEvent_i2c(void);
void receiveEvent_i2c(uint8_t Howmany);
void I2C_setup(uint8_t slave_add);
void setup_i2c(uint8_t slave_add, getData_ptr getDataFunc,sendHorizontal_ptr sendHorizFunc,sendVertical_ptr sendVertFunc,uint8_t X,uint8_t Y);
int loop_i2c(void);

