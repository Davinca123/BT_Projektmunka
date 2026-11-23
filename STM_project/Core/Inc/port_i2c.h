/*
 * port_i2c.h
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#ifndef INC_PORT_I2C_H_
#define INC_PORT_I2C_H_

#include <stdint.h>

extern int port_i2c_transmit(uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* pdata, uint16_t size, uint32_t timeout);
extern int port_i2c_receive(uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* pdata, uint16_t size, uint32_t timeout);

#endif /* INC_PORT_I2C_H_ */
