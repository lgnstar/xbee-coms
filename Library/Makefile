app: main_test.o libxbee.o
	gcc -o app -g main_test.o libxbee.o

main_test.o: main_test.c libxbee.h
	gcc -c -g main_test.c libxbee.h

libxbee.o: libxbee.c libxbee.h
	gcc -c -g libxbee.c libxbee.h

clean:
	rm main_test.o
	rm libxbee.o
