#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <signal.h>



#define DEVICE_ADDR 0x16
#define I2C_BUS "/dev/i2c-1"

typedef uint8_t u8;

#define ON 1
#define OFF 0

int fd; 

void setup_I2C() {
        if ((fd = open(I2C_BUS, O_RDWR)) < 0) 
        {
                exit(1);
        }
        if (ioctl(fd, I2C_SLAVE, DEVICE_ADDR) < 0) {
                exit(1);
        }
}


void write_u8(int reg, u8 data) 
{
        u8 arr[2] = {reg, data};

        if (write(fd, arr, 2) != 2) {
                printf("write err\n");
        }
}

void write_array(int reg, u8 *data, int length) 
{
        u8 *arr = (u8*)malloc(sizeof(u8) * length + 1);
        arr[0] = reg;

        for (int i = 0; i < length; i++) {
                arr[i + 1] = data[i];
        }

        if (write(fd, arr, length + 1) != length + 1) {
                printf("write_array I2C error\n");
        }
        free(arr);
}


void request_car(u8 dir_l, u8 speed_l, u8 dir_r, u8 speed_r) 
{
        u8 data[4] = {dir_l, speed_l, dir_r, speed_r};
        write_array(0x01, data, 4);
}


void control_car(int speed1, int speed2) 
{
        u8 dir1 = (speed1 < 0) ? OFF : ON;
        u8 dir2 = (speed2 < 0) ? OFF : ON;
        request_car(dir1, abs(speed1), dir2, abs(speed2));
}


void car_forward(u8 speed1, u8 speed2) 
{
        request_car(ON, speed1, ON, speed2);
}


void car_stop() 
{
        write_u8(0x02, 0x00);
}


void car_back(u8 speed1, u8 speed2) 
{
        request_car(OFF, speed1, OFF, speed2);
}


void car_left(u8 speed1, u8 speed2) 
{
        request_car(OFF, speed1, ON, speed2);
}


void car_right(u8 speed1, u8 speed2) 
{
        request_car(ON, speed1, OFF, speed2);
}


void ctrl_servo(u8 id, u8 angle) 
{
        if (angle < 0) angle = 0;
        else if (angle > 180) angle = 180;

        u8 data[2] = {id, angle};
        write_array(0x03, data, 2);
}

#define IDX_PIN_IR_L1 0
#define IDX_PIN_IR_L2 1
#define IDX_PIN_IR_R1 2 
#define IDX_PIN_IR_R2 3

const static u8 PIN_IRs[4] = {2, 3, 0, 7};
u8 IR_data = 0;
u8 IR_data_arr[4] = {0,};
void setup_IR() 
{
        for (int i = 0; i < 4; i++) {
                pinMode(PIN_IRs[i], INPUT);
        }
}
void read_IR()
{
	IR_data = 0;
        for (int i = 0; i < 4; i++) {
                IR_data += digitalRead(PIN_IRs[i]) << (3-i);

		IR_data_arr[i] = digitalRead(PIN_IRs[i]);
                printf("Sensor Value %d : %d\n", i, IR_data_arr[i]);
	}
	printf("READ IR_data : 0x%x\n", IR_data);
}

void handler(int sig){
        car_stop();

        exit(0);
}


#define BLACK 0
#define WHITE 1

#define PATTERN_FORWARD 0x6 // 0110
#define PATTERN_RIGHT_L  0xe // 1110
#define PATTERN_RIGHT_S  0x4 // 0100
#define PATTERN_LEFT_L 0x7 // 0111
#define PATTERN_LEFT_S 0x2 // 0010
#define PATTERN_STOP    0x0 // 0000

#define PATTERN_RIGHT_N	0x8 // 1000
#define PATTERN_LEFT_N 0x1 // 0001


#define HYPER_SPEED	100
#define HIGH_SPEED	 50
#define LOW_SPEED	 30
#define ZERO_SPEED	 0

#define FLAG_RIGHT 0x0
#define FLAG_LEFT  0x1

u8 flag_ortho = OFF;
u8 flag_dir;

