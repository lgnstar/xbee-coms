/** @file libxbee.h
 ** @brief Library of functions xbee modules
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Description: This file contains a library of functions that enable communication
 *				between xbee modules and computer and communiction between two xbee 
 *				modules. The library is intended to support other programs that wish
 *				to use xbee modules as a communication device.
 *
 *				This library provides an initialization function to prepare the xbee
 *				module for serial communication. It also provides serial receive(Rx) 
 *				and	transmission(Tx) functions and a simplified interface for running
 *				AT commands. Each AT command has a get and set function that manages 
 *				entering and exiting command mode automatically.
 *
 * @author Steven Hatch - sthatch@asu.edu
 *
 * @bugs
 * @date 4-11-2013
 */

#ifndef LIBXBEE_H
#define LIBXBEE_H

#include <sys/select.h>
#include <termios.h>

//-----------------Global Variable Definitions-------------------------------------
#define TRUE 1
#define FALSE 0

#define MAX_BUFFER_SIZE 255
#define BAUDRATE B9600

//The following two constants define the length of time that the select system call
//will wait for input from the specified descriptors
#define TIMEOUT_SEC 0;
#define TIMEOUT_USEC 1000;

int port_descriptor;				//Used to define the port associated with the device
char port_name[MAX_BUFFER_SIZE];
struct timeval timeout;				//Used to set timeout value for serial port
struct termios newtio;				//Contains parameters for the serial port
int maxfd;							//Used by the select function to define its search
fd_set readfs;						//A set of files descriptors for the select system call to check for readiness

//---------------End Global Variable Definitions-----------------------------------


//-----------------Function Prototypes---------------------------------------------

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
 * @param char * port: The complete name of the port to be opened(i.e. /dev/ttyUSB0)
 *
 * @return:			0 - success
 *			 Not Zero - Error
 */
int init_port( char * );

/* @breif Writes the given data to the initialized port
 *
 * Header files needed: unistd.h
 *
 * @param char * buffer: The data to write to the port
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int write_port( char * );

/* @breif Reads one byte from the initialized port and stores the byte
 *		  at the end of the buffer provided
 *
 * Header files needed: unistd.h
 *
 * @param char * buffer: Data read from the port will be stored here
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int read_port( char * );

/* @breif 
 *
 * Header files needed: 
 *
 * @param 
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int check_descriptors( int, int [] );

/* @breif Communicates with the module and puts it into command mode
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int enter_command_mode( void );

/* @breif 
 *
 * Header files needed: 
 *
 * @param 
 *
 * @return:		   0 - success
 *			Not Zero - Error
 */
int exit_command_mode( void );
//---------------End Function Prototypes-------------------------------------------
#endif //Include Gaurd End