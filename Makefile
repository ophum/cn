objs = cn.c cuiEdit.o 
out = cn
opt = -std=c99
CC = gcc $(opt)
libs = -lncurses
headers = defines.h cuiEdit.h structs.h

cn: $(objs)
	$(CC) $(objs) -o $(out) $(libs)

cn.o: cn.c $(headers)
	$(CC) -c cn.c -o cn.o $(libs)

cuiEdit.o: cuiEdit.c $(headers)
	$(CC) -c cuiEdit.c -o cuiEdit.o $(libs)

run:
	./$(out)

.PHONY: clean
clean:
	rm *.o

.PHONY: all
all:
	make cn
	make run

