.PHONY=all clean

all:
	gcc main.c -c -o main.o
	gcc ../fs.c -c -o fs.o -I ../../include/ -I ../../
	gcc ../super.c -c -o super.o -I ../../include/ -I ../../
	gcc ../inode.c -c -o inode.o -I ../../include/ -I ../../
	gcc -o tester main.o fs.o super.o inode.o

clean:
	rm -rf tester main.o super.o inode.o fs.o
