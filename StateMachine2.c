#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> //timer functionalitiy.

#define MAX_LINE 1024
#define APP_MAX_LINE 1024
typedef enum {
    START,
    SEND_REQUEST,
    WAIT_FOR_INFO,
    SEND_DATA,
    WAIT_FOR_PARENT,
    SEND_INFO,
    WAIT_FOR_REQUEST,
    SEND_BEACON,
    EXIT

} State;

int main(int argc, char** argv) {
	State current_state;
    int i = 0;
    char line[MAX_LINE];
    char app_line[APP_MAX_LINE];
    char * myfifo = "/tmp/myFIFO";
    char * appfifo = "/tmp/appFIFO";
    mkfifo(myfifo, 0666);
    mkfifo(appfifo, 0666);
    int pipe_in;
    int pipe_out;
    int app_pipe_in;
    int app_pipe_out;

    //open named pipes
    pipe_out = open(myfifo, O_WRONLY);
    pipe_in = open(myfifo, O_RDONLY);

    app_pipe_in = open(appfifo, O_RDONLY);
    app_pipe_out = open(appfifo, O_WRONLY);

    /*this needs to be replaced with whatever actually
    identifies/defines these things. 
    It would probably be better practice to use some #define clauses for these too*/
    char beacon = 1;
    char request_to_join = 2;
    char info = 3;
    int timedout = 0;
    clock_t beacon_t1, beacon_t2;
    int isRunning = 0;
    int isBeaconTimeSet = 0;
    int parent_id = -1;

    while(1){

        switch(current_state){
            case START: //JOHN'S
                printf("case: START\n");
                read(pipe_in, line, MAX_LINE);
                isRunning = 0;

                if(*line == beacon){
                    current_state = SEND_REQUEST;
                    isRunning = 1; //set Running to TRUE
                }

                break;
            case SEND_REQUEST: //JOHN'S
                printf("case: SEND_REQUEST\n");
                write(pipe_out, request_to_join, sizeof(request_to_join));
                current_state = WAIT_FOR_INFO;
                break;
            case WAIT_FOR_INFO: //JOHN'S
                printf("case: WAIT_FOR_INFO\n");
                read(pipe_in, line, MAX_LINE);
                if(timedout){
                    current_state = START;
                }else if(*line == info){
                    //set id and parent
                    current_state = EXIT; //should actually be SEND_DATA
                }
                break;
            case SEND_DATA://KEVIN
            	printf("case: SEND_DATA");
            	//TO-DO send a DATA message!
            	read(app_pipe_in, app_line, APP_MAX_LINE); //read in the data from the video app
            	write(pipe_out, app_line, APP_MAX_LINE);   //give the data to network layer to send it.
            	isRunning = 1;
            	current_state = WAIT_FOR_PARENT;
            	break;
            case WAIT_FOR_PARENT: //KEVIN
            	printf("case: WAIT_FOR_PARENT");
            	break;
            case SEND_INFO: //KEVIN
            	printf("case: SEND_INFO");
            	/*
            	Send a message with the max ID + 1 and the ID of the new node’s parent. This will be either your parent if you have one, or your ID if there are just the two of you.
				Set your parent to be the new node.
				Go to WAIT_FOR_PARENT state
            	*/
            	current_state = WAIT_FOR_PARENT;
            	break;
            case SEND_BEACON: //KEVIN
            	printf("case: SEND_BEACON");
            	beacon_t1 = clock(); //set timer
            	//TO-DO set time for next beacon?????? what does this mean?
            	write(pipe_out, beacon, sizeof(beacon)); //send a BEACON
            	current_state = WAIT_FOR_REQUEST; //go to wait for request
            	break;
            case WAIT_FOR_REQUEST: //KEVIN
            	printf("case: WAIT_FOR_REQUEST");
            	beacon_t2 =  clock();
            	float beacon_t_diff = ( (float) beacon_t2 - (float) beacon_t1 / (float) CLOCKS_PER_SEC) * 1000; //CLOCKS_PER_SEC comes with <time.h>
            	if (beacon_t_diff > 1) { //if the timer has expired
            		if (isRunning) {
            			current_state = SEND_DATA;
            		} else {
            			current_state = SEND_BEACON;
            		}
            		break;
            	}
            	read(pipe_in, line, MAX_LINE);
            	if (*line == request_to_join) {
            		current_state = SEND_INFO;
            	}
            case EXIT:
                printf("case: EXIT\n");
                close(pipe_in);
                close(pipe_out);
                return 0;
            default: //make this wait_for_parent?
                printf("should not see\n"); 
                break;
        }

        //break;
    }