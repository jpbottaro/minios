.PHONY=all clean

all:
	gcc main.c -c -o main.o -I ../../include/
	gcc ../fs.c -c -o fs.o -I ../../include/
	gcc ../super.c -c -o super.o -I ../../include/
	gcc ../inode.c -c -o inode.o -I ../../include/
	gcc ../file.c -c -o file.o -I ../../include/
	gcc ../../lib/misc.c -c -o misc.o -I ../../include/
	gcc -o tester main.o fs.o super.o inode.o file.o misc.o

clean:
	rm -rf tester main.o super.o inode.o fs.o file.o misc.o
