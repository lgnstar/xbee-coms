/** @file xbee_serial.h
 *  @brief An asynchronous serial chat program.
 *
 * Copyright 2012 Adolph Seema, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * This is inspired by sample codes at:
 * http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
 * and
 * http://www.easysw.com/~mike/serial/serial.html
 *
 * Description: This is an example of how to do semi-asynchronous serial
 *              programming under linux.
 *              This was tested on a Dell Inspiron running Ubuntu 12.10.
 *              The program receives user input and processes it to make it an
 *              XBee compatible command to output to the serial port.
 *              The response from the XBee to each command it understands is
 *              then printed on the console. 
 *
 *              So this program can be executed as a basic terminal emulator.
 *              So, this is basically a serial asynchronous chat program with
 *              XBee module.
 *
 *              For demonstration purposes, the program accepts and
 *              responds to 1 command:
 *
 *              "exit" --- Response: Prints:
 *                         Exiting as ordered! Goodbye!.
 *                         Then it exits/shuts down.
 *
 *              If you type: "+++<ENTER>", from the keyboard the XBee should
 *              respond with: "OK<CR>".
 *
 *              If you type anything else followed by <ENTER>, from the
 *              the program sends out that stuff you typed followed by <CR>.
 *              The XBee will respond with appropriate response followed by 
 *              "<CR>". i.e. if it understands the command you just sent.
 *
 *  @author Adolph Seema - adolph<at>ieee.org
 *
 *  @bug No known bugs.
 *  @date 2013/02/04
 *....................................................................
 */

#ifndef _XBEE_SERIAL //Include Gaurd Begin
#define _XBEE_SERIAL

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include "xbee_serial.h"
#include "xbee_walker.h"

/** @brief Main program entrypoint.
 *
 *  This is the entrypoint for the asynchronous serial chat program.
 *  You can use this to test your serial programs, right now it uses
 *  '\n' character to detect end of message, either from the keyboard
 *  or serial port.
 *  It is tight while loop with no care for CPU time and power
 *  efficieny. This must be improved and a serious production
 *  software.
 *
 *  @param int argc   : Number arguments on the command line.
 *  @param char * argv: [0] - The name of this program's executable.
 *                    : [1] - The complete serial port name to be
 *                            opened.
 *
 *  @return int : Should not return
 *                0 - Successful exit.
 *                Not Zero - Error exit.
 *....................................................................
 */
int main( int argc, char* argv[] )
{
	if( argc < 2 )
	{
		printf( "\nUsage: ./xbee_serial \"<port_name>\"\n" );
		printf( "Example: ./xbee_serial \"/tmp/ttyS0\"\n" );
		return EXIT_SUCCESS;
	}

	global_rx_buffer = calloc( MAX_BUFFER_SIZE, sizeof(char) );
	global_tx_buffer = calloc( MAX_BUFFER_SIZE, sizeof(char) );

	fd_set readfs; //file descriptor set

	//First let's setup the serial port
	if( init_serial_port( argc, argv ) != EXIT_SUCCESS )
	{
		fprintf( stderr, "Serial Port Init Failed!\n" );
		exit(EXIT_FAILURE);
	}

	//maximum bit entry (file descriptors) to test including
	//stdin, stdout and stderr
	int maxfd = global_serial_port_descriptor+3;

	//'\n' NEEDED by TTY
	printf( "%u-INPUT:\n", no_input_count );

	while( STOP == FALSE )
	{
		struct timeval timeout;
		timeout.tv_sec = 10;  //seconds
		timeout.tv_usec = 0; //microseconds

		//defining listeners
		FD_ZERO( &readfs );
		//set testing for source 1; This is the board
		FD_SET( global_serial_port_descriptor, &readfs);
		//set testing for source 2; This is the keyboard
		FD_SET( STDIN_FILENO, &readfs );

		int ret_value = select( maxfd, &readfs, NULL, NULL, &timeout );

		if( ret_value > 0 ) //We have data, cool!
		{
			//input from source 1 available
			if( FD_ISSET(global_serial_port_descriptor, &readfs) )
			{
				char rx_char;

				if( read( global_serial_port_descriptor, &rx_char, 1 ) )
				{
					global_rx_buffer[in_rx_index] = rx_char;
					in_rx_index++;

					//printf( "rx_char=[%c]=%d=[%s].\n", rx_char, in_rx_index, global_rx_buffer );

					//XBee responses are terminated by carriage return=<cr>='\r'=13=0x0d
					if( rx_char == '\r' )
					{
						//stringify message and overwrite XBee's <CR>.
						global_rx_buffer[in_rx_index-1] = '\0';

						command_buffer_ready = TRUE;

						in_rx_index = 0;

					}//END preparing end of message-----------------------------

				}//END testing for serial port input--------------------------

			}//finished processing input from source 1

			//here we can monitor buffers to see what commands we get and
			//and do certian things based in whats in the command buffer.
			if( command_buffer_ready == TRUE )
			{
				process_buffer( global_rx_buffer );
			}

			//keyboard input: Remember, keyboard processes input only after it sees:
			//newline cahacter='\n'=nl=10=0x0a.
			//So, ignore the extra '\n' from the keyboard and ensure it is not added
			//to the final command sent to XBee or this program.
			if( FD_ISSET(STDIN_FILENO, &readfs) ) //source 2 input available
			{
				char in_char;
				in_count2++; //testing how many times we received input2

				if( read( STDIN_FILENO, &in_char, 1 ) )
				{
					global_tx_buffer[in_char_index] = in_char;
					in_char_index++;

					//printf( "in_char=[%c]=%d=[%s].\n", in_char, in_char_index, global_tx_buffer );

					if( in_char == '\n' ) //Implies end of keyboard command message
					{
						//stringify message and overwrite the keyboard's <nl> with <CR>
						global_tx_buffer[in_char_index-1] = '\r';
						global_tx_buffer[in_char_index] = '\0'; //terminate like string

						//printf( "global_tx_buffer=[%d]=[%d]=[%s].\n", 
						//        (int)strlen(global_tx_buffer), 
						//        in_char_index,
						//        global_tx_buffer );

						command_buffer_ready = TRUE;

						in_char_index = 0;

					}//END processing end of message----------------------------

				}//END testing for standard/keyboard input

			}//finished processing input from source 2

			//here we can monitor buffers to see what commands we get and
			//and do certian things based in whats in the command buffer.
			if( command_buffer_ready == TRUE )
			{
				process_buffer( global_tx_buffer );
			}

		}
		else if( ret_value == 0 )
		{
			no_input_count++;
		}
		else
		{
			printf("\nWaiting for STDIN_FILENO=%d failed with error[%d].\n",
					STDIN_FILENO,
					errno
		    );

			return 1;
		}//end inut check
	}//END of while loop, we should never go past here :)---------------
	
	return 0;

}//----------------------------END of main----------------------------

