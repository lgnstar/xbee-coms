/** @file libxbee.c
 ** @brief Implementation of the libxbee.h
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Description: This file contains the implementation of functions described in the 
 *				libxbee.h file.
 *
 * @author Steven Hatch - sthatch@asu.edu
 *
 * @bugs
 * @date 4-11-2013
 */
#include <sys/signal.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include "libxbee.h"

/* @breif Initializes and opens the provided serial port
 *
 * Header files needed: signal.h
 *						termios.h
 *						string.h
 *						stdio.h
 *						errno.h
 *						unistd.h
 *						fcntl.h
 *						termios.h
 *						sys/time.h
 *
 * @param char * port: The complete name of the port to be opened
 *
 * @return :		0 - success
 *			 Not Zero - Error
 */
int init_port(char * port)
{
	int ret_value;

	if( strlen( port ) > 255)
	{
		printf(" The port name[%s] is too long. Port names must be less than %d characters.", 
			   port, 
			   MAX_BUFFER_SIZE );

		return 1;
	}

	//Copy the the provided port name into the global variable port_name to be used in other functions
	strncpy( port_name, port, MAX_BUFFER_SIZE );

	// SIGIO is used for interrupt-driven input
	// We are ignoring SIGIO because we are using the select function instead
	signal( SIGIO, SIG_IGN );

	port_descriptor = open( port_name,		//Name of the port
							O_RDWR |		//Open port for reading and writing
			                O_NOCTTY |		//The device is not the controlling terminal for the process
				            O_NONBLOCK );	//The port read will return immediately

	if( port_descriptor <= 0 )
	{
		printf( "\nSerial port[%s] failed to open with error[%d].\n",
				port_name,
				errno );

		return 2;
	}

	//Set the serial port(or the open file descriptor port_descriptor) up for asynchronous input/output.
	ret_value = fcntl( port_descriptor,
					   F_SETFL,		//Set the file status flags to the value specified by O_ASYNC
					   O_ASYNC );	//Enable the port_descriptor for asynchronous communication

	if( ret_value != 0 )
	{
		printf( "\nfcntl F_SETFL O_ASYNC on new port[%s] failed with error[%d].\n",
				port_name,
				errno );

		return 3;
	}

	//Set timeout values
	timeout.tv_sec = TIMEOUT_SEC;	//seconds
	timeout.tv_usec = TIMEOUT_USEC; //microseconds

	//Set max file descriptor(maxfd) value
	maxfd = port_descriptor + 3;

	//clear up struct for new port
	if( memset( &newtio, 0, sizeof(newtio) ) < (void *)1 )
	{
		printf( "\nClearing up new port[%s] failed with error[%d].\n",
			    port_name,
				errno );
		
		return 4;
	}

	//Set Control modes
	newtio.c_cflag = ( BAUDRATE |	//Set the Baud rate
					   CS8 |		//Set character size mask to 8 bits
					   CLOCAL |		//Ignore modem control lines
					   CREAD );		//Enable Receiver

	//Set input modes
	newtio.c_iflag = ( IGNPAR |		//Ignore framing and parity errors
					   ICRNL );		//Translate carriage return('\r') to newline('\n') on input

	//Set output modes
	newtio.c_oflag = 0;				//Turn off output processing

	//Set local modes
	newtio.c_lflag = ~(ICANON |		//Enable canonical mode
					   ECHO |		//Echo input characters
					   ECHOE |		//ERASE character erases the preceding input character, and WERASE erases the preceding word
					   ISIG );		//When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding signal

	//Set special charaters
	newtio.c_cc[VMIN] = 0;			//return as soon as there is at least 1-byte
	newtio.c_cc[VTIME] = 1;			//otherwise it returns in 0.1 s regardless.

	//Flush data received but not read
	ret_value = tcflush( port_descriptor, TCIFLUSH );

	if( ret_value != 0 )
	{
		printf( "\nSetting up, flushing Input data to port[%s] "\
			    "if not read, failed with error[%d].\n",
				port_name,
				errno );
		return 5;
	}

	//Write the attributes to the descriptor
	ret_value = tcsetattr( port_descriptor,	
		                   TCSANOW,			//Chanegs take place immediatley
			               &newtio );

	if( ret_value != 0 )
	{
		printf( "\nActivating port[%s] settings failed with error[%d].\n",
			    port_name,
				errno
				);

		return 6;
	}

	printf( "\nSuccessfully established communication with device at port[%s].\n",
		    port_name );

	return 0;
}//----- End ----- init_port(char * port)---------------------------------


/* @breif Writes the given data to the initialized port
 *
 * Header files needed: unistd.h
 *
 * @param char * buffer: The data to write to the port
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int write_port( char * buffer )
{
	int write_count = write( port_descriptor,
                             buffer,			//Data to write to the port
                             strlen(buffer) );	//Count of characters to write

    if( write_count < 0 )
    {
        printf( "Writing [%s] to serial port[%s] failed(%d)!\n",
                buffer,
                port_name,
                write_count );
	}

    return 0;

}//----- End ----- write_port(char * buffer)-----------------------------------


/* @breif Reads one byte from the initialized port and stores the byte
 *		  at the end of the buffer provided
 *
 * Header files needed: unistd.h
 *
 * @param char * buffer: Data read from the port will be stored here
 *
 * @return:		   0 - success
 *			Not Zero - All data has been received
 */
int read_port( char * buffer )
{
	char rx_char;
	int index = strlen( buffer );
	int result = 0;

	if( read( port_descriptor, &rx_char, 1 ) )
	{
		//XBee responses are terminated by carriage return=<cr>='\r'=13=0x0d
		if( rx_char == '\r' )
		{
			buffer[index] = '\0';	//Add the null char to finish the string
			result = 1;
		}
		else
			buffer[index] = rx_char;

	}//END testing for serial port input--------------------------

    return result;

}//----- End ----- read_port(char * buffer)-------------------------------

/* @breif 
 *
 * Header files needed: sys/time.h
 *						sys/types.h
 *						unistd.h
 *							or
 *						sys/select.h
 *
 * @param count: The number of file descriptors provided
 * @param fds: An array of file descriptors to be monitored
 *
 * @return:		   0 - No file descriptor is ready
 *			Not Zero - The file descriptor that is ready for communication
 */
int check_descriptors( int count, int fds[] )
{
	int result; 

	FD_ZERO( &readfs );		//Initialize the readfs set to 0

	while( count > -1)
	{
		count--;
		FD_SET( fds[count], &readfs ); //Add each provided file descriptor to the set
	}//END----- while -----

	result = select( maxfd, &readfs, NULL, NULL, &timeout );

	return result;
}//----- End ----- check_descriptors( int fds[] )-------------------------

/* @breif Communicates with the module and puts it into command mode
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int enter_command_mode( void )
{
		int fds[1];
		int result = 0;
		char rx[MAX_BUFFER_SIZE];
		
		//We only want to watch the descriptor associated with the hardware
		fds[0] = port_descriptor;	

		write_port( "+++\0" );
		
		while( result == 0)
		{
			if( check_descriptors( 1, fds ) > 0 ) //If true, the port is ready
			{
				result = read_port( rx );
			}
		}

		return ( strncmp( rx, "OK", 2 ) == 0 );
}//----- End ----- enter_command_mode( void )-----------------------------