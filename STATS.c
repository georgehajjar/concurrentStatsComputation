#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <time.h>

// initialize test data 1
int dataOne[5] = {5, 6, 8, 2, 7};
// initialize test data 2
int dataTwo[5] = {10, 9, 11, 5, 7};

int main(int argc, char **argv){
  printf("Hello\n");
}
