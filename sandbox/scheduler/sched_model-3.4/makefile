all: sched34.exe

sched34.exe: main.o sched.o list.o thread.o
	gcc -o sched34.exe main.o sched.o list.o thread.o

main.o: main.c
	gcc -Iinclude -c main.c

sched.o: sched.c sched.h sched_private.h
	gcc -Iinclude -c sched.c

thread.o: thread.c thread.h
	gcc -Iinclude -c thread.c

list.o: util/list.c
	gcc -Iinclude -c util/list.c

clean:
	del main.o
	del sched.o
	del list.o
	del thread.o
	del sched34.exe
