g=g++ -Wall

obj=mysql.o main.o fts.o
param=-lpthread -lmysqlclient

all:main

main:$(obj)
	$(g) -o main $(obj) $(param)

main.o:main.c
	$(g) -c main.c

fts.o:fts.h fts.cpp
	$(g) -c fts.h fts.cpp

mysql.o:mysql.cpp
	$(g) -c  mysql.h mysql.cpp

clean:
	@rm *.o main *.h.gch

run:clean all
	./main
