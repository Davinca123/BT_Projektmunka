/*
 * pulse_meter.h
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#ifndef IR_SENSOR_IR_SENSOR_H_
#define IR_SENSOR_IR_SENSOR_H_

extern void init_ir_sensor();
extern void read_ir_sensor(uint16_t* ir_raw, uint8_t* is_finger_on);

#endif /* IR_SENSOR_IR_SENSOR_H_ */
