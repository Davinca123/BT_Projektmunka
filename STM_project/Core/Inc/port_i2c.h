/*
 * port_i2c.h
 *
 *  Created on: Nov 16, 2026
 *      Author: hallgato
 */

#ifndef INC_PORT_I2C_H_
#define INC_PORT_I2C_H_


extern int port_i2c_transmit(uint16_t addr, uint8_t* pdata, uint16_t size, uint32_t timeout);

#endif /* INC_PORT_I2C_H_ */
