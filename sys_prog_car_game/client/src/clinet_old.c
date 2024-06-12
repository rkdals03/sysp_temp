#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "client.h"
#include "handle_car.h"
#include "qrdetector.h"
#define SERVER_IP "127.0.0.1"

int sock;
ClientAction cAction;

sem_t map_lock;
sem_t map_t_lock;
MAP_t map_s;
pthread_t pthread_s;
#define FLAG_OFF 0x0
#define FLAG_ON 0x1
u8 flag_recv = FLAG_OFF;
u8 flag_send = FLAG_OFF;
pid_t pid_s;

void* handle_receive (void* arg) 
{
        printf("recive\n");
        DGIST* game_info_s = (DGIST*)arg;
        u8 cnt_send_lock = 0;
        while (1) {
                // sem_wait(&map_lock)
                
                flag_recv = FLAG_ON;
                printf("recive qr\n");
                if (recv(sock, game_info_s, sizeof(DGIST), 0) == 0) {
                        perror("recieve error");
                        exit(EXIT_FAILURE);
                }
                
                printf("flag_send : %d\n", flag_send);
                if (flag_send == FLAG_OFF) {
                        if (cnt_send_lock++ > 1) flag_send = FLAG_OFF;
                        continue;
                }
                flag_send = FLAG_OFF;

                printf("recived!!\n");

                for (int i = 0; i < MAP_ROW; i++) {
                        for (int j = 0; j < MAP_COL; j++) {
                                map_s.map[i][j].row = game_info_s->map[i][j].row;
                                map_s.map[i][j].col = game_info_s->map[i][j].col;
                                map_s.map[i][j].item = game_info_s->map[i][j].item.score;
                        }
                }

                // sem_post(&map_lock);
                handle_car(&map_s);
                //printMap(game_info_s);
                cnt_send_lock = 0;
        }
}

void* handle_qrdetector (void* arg)
{
        while (1) {
                printf("QR detect start\n");
                int result = start_qr_code_detection();
                printf("QR result : %d\n", result);

                sem_wait(&map_t_lock);
                if(cAction.row == result / 10 && cAction.col == result % 10) continue;
                cAction.row = result / 10;
                cAction.col = result % 10;
                map_s.start_row = cAction.row;
                map_s.start_col = cAction.col;
                cAction.action = move;
                sem_post(&map_t_lock);

                while (1) {
                        printf("flag_recv : %d\n", flag_recv);
                        if (flag_recv == FLAG_ON) break;
                }
                flag_recv = FLAG_OFF;
                
                printf("Send QR %d\n", result);
                if (send(sock, &cAction, sizeof(ClientAction), 0) == -1) {
                        perror("send() error");
                }
                flag_send = FLAG_ON;
        }
}

int main(int argc, char** argv) 
{
        struct sockaddr_in address;

        // sem_init(&map_lock, 0, 1);
        sem_init(&map_t_lock, 0, 1);

        // parse opt ===============
	const int PORT = atoi(argv[1]);
        char server_ip[30];
        strcpy(server_ip, argv[2]);
        u8 init_pos = atoi(argv[3]);
        if (init_pos % 10 == 0) init_direction(UP);
        else init_direction(LEFT);

        cAction.row = -1;
        cAction.col = -1;
        // ==========================

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("sock failed");
                exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        if (inet_pton(AF_INET, SERVER_IP, &address.sin_addr) <= 0) {
                perror("inet_pton failed");
                exit(EXIT_FAILURE);
        }


        if (connect(sock, (struct sockaddr*)&address, sizeof(address)) == -1) {
                perror("connect failed");
                exit(EXIT_FAILURE);
        }

        printf("SUCCESS Connecting : %s\n", SERVER_IP);


        DGIST dgist;
        memset(&dgist, 0, sizeof(DGIST));
        pthread_create(&pthread_s, NULL, handle_qrdetector, NULL);
        handle_receive((void*)&dgist);
        
        
        
        pthread_join(pthread_s, NULL);
}

void send_cAction() {
        if (send(sock, &cAction, sizeof(ClientAction), 0) == -1) {
                perror("send() error");
        }
}


// void printMap(void* arg) 