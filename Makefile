all: exe

exe: main.c
	gcc main.c -o cw -lm -lpng
