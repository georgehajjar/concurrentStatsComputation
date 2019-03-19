#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

// initialize test data 1
int dataOne[5] = {5, 6, 8, 2, 7};
// initialize test data 2
int dataTwo[5] = {10, 9, 11, 5, 7};

struct shared {
  // initialize empty array to hold input data
  int inputData[5];
};

static int set_semvalue(int i);
static void del_semvalue(int i);
static int semaphore_p(int i);
static int semaphore_v(int i);

//initialize array to hold semaphore id's
int sem_id[5];

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
semctl call. We need to do this before we can use the semaphore. */
static int set_semvalue(int i) {
  union semun sem_union;
  sem_union.val = 1;
  if (semctl(sem_id[i], 0, SETVAL, sem_union) == -1) {
    return (0);
  }
  return (1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
the command IPC_RMID to remove the semaphore's ID. */
static void del_semvalue(int i) {
  union semun sem_union;
  if (semctl(sem_id[i], 0, IPC_RMID, sem_union) == -1) {
    fprintf(stderr, "Failed to delete semaphore\n");
  }
}

/* semaphore_p changes the semaphore by -1 (waiting). */
static int semaphore_p(int i) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1; /* P() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id[i], &sem_b, 1) == -1) {
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
  if (semop(sem_id[i], &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return (0);
  }
  return (1);
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

void createSemaphores(void) {
  for(int i = 0; i < 4; i++) {
    sem_id[i] = semget((key_t)1234+i, 1, 0666 | IPC_CREAT);

    if (!set_semvalue(i)) {
      fprintf(stderr, "Failed to initialize semaphore\n");
      exit(EXIT_FAILURE);
    }
  }
}

void destorySemaphores(void) {
  for(int i = 0; i < 4; i++) {
    del_semvalue(i);
  }
}

int main(int argc, char* argv[]) {

  int running = 1;
  void *shared_memory = (void *)0;
  struct shared *shared_data;
  int shmid;
  bool debugOn = debug();

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

  //printf("Memory attached at %X\n", (int)shared_memory);

  shared_data = (struct shared *)shared_memory;

  printf("Enter 5 distinct integers to be sorted (seperated by commas).\n");
  scanf("%d,%d,%d,%d,%d", &shared_data->inputData[0], &shared_data->inputData[1], &shared_data->inputData[2], &shared_data->inputData[3], &shared_data->inputData[4]);

  for(int i = 0; i < 5; i++) {
    printf("%d\n", shared_data->inputData[i]);
  }

  createSemaphores();

  int time, tmp;
  pid_t pid[4];

  for(int i = 0; i < 4; i++) {
    pid[i] = fork();
    if (pid[i] == 0) {
      break;
    }
  }

  if (pid[0] != 0 && pid[1] != 0 && pid[2] != 0 && pid[3] != 0) {
    //printf("I'm the parent process [pid: %d, ppid: %d]\n",getpid(),getppid());
    for(int i = 0; i < 4; i++) {
      wait(&time);
    }
  }
  else {
    printf("I'm a child process [pid: %d, ppid: %d]\n",getpid(),getppid());

    while (!(
      shared_data->inputData[0] >= shared_data->inputData[1] &&
      shared_data->inputData[1] >= shared_data->inputData[2] &&
      shared_data->inputData[2] >= shared_data->inputData[3] &&
      shared_data->inputData[3] >= shared_data->inputData[4])) {

        if (pid[0] == 0) {
          if (!semaphore_p(0)) {
            exit(EXIT_FAILURE);
          }

          if (shared_data->inputData[0] < shared_data->inputData[1]) {
            if(debugOn) {
              printf("Process %d: performed swapping\n", getpid());
            }
            tmp = shared_data->inputData[0];
            shared_data->inputData[0] = shared_data->inputData[1];
            shared_data->inputData[1] = tmp;
          }
          else {
            if(debugOn) {
              printf("Process %d: No swapping\n", getpid());
            }
          }

          if (!semaphore_v(0)) {
            exit(EXIT_FAILURE);
          }
        }

        else if (pid[1] == 0) {
          if (!semaphore_p(0)) {
            exit(EXIT_FAILURE);
          }
          if (!semaphore_p(1)) {
            exit(EXIT_FAILURE);
          }

          if (shared_data->inputData[1] < shared_data->inputData[2]) {
            if(debugOn) {
              printf("Process %d: performed swapping\n", getpid());
            }
            tmp = shared_data->inputData[1];
            shared_data->inputData[1] = shared_data->inputData[2];
            shared_data->inputData[2] = tmp;
          }
          else {
            if(debugOn) {
              printf("Process %d: No swapping\n", getpid());
            }
          }

          if (!semaphore_v(0)) {
            exit(EXIT_FAILURE);
          }
          if (!semaphore_v(1)) {
            exit(EXIT_FAILURE);
          }
        }

        else if (pid[2] == 0) {
          if (!semaphore_p(1)) {
            exit(EXIT_FAILURE);
          }
          if (!semaphore_p(2)) {
            exit(EXIT_FAILURE);
          }

          if (shared_data->inputData[1] < shared_data->inputData[2]) {
            if(debugOn) {
              printf("Process %d: performed swapping\n", getpid());
            }
            tmp = shared_data->inputData[1];
            shared_data->inputData[1] = shared_data->inputData[2];
            shared_data->inputData[2] = tmp;
          }
          else {
            if(debugOn) {
              printf("Process %d: No swapping\n", getpid());
            }
          }

          if (!semaphore_v(1)) {
            exit(EXIT_FAILURE);
          }
          if (!semaphore_v(2)) {
            exit(EXIT_FAILURE);
          }
        }

        else if (pid[3] == 0) {
          if (!semaphore_p(2)) {
            exit(EXIT_FAILURE);
          }
          if (!semaphore_p(3)) {
            exit(EXIT_FAILURE);
          }

          if (shared_data->inputData[3] < shared_data->inputData[4]) {
            if(debugOn) {
              printf("Process %d: performed swapping\n", getpid());
            }
            tmp = shared_data->inputData[3];
            shared_data->inputData[3] = shared_data->inputData[4];
            shared_data->inputData[4] = tmp;
          }
          else {
            if(debugOn) {
              printf("Process %d: No swapping\n", getpid());
            }
          }

          if (!semaphore_v(2)) {
            exit(EXIT_FAILURE);
          }
          if (!semaphore_v(3)) {
            exit(EXIT_FAILURE);
          }
        }
      }
    }

    printf("Sorted list:");
    for(int i = 0; i < 5; i++) {
      printf("%d ", shared_data->inputData[i]);
    }
    printf("\nMinimum value: %d", shared_data->inputData[0]);
    printf("\nMaximum value: %d", shared_data->inputData[4]);
    printf("\nMedian value: %d", shared_data->inputData[2]);

    destorySemaphores();

    if (shmdt(shared_memory) == -1) {
      fprintf(stderr, "shmdt failed\n");
      exit(EXIT_FAILURE);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
      fprintf(stderr, "shmctl(IPC_RMID) failed\n");
      exit(EXIT_FAILURE);
    }
  }
