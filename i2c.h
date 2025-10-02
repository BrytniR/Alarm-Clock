#ifndef I2C_H
#define I2C_H

#include <avr/io.h>

void I2C_set_internal_pullups(void);
void I2C_init(const uint32_t scl_speed);
void I2C_read(const uint8_t address, uint8_t * data, const uint8_t data_size);
void I2C_write(const uint8_t address, const uint8_t * data, const uint8_t data_size);

#endif