/** @brief Prints and processes the string commands, and reponds to
 *         them.
 *
 *  For demonstration purposes, the program accepts and responds to 3
 *  commands:
 *
 *              "exit" --- Response: Prints:
 *                         Exiting as ordered! Goodbye!.
 *                         Then it exits/shuts down.
 *
 *              "AT hello" --- Response: Sends:
 *                             AT Thank you! and Hello to you too!
 *
 *              "AT port?" --- Response: Sends:
 *                             I am at port: <it's tty port name>
 *
 *
 *  @param buffer The command to be parsed, printed or logged as
 *                desired.
 *
 *  @return int   0       : Success
 *                Not Zero: Error
 *....................................................................
 */
int process_buffer( char * buffer )
{
	if( buffer < (char *)1 ) //in case of NULL we want to avoid crushing
	{
		return 1;
	}

	if( strlen(buffer) < 1 ) //in case of zero length, don't bother
	{
		return 2;
	}

	if( buffer == global_tx_buffer ) 
	{
		if( strncmp( buffer, "exit", 4 ) == 0 ) 
		{
			printf( "\nExiting as ordered! Goodbye!.\n" );
			exit( EXIT_SUCCESS );
		} 
		else 
		{
			//The only XBee command that does not end with <CR> is "+++"
			if( strncmp( buffer, "+++", 3 ) == 0 )
			{
				//lets remove it
				buffer[3] = '\0'; 
			}
			else if ( strncmp( buffer, "get_ip", 6 ) == 0 )
			{
				buffer = get_IP();
			}

			write_port( buffer );
			printf( "==>OUT:[%s]\n", buffer );
		}
	}
	else
	{
		printf( "<===IN:[%s]\n", buffer );
	}

	buffer[0] = '\0'; //clear the buffer
	printf( "cleaned[%s]\n", buffer );

	command_buffer_ready = FALSE;

	return EXIT_SUCCESS;

}//END process_buffer----------------------------------------------------------

/** @brief Writes the buffer given to serial port..............................
 *
 *  This assumes that the port has already been opened and initialized
 *  successfully.
 *
 *  N.B. Do not use this to send any data that is not a null
 *       terminated string. This is just a convenience function. If
 *       you want to send raw data only or a file that is not a legal
 *       string, then use the write function directly and do the
 *       proper error checking and termination.
 *
 *  @param char * bfr: The pointer to the byte/char buffer data to be
 *                     written or sent to/over the serial port.
 *
 *  @return int : 0 - Success.
 *                Not Zero - Error.
 *.............................................................................
 */
