#ifndef __HANDLE_CAR_H__
#define __HANDLE_CAR_H__

#include "algorithm.h"
typedef uint8_t u8;

void setup_IR();

void setup_I2C();
void write_array(int reg, u8 *data, int length) ;

void write_u8(int reg, u8 data) ;

void request_car(u8 dir_l, u8 speed_l, u8 dir_r, u8 speed_r) ;
void init_direction(Direction dir);

void read_IR();

void car_forward(u8 speed1, u8 speed2);

void car_stop() ;

void handle_car (MAP_t* map_s);

#endif