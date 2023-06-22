#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define N_tel 3
#define N_cash 2
#define N_seat 10
#define N_zone_A 10
#define N_zone_B 20
#define P_zone_A 0.3f
#define C_zone_A 30.0f
#define C_zone_B 20.0f
#define N_seat_low 1
#define N_seat_high 5
#define t_res_low 1
#define t_res_high 5
#define t_seat_low 5
#define t_seat_high 13
#define t_cash_low 4
#define t_cash_high 8
#define P_card_success 0.9f

int N_cust;
int seed;

float account_balance;
int remaining_N_tel;
int remaining_N_cash;

int** zone_A;
int** zone_B;

int total_customer_waiting_time;
int total_service_time;

int total_successful_res;
int total_unsuccessful_res_seat;
int total_unsuccessful_res_card;

pthread_mutex_t mutex_tel;
pthread_mutex_t mutex_cash;
pthread_mutex_t mutex_search_A;
pthread_mutex_t mutex_search_B;
pthread_mutex_t mutex_seats_A;
pthread_mutex_t mutex_seats_B;
pthread_mutex_t mutex_customer_waiting_time;
pthread_mutex_t mutex_service_time;
pthread_mutex_t mutex_unsuccessful_res_seat;
pthread_mutex_t mutex_unsuccessful_res_card;
pthread_mutex_t mutex_successful_res;
pthread_mutex_t mutex_account_balance;
pthread_mutex_t mutex_print_console;

pthread_cond_t cond_tel;
pthread_cond_t cond_cash;