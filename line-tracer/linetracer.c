#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>

#define PIN_TRACK_L1 2
#define PIN_TRACK_L2 3
#define PIN_TRACK_R1 0
#define PIN_TRACK_R2 7

#define I2C_ADDR 0x16
#define MOTOR_FORWARD 0x01
#define MOTOR_BACKWARD 0x02
#define MOTOR_STOP 0x00


#define NUM_PINS 4

int fd;

static const int PIN_TRACKS[NUM_PINS] = {
	PIN_TRACK_L1,
	PIN_TRACK_L2,
	PIN_TRACK_R1,
	PIN_TRACK_R2,
};

void forward_car() 
{
	printf("move!!!!!!!!1\n");
	if (wiringPiI2CWrite(fd, MOTOR_FORWARD) == -1) printf("fuck\n");
	else printf("forward\n");

}

void stop_car()
{
	wiringPiI2CWrite(fd, MOTOR_STOP);
}

void run_car (int* sensor_values)
{
	if (sensor_values[1] == 0 || sensor_values[2] == 0) {
		forward_car();
	} else {
		stop_car();
	}
}

void read_sensor ()
{
	int sensor_values[NUM_PINS] = {0,};

	for (int i = 0; i < NUM_PINS; i++) {
		sensor_values[i] = digitalRead(PIN_TRACKS[i]);
		printf("%d value : %d\n", i, sensor_values[i]);
	}
	run_car(sensor_values);
}

int main() {
	if (wiringPiSetup()  == -1) {
		printf("Unable to init Wiring Pi lib.\n");
		return 1;
	}
	for (int i = 0; i < NUM_PINS; i++) {
		pinMode(PIN_TRACKS[i], INPUT);
	}
	fd = wiringPiI2CSetup(I2C_ADDR);
	if (fd == -1) {
		printf("fail to init I2C\n");
		return 1;
	}

	while(1) {
		read_sensor();
		delay(100);
	}
	return 0;

}