int write_port( char * bfr )
{
	int write_count = write( global_serial_port_descriptor,
                             bfr,
                             strlen(bfr) );

    if( write_count < 0 )
    {
        printf( "Writing [%s] to serial port[%s] FAILED(%d)!\n",
                bfr,
                port_name,
                write_count );
	}

    return EXIT_SUCCESS;

}//END write_port..............................................................

/** @brief Restores the port settings the program saved before
 *         opening the port for itself.
 *
 *  This is registered by the init_serial_port function to be executed
 *  when the program exits for any reason.
 *
 *  @param void.
 *  @return void.
 *.............................................................................
 */
void restore_old_port_settings(void)
{
    free( global_rx_buffer );
    free( global_tx_buffer );

    //restore old port settings if there was any.
    tcsetattr( global_serial_port_descriptor, TCSANOW, &oldtio );
}//END restore_old_port_settings-----------------------------------------------

/** @brief Initializes the serial port settings and opens the serial
 *         port.
 *
 *  Before opening the port for itself this functions saves the old
 *  port settings. It also registers the restore_old_port_settings
 *  function to be executed when the program exits for any reason.
 *
 *  @param int argc   : Number arguments on the command line.
 *  @param char * argv: [0] - The name of this program's executable.
 *                    : [1] - The complete serial port name to be
 *                            opened.
 *
 *  @return int : 0 - Success.
 *                Not Zero - Error.
 *.............................................................................
 */
int init_serial_port( int argc, char * argv[] )
{
	if( argc > 1 ) //If user passes their own full serial port name
	{ //then use it instead
		strncpy( port_name, argv[1], MAX_BUFFER_SIZE );
	}//else use the default one

	//We are ignoring SIGIO signal since we are using "select" instead
	signal( SIGIO, SIG_IGN );

	//Open device to be non-blocking (read will return immediately)
	global_serial_port_descriptor = open( port_name,
					      O_RDWR |
			                      O_NOCTTY |
				              O_NONBLOCK );

	if( global_serial_port_descriptor <= 0 )
	{
		printf( "\nSerial port[%s] failed to open with error[%d].\n",
			port_name,
			errno
			);
		return 1;
	}

	printf( "\nSerial port[%s] was successfully opened.\n",
		    port_name
			);

	//Make serial port do asynchronous input/outpuAt.
	int ret_value = fcntl( global_serial_port_descriptor,
		               F_SETFL,
			       O_ASYNC );
	if( ret_value != 0 )
	{
		printf( "\nfcntl F_SETFL O_ASYNC on new port[%s] failed with "\
			"error[%d].\n",
			port_name,
			errno
			);
		return 2;
	}

	printf( "\nfcntl F_SETFL O_ASYNC on new port[%s] succeeded.\n",
		      port_name
			);

	//save old port settings
	ret_value = tcgetattr( global_serial_port_descriptor, &oldtio );
	if( ret_value != 0 )
	{
		printf( "\nPreserving old port[%s] failed with error[%d].\n",
			    port_name,
				errno
				);
		return 3;
	}

	printf( "\nPreserving old port[%s] was successful.\n",
		    port_name
			);
	
	//register this function to be executed if program exits.
	atexit( restore_old_port_settings );

	//clear up struct for new port
	if( memset( &newtio, 0, sizeof(newtio) ) < (void *)1 )
	{
		printf( "\nClearing up new port[%s] failed with error[%d].\n",
			    port_name,
				errno
				);
		return 4;
	}

	printf( "\nClearing up new port[%s] was successful.\n",
		    port_name
			);

	//set new port 8N1 settings for non-canonical input processing
	//must be NOCTTY, set baud rate
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	//(ignore parity | map '\r' to '\n')
	newtio.c_iflag = ( IGNPAR | ICRNL );
	newtio.c_oflag = 0; //ONLCR converts '\n' to CR-LF pairs
	newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_cc[VMIN] = 0;  //return as soon as there is at least 1-byte
	newtio.c_cc[VTIME] = 1; //otherwise it returns in 0.1 s regardless.

	ret_value = tcflush( global_serial_port_descriptor, TCIFLUSH );
	if( ret_value != 0 )
	{
		printf( "\nSetting up, flushing Input data to port[%s] "\
			    "if not read, failed with error[%d].\n",
				port_name,
				errno
		);
		return 5;
	}

	printf( "\nSetting up, flushing Input data to port[%s] "\
		    "if not read, was successful.\n",
			port_name
			);

	ret_value = tcsetattr( global_serial_port_descriptor,
		                     TCSANOW,
			                 &newtio );
	if( ret_value != 0 )
	{
		printf( "\nActivating port[%s] settings failed with error[%d].\n",
			    port_name,
				errno
				);
		return 6;
	}

	printf( "\nActivating port[%s] settings successful.\n",
		    port_name
			);

	return EXIT_SUCCESS;

}//END init_serial_port-----------------------------------------------

//..............................END OF FILE...........................

#endif //Include Guard End
