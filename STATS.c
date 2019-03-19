#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include "semun.h"


#define NUM 5

// initialize test data 1
int dataOne[5] = {5, 6, 8, 2, 7};
// initialize test data 2
int dataTwo[5] = {10, 9, 11, 5, 7};

struct shared_data {
    // initialize empty array to hold input data
    int data[NUM];
};

struct shared_registers {
    // initialize empty array to hold input data
    int statusRegister[NUM-1];
    int count;
};


static int data_sem[NUM];
static int status_sem[NUM-1];
static int count_sem;

static int set_semvalue(int i);
static void del_semvalue(int i);
static int semaphore_p(int i);
static int semaphore_v(int i);
static void createSemaphores();
bool debug();

// simplest solution, odd numbered philosophers pick up right first even pick up left first
int main(int argc, char* argv[]){
    int running = 1;
    int status;
    void *shared_memory_1 = (void *)0;
    void *shared_memory_2 = (void*)0;
    struct shared_data *shared_data;
    struct shared_registers *shared_registers;
    int shmid1;
    int shmid2;
    bool debugOn = debug();

    shmid1 = shmget((key_t)1234, sizeof(struct shared_data), 0666 | IPC_CREAT);
    shmid2 = shmget((key_t)1235, sizeof(struct shared_registers), 0666 | IPC_CREAT);

    if (shmid1 == -1 || shmid2 == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory_1 = shmat(shmid1, (void *)0, 0);
    shared_memory_2 = shmat(shmid2, (void *)0, 0);
    if (shared_memory_1 == (void *)-1 || shared_memory_2 == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }

    //printf("Memory attached at %X\n", (int)shared_memory);

    shared_data = (struct shared_data *)shared_memory_1;
    shared_registers = (struct shared_register *)shared_memory_2;

    for(int i=0; i<NUM-1; i++){
        shared_registers -> statusRegister[i] = 0;
    }
    shared_registers -> count = 0;

    printf("Enter %d distinct integers to be sorted (seperated by spaces).\n", NUM);
    for(int i = 0; i<NUM; i++){
        scanf("%d", &shared_data -> data[i]);
    }

    for(int i = 0; i < NUM; i++) {
        printf("%d\n", shared_data->data[i]);
    }

    createSemaphores();

    int time, tmp;
    int index = 0;
    pid_t pid[NUM-1];

    for(int i = 0; i<(NUM-1); i++){
        pid[i] = fork();
        if (pid[i] == 0){
            index = i;
            break;
        }
    }
    int internal_count;
    switch(pid[index]){
        case 0:
            // do children things
            // while true
            while(true) {
                // child gets a lock on it's status
                semaphore_p(status_sem[index]);
                // check status
                int status = shared_registers -> statusRegister[index];
                // release lock
                semaphore_v(status_sem[index]);
                // if the status is a 1 continue
                if (status == 1){
                    continue;
                }
                // if the status is 0
                else {
                    // internal count = 0;
                    internal_count = 0;
                    // if it's even
                    if (index%2 == 0){
                        //get a lock on the right one then the left one
                        semaphore_p(data_sem[index+1]);
                        semaphore_p(data_sem[index]);

                    }
                    // if it's odd
                    else{
                        // get a lock on the left one then the right one
                        semaphore_p(data_sem[index]);
                        semaphore_p(data_sem[index+1]);
                    }
                    // check the two values to see if they are in the right order
                    // if not
                    if (shared_data -> data[index] < shared_data -> data[index+1]){
                        // swap
                        int temp = shared_data -> data[index];
                        shared_data -> data[index] = shared_data -> data[index + 1];
                        shared_data -> data[index + 1] = temp;
                        // get a lock on status index i-1 if true change to false and decrement internal count else do nothing
                        if (index != 0){
                            semaphore_p(status_sem[index-1]);
                            if (shared_registers -> statusRegister[index-1] == 1){
                                shared_registers -> statusRegister[index-1] = 0;
                                internal_count--;
                            }
                            // release
                            semaphore_v(status_sem[index-1]);
                        }
                        // get a lock on index i+1 if true change to false and decrement internal count else do nothing
                        if (index != NUM-2){
                            semaphore_p(status_sem[index+1]);
                            if (shared_registers -> statusRegister[index+1] == 1){
                                shared_registers -> statusRegister[index+1] = 0;
                                internal_count--;
                            }
                            // release
                            semaphore_v(status_sem[index+1]);
                        }
                    }
                    else{
                    }
                    // increase internal count by 1 and set its status to solved
                    internal_count ++;

                    semaphore_p(status_sem[index]);
                    shared_registers -> statusRegister[index] = 1;
                    semaphore_v(status_sem[index]);
                    // get a lock on count, increase by internal count
                    semaphore_p(count_sem);
                    shared_registers -> count += internal_count;
                    semaphore_v(data_sem[index]);
                    semaphore_v(data_sem[index+1]);
                    // check if count is equal to n and if it is exit
                    if (shared_registers -> count == NUM - 1){
                        semaphore_v(count_sem);
                        // we're sorted
                        // exit the process
                        exit(1);
                    }
                    semaphore_v(count_sem);
                }
            }

            break;
        default:
            // do parent things
            // wait until a child process terminates
            wait(&status);
            // kill all other children
            for(int i =0; i<NUM-1; i++){
                kill(pid[i], SIGKILL);
            }
            // sorted messages
            printf("Sorted\n");
            for(int i = 0; i < NUM; i++) {
                printf("%d ", shared_data->data[i]);
            }
            printf("\nMinimum value: %d", shared_data->data[0]);
            printf("\nMaximum value: %d", shared_data->data[4]);
            printf("\nMedian value: %d", shared_data->data[2]);
            break;
    }

    // start forking
}

bool debug(void) {
    char answer;

    printf("Do you want to use this program in debug mode? (y/n)\n");
    scanf(" %c", &answer);

    if (strcmp(&answer, "y") == 0) {
        return true;
    }
    else {
        return false;
    }


}

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
semctl call. We need to do this before we can use the semaphore. */
static int set_semvalue(int sem_id) {
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        return (0);
    }
    return (1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
the command IPC_RMID to remove the semaphore's ID. */
static void del_semvalue(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        fprintf(stderr, "Failed to delete semaphore\n");
    }
}

/* semaphore_p changes the semaphore by -1 (waiting). */
static int semaphore_p(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return (0);
    }
    return (1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
so that the semaphore becomes available. */
static int semaphore_v(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return (0);
    }
    return (1);
}

void createSemaphores(void) {
    for(int i = 0; i < (NUM); i++) {
        data_sem[i] = semget((key_t)1234+i, 1, 0666 | IPC_CREAT);

        if (!set_semvalue(data_sem[i])) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
    }
    for (int i=0; i<(NUM-1); i++){
        status_sem[i] = semget((key_t)1234+NUM+i, 1, 0666 | IPC_CREAT);

        if (!set_semvalue(status_sem[i])) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
    }
    count_sem = semget((key_t)1234+(2*NUM)-1, 1, 0666 | IPC_CREAT);
    if (!set_semvalue(count_sem)) {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
    }
}
