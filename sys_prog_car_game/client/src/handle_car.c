#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "handle_car.h"

#define DEVICE_ADDR 0x16
#define I2C_BUS "/dev/i2c-1"



#define ON 1
#define OFF 0

int fd; 
const static u8 PIN_IRs[4] = {2, 3, 0, 7};
u8 IR_data = 0;
u8 IR_data_arr[4] = {0,};

int repeat_count = 0;
Direction current_direction = 0;

int current_position[2] = {0, 0};

static int** best_path;




void setup_IR()
{
    for (int i = 0; i < 4; i++) {
        pinMode(PIN_IRs[i], INPUT);
    }
}

void setup_I2C() {
    if ((fd = open(I2C_BUS, O_RDWR)) < 0) 
    {
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, DEVICE_ADDR) < 0) {
        exit(1);
    }
}

void write_array(int reg, u8 *data, int length) 
{
        //printf("fsck\n");
        u8 *arr = (u8*)malloc(sizeof(u8) * length + 1);
        arr[0] = reg;

        for (int i = 0; i < length; i++) {
                arr[i + 1] = data[i];
                
        }
        // printf("write : %d %d %d %d\n", arr[1], arr[2], arr[3], arr[4]);
        if (write(fd, arr, length + 1) != length + 1) {
                //printf("write_array I2C error\n");
        }
        free(arr);
}

void write_u8(int reg, u8 data) 
{
    u8 arr[2] = {reg, data};

    if (write(fd, arr, 2) != 2) {
        //printf("write err\n");
    }
}

void request_car(u8 dir_l, u8 speed_l, u8 dir_r, u8 speed_r) 
{
        //printf("fuck\n");
        u8 data[4] = {dir_l, speed_l, dir_r, speed_r};
        write_array(0x01, data, 4);
}


void read_IR()
{
	IR_data = 0;
    for (int i = 0; i < 4; i++) {
        IR_data += digitalRead(PIN_IRs[i]) << (3-i);

        IR_data_arr[i] = digitalRead(PIN_IRs[i]);
        //printf("Sensor Value %d : %d\n", i, IR_data_arr[i]);
	}
	////printf("READ IR_data : 0x%x\n", IR_data);
}

void car_forward(u8 speed1, u8 speed2) 
{
    //printf("forward speed1 : %d, speed2 : %d\n", speed1, speed2);
    request_car(ON, speed1, ON, speed2);
}

void car_turn(u8 speed)
{
    request_car(OFF, speed + 10, ON, speed);
}
void car_stop() 
{
        write_u8(0x02, 0x00);
}


#define BLACK 0
#define WHITE 1

#define HYPER_SPEED	100
#define HIGH_SPEED	 50
#define LOW_SPEED	 30
#define ZERO_SPEED	 0

#define LONG_SLEEP 800000
#define SHORT_SLEEP 400000

u8 right_speed = 0;
u8 left_speed = 0;
#define WEIGHT_MID  60
#define WEIGHT_SIDE 90

#define FLAG_ON 0x1
#define FLAG_OFF 0x0
u8 flag_ortho = FLAG_OFF;

