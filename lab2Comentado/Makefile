all: lab2 broker worker
FLAGS = -Wall -lm

lab2: lab2.c funciones.o
	gcc $(FLAGS) -o lab2 lab2.c funciones.o

broker: broker.c fbroker.o funciones.o
	gcc $(FLAGS) -o broker broker.c fbroker.o funciones.o

fbroker.o: fbroker.c fbroker.h
	gcc -c $(FLAGS) fbroker.c
	
worker: worker.c fworker.o funciones.o
	gcc $(FLAGS) -o worker worker.c fworker.o funciones.o

fworker.o: fworker.c fworker.h
	gcc -c $(FLAGS) fworker.c
	
funciones.o: funciones.c funciones.h
	gcc -c $(FLAGS) funciones.c
	
clear:
	rm -f lab2 broker *.o

