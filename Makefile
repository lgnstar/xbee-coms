app: xbee_serial.o xbee_walker.o
	gcc -o app xbee_serial.o xbee_walker.o

xbee_serial.o: xbee_serial.c xbee_serial.h xbee_walker.h
	gcc -c xbee_serial.c

xbee_walker.o: xbee_walker.c xbee_walker.h
	gcc -c xbee_walker.c
clean:
	rm xbee_serial.o
	rm xbee_walker.o
