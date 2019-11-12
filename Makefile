
all: page-rank-calc.c
	mpicc page-rank-calc.c -O3 -o hello

run: hello
	mpirun --mca shmem posix --oversubscribe -np 4 hello /cs/classes/www/16S/cis630/PROJ/fl_compact.tab /cs/classes/www/16S/cis630/PROJ/fl_compact_part.4 5 4

clean:
	rm Output.txt
