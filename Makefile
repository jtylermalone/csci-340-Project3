all: spro_mcon

clean: 
	rm -f spro_mcon

spro_mcon: spro_mcon.c
	gcc -Wall -pthread -o spro_mcon spro_mcon.c
