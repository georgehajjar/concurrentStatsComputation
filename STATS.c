#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

//#include "shm.h"

// initialize test data 1
int dataOne[5] = {5, 6, 8, 2, 7};
// initialize test data 2
int dataTwo[5] = {10, 9, 11, 5, 7};

struct shared {
  // initialize empty array to hold input data
  int inputData[5];
  //initialize array to hold semaphore id's
  int sem_id[5];
  //initialize an array of booolean with mutex's
  bool isGreater[5];
};

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
semctl call. We need to do this before we can use the semaphore. */
static int set_semvalue(int i) {
  union semun sem_union;
  sem_union.val = 1;
  if (semctl(shared->sem_id[i-1], 0, SETVAL, sem_union) == -1) {
    return (0);
  }
  return (1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
the command IPC_RMID to remove the semaphore's ID. */
static void del_semvalue(int i) {
  union semun sem_union;
  if (semctl(shared->sem_id[i-1], 0, IPC_RMID, sem_union) == -1) {
    fprintf(stderr, "Failed to delete semaphore\n");
  }
}

/* semaphore_p changes the semaphore by -1 (waiting). */
static int semaphore_p(int i) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1; /* P() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(shared->sem_id[i-1], &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_p failed\n");
    return (0);
  }
  return (1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
so that the semaphore becomes available. */
static int semaphore_v(int i) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = 1; /* V() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(shared->sem_id[i-1], &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return (0);
  }
  return (1);
}


void createProcesses(void) {
  int i, tmp;
  pid_t pid[4];

  for(i = 0; i < 4; i++) {
    pid[i] = fork();
    if(pid[i] == 0) {
      break;
    }
  }

  if (pid[0] != 0 && pid[1] != 0 && pid[2] != 0 && pid[3] != 0) {
    printf("I'm the parent process [pid: %d, ppid: %d]\n",getpid(),getppid());
    for(i = 0; i < 4; i++) {
      wait(&tmp);
    }
  } else {
    printf("I'm a child process [pid: %d, ppid: %d]\n",getpid(),getppid());
  }

  for(i = 0; i < 4; i++) {
    shared->sem_id[i]; = semget((key_t)1234+i, 1, 0666 | IPC_CREAT);

    if (!set_semvalue(i)) {
      fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char* argv[]) {

  int running = 1;
  void *shared_memory = (void *)0;
  struct shared *shared_data;
  int shmid;

  shmid = shmget((key_t)1234, sizeof(struct shared), 0666 | IPC_CREAT);

  if (shmid == -1) {
    fprintf(stderr, "shmget failed\n");
    exit(EXIT_FAILURE);
  }

  shared_memory = shmat(shmid, (void *)0, 0);
  if (shared_memory == (void *)-1) {
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }

  printf("Memory attached at %X\n", (int)shared_memory);

  shared_data = (struct shared *)shared_memory;
  for(int i = 0; i < 5; i++) {
    shared_data->isGreater[i] = false;
  }

  printf("Enter 5 distinct integers to be sorted (seperated by commas).\n");
  scanf("%d,%d,%d,%d,%d", &shared_data->inputData[0], &shared_data->inputData[1], &shared_data->inputData[2], &shared_data->inputData[3], &shared_data->inputData[4]);

  for(int i = 0; i < 5; i++) {
    printf("%d\n", shared_data->inputData[i]);
  }

  // while(running) {
  //   if (shared_stuff->written_by_you) {
  //     printf("You wrote: %s", shared_stuff->some_text);
  //     sleep(rand() % 4); /*Make the other process wait for us!*/
  //     shared_stuff->written_by_you = 0;
  //     if (strncmp(shared_stuff->some_text, "end", 3) == 0) {
  //       running = 0;
  //     }
  //   }
  // }

  if (shmdt(shared_memory) == -1) {
    fprintf(stderr, "shmdt failed\n");
    exit(EXIT_FAILURE);
  }

  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
  }

  createProcesses();
}
