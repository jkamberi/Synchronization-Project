#include "p3200016-p3180064.h"

void* routine(void *cust_num) {
	int *cust_n = (int *) cust_num;

	struct timespec time_cust_arrived;
	struct timespec time_tel_start;
	struct timespec time_tel_end;
	struct timespec time_cash_start;
	struct timespec time_cash_end;

	int rc;

	// Customer arrived
	clock_gettime(CLOCK_REALTIME, &time_cust_arrived);
	
	rc = pthread_mutex_lock(&mutex_tel);
	while(remaining_N_tel == 0) {
		rc = pthread_cond_wait(&cond_tel, &mutex_tel);
	}
	remaining_N_tel--;
	rc = pthread_mutex_unlock(&mutex_tel);
	clock_gettime(CLOCK_REALTIME, &time_tel_start);
	// A telephonist was found for the customer

	int zone_n = 1;
	int zone_rows = N_zone_B;
	int** zone = zone_B;
	int unique_seed = seed^(*cust_n);
	if(rand_r(&unique_seed) % 100 + 1 <= P_zone_A*100) {	// 30% probability
		zone_n = 0;
		zone_rows = N_zone_A;
		zone = zone_A;
	} 

	int seats_n = rand_r(&unique_seed) % N_seat_high + N_seat_low;
	int time_to_search = rand_r(&unique_seed) % t_seat_high + t_seat_low;
	sleep(time_to_search);
	if(zone_n == 0) {
		rc = pthread_mutex_lock(&mutex_seats_A);
	} else {
		rc = pthread_mutex_lock(&mutex_seats_B);
	}

	int seats_row = -1;
	int seats_starting_column = -1;
	int count;
	for(int i = 0; i < zone_rows; i++) {
		count = 0;
		for(int j = 0; j < N_seat; j++) {
			if(zone[i][j] == 0) {
				count++;
			} else {
				count = 0;
			}
			if(count == seats_n) {
				seats_row = i;
				seats_starting_column = j-count+1;
				break;
			}
		}
		if(count == seats_n) {
			break;
		}
	}

	if(seats_row != -1) {	// If consecutive seats found
		for(int j = seats_starting_column; j < seats_starting_column + seats_n; j++) {
			zone[seats_row][j] = *cust_n;
		}
	}

	if(zone_n == 0) {
		rc = pthread_mutex_unlock(&mutex_seats_A);
	} else {
		rc = pthread_mutex_unlock(&mutex_seats_B);
	}
	

	clock_gettime(CLOCK_REALTIME, &time_tel_end);
	
	rc = pthread_mutex_lock(&mutex_customer_waiting_time);
	total_customer_waiting_time += (time_tel_start.tv_sec - time_cust_arrived.tv_sec);
	rc = pthread_mutex_unlock(&mutex_customer_waiting_time);

	rc = pthread_mutex_lock(&mutex_tel);
	remaining_N_tel++;
	pthread_cond_signal(&cond_tel);
	rc = pthread_mutex_unlock(&mutex_tel);

	if(seats_row == -1) {	// If available seats not found
		rc = pthread_mutex_lock(&mutex_service_time);
		total_service_time += (time_tel_end.tv_sec-time_cust_arrived.tv_sec);
		rc = pthread_mutex_unlock(&mutex_service_time);

		rc = pthread_mutex_lock(&mutex_unsuccessful_res_seat);
		total_unsuccessful_res_seat++;
		rc = pthread_mutex_unlock(&mutex_unsuccessful_res_seat);

		printf("Reservation Failed: Could not find available seats for customer %d.\n", *cust_n);
		pthread_exit(NULL);
	}
	// At this point the customer is done with the telephonist

	// calculate cost
	float cost;
	if(zone_n == 0) {
		cost = seats_n*C_zone_A;
	} else {
		cost = seats_n*C_zone_B;
	}

	rc = pthread_mutex_lock(&mutex_cash);
	while(remaining_N_cash == 0) {
		rc = pthread_cond_wait(&cond_cash, &mutex_cash);
	}
	remaining_N_cash--;
	rc = pthread_mutex_unlock(&mutex_cash);
	clock_gettime(CLOCK_REALTIME, &time_cash_start);
	int time_to_test_payment = rand_r(&unique_seed) % t_cash_high + t_cash_low;
	sleep(time_to_test_payment);

	int card_success = 0;
	if(rand_r(&unique_seed) % 100 + 1 <= P_card_success*100) {
		card_success = 1;
	}

	clock_gettime(CLOCK_REALTIME, &time_cash_end);

	rc = pthread_mutex_lock(&mutex_customer_waiting_time);
	total_customer_waiting_time += (time_cash_start.tv_sec-time_tel_end.tv_sec);
	rc = pthread_mutex_unlock(&mutex_customer_waiting_time);

	rc = pthread_mutex_lock(&mutex_service_time);
	total_service_time += (time_cash_end.tv_sec-time_cust_arrived.tv_sec);
	rc = pthread_mutex_unlock(&mutex_service_time);

	rc = pthread_mutex_lock(&mutex_cash);
	remaining_N_cash++;
	pthread_cond_signal(&cond_cash);
	rc = pthread_mutex_unlock(&mutex_cash);

	if(!card_success) {
		if(zone_n == 0) {
			rc = pthread_mutex_lock(&mutex_seats_A);
		} else {
			rc = pthread_mutex_lock(&mutex_seats_B);
		}
		for(int j = seats_starting_column; j < seats_starting_column + seats_n; j++) {
			zone[seats_row][j] = 0;
		}
		if(zone_n == 0) {
			rc = pthread_mutex_unlock(&mutex_seats_A);
		} else {
			rc = pthread_mutex_unlock(&mutex_seats_B);
		}
		
		rc = pthread_mutex_lock(&mutex_unsuccessful_res_card);
		total_unsuccessful_res_card++;
		rc = pthread_mutex_unlock(&mutex_unsuccessful_res_card);

		printf("Reservation Failed: Transaction was not accepted for customer %d.\n", *cust_n);
		pthread_exit(NULL);
	}

	rc = pthread_mutex_lock(&mutex_successful_res);
	total_successful_res++;
	rc = pthread_mutex_unlock(&mutex_successful_res);

	rc = pthread_mutex_lock(&mutex_account_balance);
	account_balance += cost;
	rc = pthread_mutex_unlock(&mutex_account_balance);


	rc = pthread_mutex_lock(&mutex_print_console);
	printf("┌------------------------------------------------------┐\n");
	printf(" Reservation Success for customer %d.\n", *cust_n);
	printf(" Reservation Cost: %.2f.\n", cost);
	printf(" Seats Reserved in row %d: ", seats_row + 1);
	for(int j = seats_starting_column; j < seats_starting_column + seats_n; j++) {
		printf("%d, ", j + 1);
	}
	if(zone_n == 0) {
		printf(" in Zone A.\n");
	}else{
		printf(" in Zone B.\n");
	}
	printf("└------------------------------------------------------┘\n");
	rc = pthread_mutex_unlock(&mutex_print_console);
}

