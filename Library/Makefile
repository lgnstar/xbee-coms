app: main.o libxbee.o
	gcc -o app main.o libxbee.o

main.o: main.c libxbee.h
	gcc -c main.c libxbee.h

libxbee.o: libxbee.c libxbee.h
	gcc -c libxbee.c libxbee.h

clean:
	rm main.o
	rm libxbee.o
