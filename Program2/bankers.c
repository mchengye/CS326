	/*
	 ============================================================================
	 Name        : bankers.c
	 Date        : March. 9,2017
	 Authors     : Sherry Feng & Marbo Cheng
	 Description : A multithreaded program that implements the Banker's algorithm.
	 			   Uses threads to generate customer's and uses mutex lock to avoid 
	 			   race conditions when updating global variables.
	 ============================================================================
	 Compile code: gcc bankers.c -o bankers -lpthread
	 run: ./bankers 10 5 7
	 */

	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <assert.h>
	#include <string.h>
	#include <time.h>

	 #define NUMBER_OF_CUSTOMERS 5
	 #define NUMBER_OF_RESOURCES 3

	 //initialize pthread mutex 
	 pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	 /*the avaible amount of each resources */
	 int available[NUMBER_OF_RESOURCES];

	 /*the maxiumum demand of each customer */
	 int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = { {5,1,1},
	 {3,2,1},
	 {3,3,2},
	 {4,6,1},
	 {6,3,2}};

	 /* the allocated resources of each customer */
	 int allocation [NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = { {0,0,0},
	 {0,0,0},
	 {0,0,0},
	 {0,0,0},
	 {0,0,0}};

	 /* the remaining need of each customer */
	 int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

	 
	//the customers
	/*create n customers threads that request and release resources from the bank.
	The customers will continually loop, requesting and then releasing random number
	of resources. The customers' request of resources will be bounded by their
	respective values in the need array. The banker will grant a request if it satifies
	the safety algorithm outlined in Section 7.5.3.1 If a request does not leave the
	system in a safe state, the banker will deny it. Function prototypes for requesting
	and resources are as follows:
	*/
	int request_resources(int customer_num, int request[NUMBER_OF_RESOURCES]);

	int release_resources(int customer_num, int release[NUMBER_OF_RESOURCES]);

	 //int** need(int max[][3], int allocation[][3], int numCustomer);

	 /*You should invokeyour program by passing the number of resources of each type
	 on the command line. For example, if there were three resources types, with
	 ten instances of the first type, fiive of the second type, and seven of the third
	 type, you would invoke your program follows:
	 
	 ./a.out 10 5 7

	 The available array would be initialized to these values. You may initialize
	 the maximum array (which holds the maximum demand of each customers) using any
	 method you find convenient.
	*/
	 // customer thread
	 void* customer(void* argument);

	 // wait time used for delay in releasing resources
	 void waitFor (unsigned int secs);

	//banker's algorithm
	 int bankers( int customerId, int request[NUMBER_OF_RESOURCES] );


	 /*-------------------------------------------------------------------
	 * Function:    main
	 * Purpose:     Initialize customer threads, set available resources with command line input, 
	 				and need array
	 * In args:     takes in 3 integers from command line execution
	 * Outputs:     None
	 *
	 */
	 int main(int argc, char **argv)
	 {

	 	//place input avaliable into the available array
	 	for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
	 	{
	 		available[i] = atoi(argv[i+1]);

	 	}

	    //calculate need 
	 	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
	 		for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
	 			need[i][j] = maximum[i][j] - allocation[i][j];
	 		}
	 	}


	 	int i,j;

	 	pthread_t threads[NUMBER_OF_CUSTOMERS];
	 	int thread_args[NUMBER_OF_CUSTOMERS];
	 	int result_code, index;
	 	int customerId;

	   // create all customer threads one by one
	 	for ( customerId = 0; customerId < NUMBER_OF_CUSTOMERS; ++customerId) {
	 		thread_args[customerId]= customerId;
	 		
	      	//instead of passing customerId we already have threadId which is customerId
	      	//need to pass request as argument
	 		result_code = pthread_create(&threads[customerId], NULL, customer, &thread_args[customerId]);
	 		assert(0 == result_code);
	 	}

	 	for (customerId = 0; customerId < NUMBER_OF_CUSTOMERS; ++customerId) {
	      // block until thread 'index' completes
	 		result_code = pthread_join(threads[customerId], NULL);
	      //printf("In main: customer %d has completed\n", customerId);
	 		assert(0 == result_code);
	 	}


	 }
	/*-------------------------------------------------------------------
	 * Function:    customer
	 * Purpose:     Generate random resource request for customer, execute banker's algorithm
	 				to test whether current request is safe, and release current customer 
	 				resource randomly.
	 * In args:     integer id value associated with current customer thread
	 * Outputs:     none
	 */
	 void* customer(void* argument) {


	 	int customerId = *((int *) argument);

	   //random request <= request+allocation
	 	while(1){
	   		//generate random number that is less than max - allocation

	 		int r1 = rand()%(maximum[customerId][0]-allocation[customerId][0]+1);

	   		//random request cannot be bigger than (max - allocation)
	 		while (r1 > (rand()%(maximum[customerId][0]-allocation[customerId][0])+1) ) {

	 			r1 = rand()%(maximum[customerId][0]-allocation[customerId][0]+1);
	 		}


	 		int r2 = rand()%(maximum[customerId][1]-allocation[customerId][1]+1);

	 		while (r2 > (rand()%(maximum[customerId][1]-allocation[customerId][1])+1) ) {

	 			r2 = rand()%(maximum[customerId][1]-allocation[customerId][1]+1);
	 		}


	 		int r3 = rand()%(maximum[customerId][2]-allocation[customerId][2]+1);

	 		while (r3 > (rand()%(maximum[customerId][2]-allocation[customerId][2])+1) ) {

	 			r3 = rand()%(maximum[customerId][2]-allocation[customerId][2]+1);
	 		}

	 		
	 		
	 		int request[3] = {r1, r2, r3};
	 			//request is only acceptable when it is non-zero (0 0 0)
	 		if (request[0] != 0 || request[1] != 0 || request[2] != 0 ) {
	 			int requestSuccess = request_resources(customerId, request);

	 			if (requestSuccess == 0) {



	 				pthread_mutex_lock(&mutex);

	 				int safe = bankers(customerId , request);

	 				if (safe) {


	 					for (int i = 0 ; i < NUMBER_OF_RESOURCES; i++) {
	 						//update available - request
	 						available[i] = available[i] - request[i];


	 						//update current customerId allocation
	 						allocation[customerId][i] = allocation[customerId][i] + request[i];

	 						need[customerId][i] = maximum[customerId][i]- allocation[customerId][i];

	 					}
	 					printf("Customer %d | Request   %d %d %d | Available = %d %d %d  | Allocation = %d %d %d \n", customerId, 
	 						request[0], request[1], request[2], available[0], available[1], available[2], 
	 						allocation[customerId][0], allocation[customerId][1], allocation[customerId][2]); 


	 				}
	 				else {
	 					printf("Customer %d | Request   %d %d %d | Available = %d %d %d | INSUFFICIENT RESOURCES \n", customerId, 
	 						request[0], request[1], request[2], available[0], available[1], available[2]);
	 				}



	 				pthread_mutex_unlock(&mutex);

	 				//random wait within 3 seconds
	 				int randomTime = rand()%3;
	 				waitFor(randomTime);


				 	//release all allocation of resource on current customer
				 	//release only is current allocation is not 0
	 				pthread_mutex_lock(&mutex);
	 				if (allocation[customerId][0] > 0 || allocation[customerId][1] || allocation[customerId][2]) {
	 					release_resources(customerId, allocation[customerId]);
	 				}
	 				pthread_mutex_unlock(&mutex);


	 			}
	 		}

	 	}

	 }

	 /*-------------------------------------------------------------------
	 * Function:    bankers
	 * Purpose:     Run the banker's algorithm on current customer's request
	 				to test whether it is safe to grant the request.
	 * In args:     int customerId; Customer's id who is requesting resources
	 				int request[];  Array of resources being request from customer
	 * Outputs:     If it is safe to grant request a 1 will be returned.
	 				If not safe then 0 will be returned.
	 */

	 int bankers( int customerId, int request[] ) {

	 	

	    //copies of each global variable to test banker's algorithm
	 	int availableCopy[NUMBER_OF_RESOURCES];

	 	//assume request has been allocated so it subtract available
	 	for (int i = 0 ; i < NUMBER_OF_RESOURCES; i++) {
	 		availableCopy[i] = available[i]-request[i];
	 	}



	 	int allocationCopy [NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

	 	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
	 		for (int j =0 ; j < NUMBER_OF_RESOURCES; j++) {
	 			allocationCopy[i][j] = allocation[i][j];
	 		}
	 	}
	 	
	 	int needCopy[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
	 	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
	 		for (int j =0 ; j < NUMBER_OF_RESOURCES; j++) {
	 			needCopy[i][j] = need[i][j];
	 		}
	 	}
	    //update allocation with request

	 	
	    //assume requested granted update current customer allocation
	 	for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
	 		allocationCopy[customerId][i] = allocation[customerId][i]+ request[i];
	 	}

	    //calculate need for current update allocation
	 	for (int i= 0 ; i < NUMBER_OF_RESOURCES; i++) {
	        //assume allocated so request is allocation
	 		needCopy[customerId][i] = maximum[customerId][i] - allocationCopy[customerId][i];

	 	}


	    //check with the available to run
	 	int customerStatus[5] = {0 , 0, 0, 0 ,0};

	 	int testCount = 5;

	 	int safe = 0;
	 	int execute = 1;



	//}
	 	while (testCount != 0) {
	 		safe = 0; 

	    //Loop throught each customer
	 		for (int id = 0; id < NUMBER_OF_CUSTOMERS ; id++) {
	        //check status of current status if 0 mean it has not been executed
	 			if (customerStatus[id] == 0 ) {
	        //check if available can satisfy need
	 				execute = 1;
	 				for (int j = 0 ; j < NUMBER_OF_RESOURCES; j++) {

	 					//if need if higher than my available do not execute
	 					if ( needCopy[id][j] > availableCopy[j]){

	 						execute = 0;
	 						break;
	 					}

	 				}

	 				if (execute) {

	            //update customer status to tested
	 					customerStatus[id] = 1;

	            //we have less customer to look at
	 					testCount--;

	 					safe = 1;

	            //update available with current customer's allocation
	 					for (int i =0 ; i< NUMBER_OF_RESOURCES; i++) {

	 						//availableCopy[i] = availableCopy[i] - request[i];

	 						//if safe release resource and update available
	 						availableCopy[i] = availableCopy[i] + allocationCopy[id][i];

	 					}
	 					break;


	 				}


	 			}
	 		}
	 		if (safe) {

	 			return 1;
	 		}
	 		else {
	 				
	 			return 0;
	 		}

	 	}

	 }


	 /*-------------------------------------------------------------------
	 * Function:    request_resources
	 * Purpose:     test to make sure customer's request + customer's allocation
	 				is not bigger than maximum.
	 * In args:     int customerId; Customer's id who is requesting resources
	 				int request[];  Array of resources being request from customer
	 * Outputs:     If customer's request + customer's allocation < maximum 
	 				then 0 will be returned.
	 				If customer's request + customer's allocation > maximum 
	 				then -1 will be returned.
	 */
	
	 int request_resources(int customerId, int request[]) {

	 	for (int i = 0 ; i< NUMBER_OF_RESOURCES; i++) {
	 		//if any resource request is more than available return -1
	 		if ((request[i]+ allocation[customerId][i]) <= maximum[customerId][i]) {

	 			return 0;
	 		} 

	 		else {
	 			return -1;
	 		}
	 	}

	 }

	/*-------------------------------------------------------------------
	 * Function:    release_resources
	 * Purpose:     Release current customer's resource allocation and update
	 				available resources by adding current customer released resources
	 * In args:     int customerId; Customer's id who is releasing resources
	 				int release[];  Array of resources being released from customer
	 * Outputs:     Update current customer's allocation to 0 and update available resources
	 */

	 int release_resources(int customerId, int release[]) {

	 	for (int i = 0 ; i< NUMBER_OF_RESOURCES; i++) {

	 	//updating available to add current customer resource allocation

	 		available[i] = available[i] + release[i];


	 	} 
	 	printf("Customer %d | Releasing %d %d %d | Available = %d %d %d | Allocated = 0 0 0  \n", customerId, 
	 		release[0], release[1], release[2], available[0], available[1],
	 		available[2]);


	 	for (int j =0; j < NUMBER_OF_RESOURCES; j++) {
	 		allocation[customerId][j] = 0;

	 	}


	 	return 0;
	 }
	 /*-------------------------------------------------------------------
	 * Function:    waitFor
	 * Purpose:     Generate a delay time used for releasing resources
	 * In args:     unsigned int secs; Seconds to be delayed. 
	 * Outputs:     None. It only generate a delay time.
	 */
	 void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime) {}               // Loop until it arrives at that time.
}
