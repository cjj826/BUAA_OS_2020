all: code/fibo.c code/main.c
	make -C code
	gcc code/fibo.c code/main.c -o fibo -I include
clean:
	make -C code clean
