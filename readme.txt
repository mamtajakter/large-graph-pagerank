CIS630, Spring 2019, Term Project 1

Your First and Last name: Mamtaj Akter
Your Student ID: 951641885

Submission Date: 05/13/2019

What programming language did you use to write your code? C

Does your program compile on ix (CIS department server)? Yes

How should we compile your program on ix? (please provide a makefile) Makefile is attached

Does your program run on ix? Yes/No Yes

Does your program calculate the credit values accurately? Yes

Does your program have a limit for the number of nodes it can handle in the input graph? Yes
If yes, what is the limit on graph size that your program can handle? 1000000000

How long does it take for your program with 2(two) partitions to read the Flickr input files, perform 5 round and write
the output of each round in the output files on ix-dev?
Time to read input file = 9.845091 sec
	Time for round 1 = 2.737573 sec
	Time for round 2 = 2.758433 sec
	Time for round 3 = 2.742039 sec
	Time for round 4 = 2.748047 sec
	Time for round 5 = 2.777727 sec
Time to write the output file = 7.102895 sec

real	0m31.929s
user	1m1.062s
sys	0m1.398s

Does your program run correctly with four partitions on ix-dev? Yes

Does you program end gracefully after completing the specified number of rounds? Yes

***************
This code writes output in a file called "Output.txt". It dumps ID, Degree, Partition ID, Credit vales of each round in the same file. 
