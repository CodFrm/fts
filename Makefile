g=g++ -Wall

obj=main.o fts.o
param=-lpthread

all:main

main:$(obj)
	$(g) -o main main.o fts.o $(param)

main.o:main.c
	$(g) -c main.c

fts.o:fts.cpp
	$(g) -c fts.cpp
	
clean:
	@rm *.o main

run:clean all
	./main
