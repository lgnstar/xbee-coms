app: main_test.o libxbee.o
	gcc -o app main_test.o libxbee.o

main_test.o: main_test.c libxbee.h
	gcc -c main_test.c libxbee.h

libxbee.o: libxbee.c libxbee.h
	gcc -c libxbee.c libxbee.h

clean:
	rm main_test.o
	rm libxbee.o
