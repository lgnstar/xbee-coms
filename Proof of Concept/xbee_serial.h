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

#ifndef _XBEE_SERIAL_H
#define _XBEE_SERIAL_H

//------------Global Definition for Serial Port hand shaking----------

#define FALSE 0
#define TRUE 1

//#define BAUDRATE B115200
#define BAUDRATE B9600

#define MAX_BUFFER_SIZE 255

int global_serial_port_descriptor;

struct termios oldtio; //Just for saving old serail port settings
struct termios newtio; //These are the new serial port parameters

char * global_rx_buffer;
char * global_tx_buffer;

uint32_t command_buffer_ready = FALSE;

uint32_t in_char_index = 0;
uint32_t in_rx_index = 0;

uint32_t in_count2 = 0;
uint32_t no_input_count = 0;

uint32_t STOP = FALSE;

char port_name[MAX_BUFFER_SIZE] = "/dev/ttyUSB0";

//------------------Helper function Prototypes------------------------

int init_serial_port( int argc, char * argv[] );
int process_buffer( char * buffer );
int write_port( char * bfr );
void restore_old_port_settings(void);

//------------------End of Helper function Prototypes-----------------
#endif