void proc_pattern_new()
{
    // if (flag_ortho == FLAG_ON) {
    //     printf("sleep...\n");
    //     usleep(700 * 1000);
    //     flag_ortho = OFF;
    //     return;
    // }

	right_speed = 0;
	left_speed = 0;
	if (IR_data_arr[0] == BLACK) right_speed += WEIGHT_SIDE;
	if (IR_data_arr[1] == BLACK) right_speed += WEIGHT_MID;
	if (IR_data_arr[2] == BLACK) left_speed  += WEIGHT_MID;
	if (IR_data_arr[3] == BLACK) left_speed  += WEIGHT_SIDE;

        // //printf("left speed : %d\n", left_speed);
        // //printf("right speed : %d\n", right_speed);




        if (right_speed == (WEIGHT_MID + WEIGHT_SIDE) || left_speed == (WEIGHT_MID + WEIGHT_SIDE)){
            flag_ortho = FLAG_ON;
            printf("ortho!!!!!!!!!!!!!!!!!11\n");
            if (current_direction == UP){
                if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == 1){
                    current_direction = RIGHT;
                    printf("UP - RIGHT\n");
                    car_forward(WEIGHT_MID + WEIGHT_SIDE, 0);
                    

                }
                else if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == -1){
                    current_direction = LEFT;
                    printf("UP - LEFT\n");
                    car_forward(0, WEIGHT_MID + WEIGHT_SIDE);
                    
                    
                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == 1){
                    current_direction = UP;
                    printf("UP - UP\n");
                    car_forward(WEIGHT_MID, WEIGHT_MID);
                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == -1){
                    current_direction = DOWN;
                    printf("UP - DOWN\n");
                    car_turn(WEIGHT_MID + WEIGHT_SIDE);
                    usleep(1000 * 140);
                }
            }
            else if (current_direction == DOWN){
                if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == 1){
                    current_direction = RIGHT;
                    printf("DOWN - RIGHT\n");
                    car_forward(0, WEIGHT_MID + WEIGHT_SIDE);
                    
                    

                }
                else if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == -1){
                    current_direction = LEFT;
                    printf("DOWN - LEFT\n");
                    car_forward(WEIGHT_MID + WEIGHT_SIDE, 0);
                    
  
                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == -1){
                    current_direction = DOWN;
                    printf("DOWN - DOWN\n");
                    car_forward(WEIGHT_MID, WEIGHT_MID);
                    

                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == 1){
                    current_direction = UP;
                    printf("DOWN - UP\n");
                    car_turn(WEIGHT_MID + WEIGHT_SIDE);
                    usleep(1000 * 140);
                }
            }
            else if (current_direction == RIGHT){
                if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == 1){
                    current_direction = UP;
                    printf("RIGHT - UP\n");
                    car_forward(0 ,WEIGHT_MID + WEIGHT_SIDE);
                    
                    

                }
                else if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == 1){
                    current_direction = RIGHT;
                    printf("RIGHT - RIGHT\n");
                    car_forward(WEIGHT_MID, WEIGHT_MID);
                    

                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == -1){
                    current_direction = DOWN;
                    printf("RIGHT - DOWN\n");
                    car_forward(WEIGHT_MID + WEIGHT_SIDE, 0);
                }
                else if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == -1){
                    current_direction = LEFT;
                    printf("RIGHT - LEFT\n");
                    car_turn(WEIGHT_MID + WEIGHT_SIDE);
                    usleep(1000 * 140);
                }
            }
            else if (current_direction == LEFT){
                if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == -1){
                    current_direction = LEFT;
                    printf("LEFT - LEFT\n");
                    car_forward(WEIGHT_MID, WEIGHT_MID);
                    

                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == 1){
                    current_direction = UP;
                    printf("LEFT - UP\n");
                    car_forward(WEIGHT_MID + WEIGHT_SIDE, 0);
                    

                }
                else if (best_path[repeat_count + 1][1] - best_path[repeat_count][1] == -1){
                    current_direction = DOWN;
                    printf("LEFT - DOWN\n");
                    car_forward(0 ,WEIGHT_MID + WEIGHT_SIDE);
                    

                }
                else if (best_path[repeat_count + 1][0] - best_path[repeat_count][0] == 1){
                    current_direction = RIGHT;
                    printf("LEFT - RIGHT\n");
                    car_turn(WEIGHT_MID + WEIGHT_SIDE);
                    usleep(1000 * 140);
                }
            }
            repeat_count++;
            usleep(630 * 1000);
            return;
        }

        if (right_speed + left_speed == 0) {
                //printf("car back\n");
                car_forward(HIGH_SPEED + 30, HIGH_SPEED + 30);
                //nanosleep(&req, NULL);
                return ;
        }
            // if (right_speed + left_speed == (WEIGHT_MID + WEIGHT_SIDE) * 2) car_forward(HIGH_SPEED, HIGH_SPEED);

        right_speed = (right_speed < left_speed) ? 0 : right_speed;
        left_speed  = (left_speed  < right_speed) ? 0 : left_speed;

        car_forward(left_speed + 10, right_speed + 10);
        

        


        // if (right_speed == (WEIGHT_MID + WEIGHT_SIDE)) {
        //         nanosleep(&req, NULL));
        // }
        // if (left_speed == (WEIGHT_MID + WEIGHT_SIDE)) {
        //         nanosleep(&req, NULL));
        // }
	    if (right_speed == (WEIGHT_SIDE) || left_speed == (WEIGHT_SIDE)){
                usleep(1000 * 30);
        }

}

/*
void* car_run(void* arg)
{
    if (repeat_count == 4){
        // 여기서 정지
        repeat_count = 0;
        dgist = NULL; // dgist 서버에서 받아오기
        int start_row = best_path[4][0], start_col = best_path[4][1];
        int max_score_obtained = getMaxScore(dgist, start_row, start_col);
    }
    nanosleep(&req, NULL)
    read_IR();
    proc_pattern_new();
}*/
void handler(int sig){
        car_stop();

        exit(0);
}

void init_direction(Direction dir) {
    printf("direction init : %d\n", dir);
    current_direction = dir;
}

void handle_car (MAP_t* map_s)
{
    signal(SIGINT, handler);
    if (wiringPiSetup() == -1) {
        //printf("Unable to initialize WiringPi library\n");
        return;
    }

    setup_IR();
    setup_I2C(); 
    car_stop();


    best_path = (int**)malloc(sizeof(int*) * 5);
    for (int i = 0; i < 5; i++) {
        best_path[i] = (int*)malloc(sizeof(int) * 2);
    }
    
    MAP_t* dgist = map_s; // dgist 서버에서 받아오기
    int start_row = map_s->start_row;
    int start_col = map_s->start_col; // 시작 지점 설정
    printf("start path : %d %d\n", map_s->start_row, map_s->start_col);
    int max_score_obtained = getMaxScore(dgist, start_row, start_col, best_path);

    for (int i = 0; i < 5; i++){
            //printf("%d %d\n", best_path[i][0] ,best_path[i][1] ) ;
    }
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 3000000;
    while(1) {
        if (repeat_count == 1){
            // 여기서 정지
            //req.tv_nsec = 700 * 1000 * 1000;
            //nanosleep(&req, NULL);
            // req.tv_nsec = 3000000;
            //usleep(700 * 1000);
            repeat_count = 0;
            
            map_s->start_row = best_path[1][0];
            map_s->start_col = best_path[1][1];
            printf("curr path : %d %d\n", best_path[0][0], best_path[0][1]);
            printf("best path : %d %d\n", best_path[1][0], best_path[1][1]);
            for (int i = 0; i < 5; i++){
                free(best_path[i]);
            }
            free(best_path);
            
            car_stop();
            printf("CAR STOP!\n");
            close(fd);
            return; 
        }
        
        read_IR();
        proc_pattern_new();
        nanosleep(&req, NULL);
    }

    close(fd);
    return;
}