u8 right_speed = 0;
u8 left_speed = 0;
#define WEIGHT_MID  50
#define WEIGHT_SIDE 80
void proc_pattern_new()
{
        
        // if (flag_ortho == ON  ) {
        //         printf("flag_ortho\n");
        //         if (IR_data_arr[0] == WHITE && IR_data_arr[1] == BLACK && IR_data_arr[2] == BLACK && IR_data_arr[3] == WHITE){
        //                 flag_ortho = OFF;
        //         }
        //         if ( ((IR_data_arr[0] == BLACK && IR_data_arr[1] == WHITE && IR_data_arr[2] == WHITE && IR_data_arr[3] == WHITE) && flag_dir == FLAG_LEFT) 
        //              || ((IR_data_arr[0] == WHITE && IR_data_arr[1] == WHITE && IR_data_arr[2] == WHITE && IR_data_arr[3] == BLACK) && flag_dir == FLAG_RIGHT)) {
        //                 flag_ortho = OFF;
        //         }
        //         else {
        //                 return;
        //         }
        // }
        
	right_speed = 0;
	left_speed = 0;
	if (IR_data_arr[0] == BLACK) right_speed += WEIGHT_SIDE;
	if (IR_data_arr[1] == BLACK) right_speed += WEIGHT_MID;
	if (IR_data_arr[2] == BLACK) left_speed  += WEIGHT_MID;
	if (IR_data_arr[3] == BLACK) left_speed  += WEIGHT_SIDE;

        printf("left speed : %d\n", left_speed);
        printf("right speed : %d\n", right_speed);

        if (right_speed + left_speed == 0) {
                printf("car back\n");
                car_forward(HIGH_SPEED, HIGH_SPEED);
                //usleep(1000 * 200);
                return ;
        }

        if (right_speed + left_speed == (WEIGHT_MID + WEIGHT_SIDE) * 2) car_forward(HIGH_SPEED, HIGH_SPEED);

        right_speed = (right_speed < left_speed) ? 0 : right_speed;
        left_speed  = (left_speed  < right_speed) ? 0 : left_speed;
        
        car_forward(left_speed, right_speed);

        if (right_speed == (WEIGHT_MID + WEIGHT_SIDE)) {
                usleep(1000 * 630);
                flag_ortho == ON;
                flag_dir = FLAG_RIGHT;
        }
        if (left_speed == (WEIGHT_MID + WEIGHT_SIDE)) {
                usleep(1000 * 630);
                flag_ortho == ON;
                flag_dir = FLAG_LEFT;
        }
	if (right_speed == (WEIGHT_SIDE) || left_speed == (WEIGHT_SIDE)){
                usleep(1000 * 200);
        }

}
void proc_pattern()
{
	if( flag_ortho == ON && IR_data != PATTERN_FORWARD) {
		IR_data = (flag_dir == FLAG_RIGHT) ? PATTERN_LEFT_L : PATTERN_RIGHT_L;
	} 

	printf("PROCESSED IR DATA : 0x%x\n");
        switch (IR_data) {
        case PATTERN_STOP   :
                car_back(HIGH_SPEED, HIGH_SPEED);
                break;
        case PATTERN_FORWARD:
                car_forward(HIGH_SPEED, HIGH_SPEED);        
        	break;
        case PATTERN_LEFT_L :
                car_left(HYPER_SPEED, HYPER_SPEED);
		flag_ortho = ON;
		flag_dir   =  FLAG_LEFT;
                return;
		break;
	case PATTERN_LEFT_N :
		car_left(HIGH_SPEED, HIGH_SPEED);
		break;
        case PATTERN_LEFT_S :
                car_forward(LOW_SPEED, HIGH_SPEED);
                break;
        case PATTERN_RIGHT_L:
                car_right(HYPER_SPEED, HYPER_SPEED);
                flag_ortho = ON;
		flag_dir   = FLAG_RIGHT; 
		return;
		break;
	case PATTERN_RIGHT_N:
		car_right(HIGH_SPEED, HIGH_SPEED);
		break;
        case PATTERN_RIGHT_S:
                car_forward(HIGH_SPEED, LOW_SPEED);
                break;
        default:
                car_forward(LOW_SPEED, LOW_SPEED);
        }
	flag_ortho = OFF;
	sleep(1);
}

int main() {
        signal(SIGINT, handler);
        if (wiringPiSetup() == -1) {
                printf("Unable to initialize WiringPi library\n");
                return 1;
        }
        setup_IR();
        setup_I2C(); 
        car_stop();

        while(1) {
                usleep(1000 * 10);
                read_IR();
                proc_pattern_new();
        }

        close(fd);
        return 0;
}