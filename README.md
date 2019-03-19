Mattias Lightstone
George Hajjar


**How to compile and run**
1. Run make
2. Run ./STATS
3. Follow the prompts in the command line
3.1 y for debug mode, n for no debug, press enter
3.2 input 5 integers separated by spaces, press enter

**Our Algorithm**

For the purposes of our algorithm, the number of items to be sorted is n

The algorithm uses 3 types of data structure all in shared memory and all with a corresponding semaphore
- An array of n int data elements representing the current list and order of values to be sorted
- An array of n-1 int status registers each corresponding to a process needing to check the data values associated with it. 0 indicates that it needs action, 1 indicates that it is "completed" (until another process makes a change to its elements)
- One int count that indicates the number of processes that are in the completed state.
- In order to simplify the notation the semaphore gets and releases are implied, each time that a value is read or written from shared memory
- This will not exactly be the case in the code, but if you want the exact details look at the source code

*Pseudo code*

int shared_data = {userinput[0], userinput[1], userinput[2] ... userinput[n-1]}; // get user data
int debugMode = userInput y or n;
for (i in 0..n-1){ // make n-1 child processes
  pid = fork()
  int index = i;
}

switch(pid){
  case 0 (child):
    while(true){ // forever
      status = status_data(index)
      if (status){
        // no action needs to be taken
        continue
      }
      else{
        // values need to be checked
        if data(index) is less than data(index){
          swap them
          count - status_data(index-1) - status_data(index-2) // reduce the value of count by the number of neighbours with 1s as their status
          change the status of both neighbors to 0 (unsorted)
        }
        else{
          don't do anything if they are sorted
        }
        increase count by 1
        check if the count is equal to n this would mean that each process has a status of 1 and that all elements in the array are sorted
        if count is equal to n{
          exit process;
        }
      }
    }

  default (parent):
    wait until any child exits
    when any child exits the array is sorted
    kill all other child processes
    print sorted array
    print minimum data[n-1]
    print maximum data[0]
    print minimum data[n/2 -1]
}


**Testing Results*
All test results are with debugging on unless specified in title

*Required Test 1*
values: 5, 6, 8, 2, 7  
Output:

Do you want to use this program in debug mode? (y/n)
y
Enter 5 distinct integers to be sorted (seperated by spaces).
5 6 8 2 7
5
6
8
2
7
Process 30793 (0) swaps 5 and 6
Process 30794 (1) swaps 5 and 8
Process 30796 (3) swaps 2 and 7
Process 30795 (2) swaps 5 and 7
Process 30796 (3) does not swap 5 and 2
Process 30793 (0) swaps 6 and 8
Process 30794 (1) swaps 6 and 7
Process 30795 (2) does not swap 6 and 5
Process 30793 (0) does not swap 8 and 7

Sorted
8 7 6 5 2 
Minimum value: 2
Maximum value: 8
Median value: 6
Process finished with exit code 0



*Required Test 2*
values: 10, 9, 11, 5, 7            
Output:

Do you want to use this program in debug mode? (y/n)
y
Enter 5 distinct integers to be sorted (seperated by spaces).
10 9 11 5 7
10
9
11
5
7
Process 30899 (0) does not swap 10 and 9
Process 30900 (1) swaps 9 and 11
Process 30899 (0) swaps 10 and 11
Process 30901 (2) does not swap 9 and 5
Process 30900 (1) does not swap 10 and 9
Process 30902 (3) swaps 5 and 7
Process 30901 (2) does not swap 9 and 7

Sorted
11 10 9 7 5 
Minimum value: 5
Maximum value: 11
Median value: 9
Process finished with exit code 0

*Three Digit Test*
values: 123 4672 100003 21 2953
Do you want to use this program in debug mode? (y/n)
y
Enter 5 distinct integers to be sorted (seperated by spaces).
123 4672 100003 21 2953
123
4672
100003
21
2953
Process 31263 (0) swaps 123 and 4672
Process 31266 (3) swaps 21 and 2953
Process 31264 (1) swaps 123 and 100003
Process 31263 (0) swaps 4672 and 100003
Process 31265 (2) swaps 123 and 2953
Process 31266 (3) does not swap 123 and 21
Process 31264 (1) does not swap 4672 and 2953

Sorted
100003 4672 2953 123 21 
Minimum value: 21
Maximum value: 100003
Median value: 2953
Process finished with exit code 0


*Already sorted no debug*
Do you want to use this program in debug mode? (y/n)
n
Enter 5 distinct integers to be sorted (seperated by spaces).
100 90 50 30 10
100
90
50
30
10

Sorted
100 90 50 30 10 
Minimum value: 10
Maximum value: 100
Median value: 50
Process finished with exit code 0

*Reverse order no debug*
values: 1, 3, 5, 8, 10
Output:
Do you want to use this program in debug mode? (y/n)
n
Enter 5 distinct integers to be sorted (seperated by spaces).
1 3 5 8 10
1
3
5
8
10

Sorted
10 8 5 3 1 
Minimum value: 1
Maximum value: 10
Median value: 5
Process finished with exit code 0
