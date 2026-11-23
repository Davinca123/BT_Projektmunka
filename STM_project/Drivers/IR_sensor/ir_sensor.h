/*
 * pulse_meter.h
 *
 *  Created on: Nov 23, 2026
 *      Author: hallgato
 */

#ifndef IR_SENSOR_IR_SENSOR_H_
#define IR_SENSOR_IR_SENSOR_H_

extern void init_ir_sensor();
#define IR_FIFO_DEPTH 16u // MAX30100 FIFO holds max. 16 samples

// Reads all samples waiting in the FIFO into ir_raw[] / is_finger_on[] (arrays of max_samples elements).
// Returns 0 on success, non-zero on I2C error (outputs are not valid then).
extern int read_ir_sensor(uint16_t* ir_raw, uint8_t* is_finger_on, uint8_t max_samples, uint8_t* num_samples);

#endif /* IR_SENSOR_IR_SENSOR_H_ */