int main(int argc, char* argv[]) {

	if (argc < 2 || argc > 3) {
		printf("Invalid number of arguments.\n");
		return -1;
	} 
	N_cust = atoi(argv[1]);
	seed = atoi(argv[2]);
	if (N_cust < 0) { 
		printf("Invalid number of customers.\n");
		return -1;
	}

	// Allocation of seats
	zone_A = malloc(N_zone_A*sizeof(int*));
	zone_B = malloc(N_zone_B*sizeof(int*));

	int i;
	for (i = 0; i < N_zone_A; i++) {
		zone_A[i] = malloc(N_seat*sizeof(int));
	}

	for (i = 0; i < N_zone_B; i++) {
		zone_B[i] = malloc(N_seat*sizeof(int));
	}
	// End of seat allocation

	account_balance = 0.0f;
	remaining_N_tel = N_tel;
	remaining_N_cash = N_cash;
	total_customer_waiting_time = 0;
	total_service_time = 0;
	total_successful_res = 0;
	total_unsuccessful_res_seat = 0;
	total_unsuccessful_res_card = 0;

	pthread_mutex_init(&mutex_tel, NULL);
	pthread_mutex_init(&mutex_cash, NULL);
	pthread_mutex_init(&mutex_seats_A, NULL);
	pthread_mutex_init(&mutex_seats_B, NULL);
	pthread_mutex_init(&mutex_customer_waiting_time, NULL);
	pthread_mutex_init(&mutex_service_time, NULL);
	pthread_mutex_init(&mutex_unsuccessful_res_seat, NULL);
	pthread_mutex_init(&mutex_unsuccessful_res_card, NULL);
	pthread_mutex_init(&mutex_successful_res, NULL);
	pthread_mutex_init(&mutex_account_balance, NULL);
	pthread_mutex_init(&mutex_print_console, NULL);

	pthread_cond_init(&cond_tel, NULL);
	pthread_cond_init(&cond_cash, NULL);

	pthread_t *th;
	th = malloc(N_cust*sizeof(pthread_t));

	int *customer_number = malloc(N_cust*sizeof(int)); 
	for (int i = 0; i < N_cust; i++) {
		customer_number[i] = i+1;
	}

	for(int i=0; i < N_cust; i++) {
		if(pthread_create(&th[i], NULL, &routine, &customer_number[i]) != 0) {
			printf("Error creating pthread.\n");
			return -1;
		}
		int wait_time = rand_r(&seed) % t_res_high + t_res_low;
		sleep(wait_time);
	}
	
	
	for (int i=0; i < N_cust; i++) {
		if (pthread_join(th[i], NULL) != 0) {
			printf("Failed to join thread.\n");
			return -1;
		}
	}
	
	printf("\n\nEnd of Customer Service.\n\n");

	for(int i = 0; i < N_zone_A; i++) {
		for(int j = 0; j < N_seat; j++) {
			if(zone_A[i][j] != 0)
				printf("Zone A, Row: %d, Seat: %d, Customer: %d\n", i+1, j+1, zone_A[i][j]);
		}
	}

	for(int i = 0; i < N_zone_B; i++) {
		for(int j = 0; j < N_seat; j++) {
			if(zone_B[i][j] != 0)
				printf("Zone B, Row: %d, Seat: %d, Customer: %d\n", i+1, j+1, zone_B[i][j]);
		}
	}

	printf("\nTotal Revenue: %.2f\n", account_balance);
	if(N_cust > 0) {
		printf("\nSuccessful Reservations: %.2f%%\n", total_successful_res*100.0f/N_cust);
		printf("\nUnsuccessful Reservations (Seats Unavailable): %.2f%%\n", total_unsuccessful_res_seat*100.0f/N_cust);
		printf("\nUnsuccessful Reservations (Transaction Error): %.2f%%\n", total_unsuccessful_res_card*100.0f/N_cust);

		printf("\nAverage Customer Waiting Time: %.2fs\n", total_customer_waiting_time*1.0f/N_cust);
		printf("\nAverage Customer Service Time: %.2fs\n", total_service_time*1.0f/N_cust);
	}
	// Deallocate Thread Array
	free(th);

	// Deallocate Seat Arrays
	for (i = 0; i < N_zone_A; i++) {
		free(zone_A[i]);
	}
	free(zone_A);
	for (i = 0; i < N_zone_B; i++) {
		free(zone_B[i]);
	}
	free(zone_B);

	pthread_mutex_destroy(&mutex_tel);
	pthread_mutex_destroy(&mutex_seats_A);
	pthread_mutex_destroy(&mutex_seats_B);
	pthread_mutex_destroy(&mutex_cash);
	pthread_mutex_destroy(&mutex_customer_waiting_time);
	pthread_mutex_destroy(&mutex_service_time);
	pthread_mutex_destroy(&mutex_unsuccessful_res_card);
	pthread_mutex_destroy(&mutex_unsuccessful_res_seat);
	pthread_mutex_destroy(&mutex_successful_res);
	pthread_mutex_destroy(&mutex_print_console);
	pthread_mutex_destroy(&mutex_account_balance);
	
	pthread_cond_destroy(&cond_tel);
	pthread_cond_destroy(&cond_cash);

	printf("\nProgram Executed Successfully\n");
	return 0;
}