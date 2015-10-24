#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_LINE 1024

typedef enum {
    START,
    SEND_REQUEST,
    WAIT_FOR_INFO,
    SEND_BEACON,
    WAIT_FOR_REQUEST,
    SEND_INFO,
    EXIT

} state;

int main(int argc, char** argv){

    state current_state;
    int i = 0;
    char line[MAX_LINE];
    char * myfifo = "/tmp/myFIFO";
    mkfifo(myfifo, 0666);
    int pipe_in;
    int pipe_out;

    //open named pipes
    pipe_out = open(myfifo, O_WRONLY);
    pipe_in = open(myfifo, O_RDONLY);

    /*this needs to be replaced with whatever actually
    identifies/defines these things. 
    It would probably be better practice to use some #define clauses for these too*/
    char beacon = 1;
    char request_to_join = 2;
    char info = 3;
    int timedout = 0;

    while(1){

        switch(current_state){
            case START: 
                printf("case: START\n");
                read(pipe_in, line, MAX_LINE);

                if(*line == beacon){
                    current_state = SEND_REQUEST;
                }

                break;
            case SEND_REQUEST:
                printf("case: SEND_REQUEST\n");
                write(pipe_out, request_to_join, sizeof(request_to_join));
                current_state = WAIT_FOR_INFO;
                break;
            case WAIT_FOR_INFO:
                printf("case: WAIT_FOR_INFO\n");
                read(pipe_in, line, MAX_LINE);
                if(timedout){
                    current_state = START;
                }else if(*line == info){
                    //set id and parent
                    current_state = EXIT; //should actually be SEND_DATA
                }
                break;
            case SEND_BEACON:
                printf("case: SEND_BEACON");
            case WAIT_FOR_REQUEST:
                printf("case: WAIT_FOR_REQUEST");
                if (isRunning) {
                    current_state = SEND_DATA;
                } else {
                    current_state = SEND_BEACON;
                }
                break;
            case SEND_INFO:
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

    //close named pipes
    close(pipe_in);
    close(pipe_out);

    return 0;
}