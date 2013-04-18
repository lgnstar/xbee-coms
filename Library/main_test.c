/** @file main.c
 ** @brief Library of functions xbee modules
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Description: This program is designed to test the libxbee library
 *
 * @author Steven Hatch - sthatch@asu.edu
 *
 * @bugs
 * @date 4-13-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include "libxbee.h"

void display( void )
{
        printf( "\nProgram choices:\n"\
                "   1. Open a port\n"\
                "   2. Enter Command Mode\n"\
                "   3. Exit Command Mode\n"\
		"   4. Get IP Address\n"\
                "   0. Exit program\n"\
                "\nSelect a number:" );
}

int main( int argc, char * argv[] )
{
	int choice = -1;
	int result;
	char * port;
	char buffer[MAX_BUFFER_SIZE];

	printf( "\nTest program for libxbee library version 1.0\n" );

	init_port( "" );
	
	display( );
	
	while( scanf( "%d", &choice ) != 0 )
	{
		switch( choice )
                {
                        case 0:
                                printf( "\nGoodbye\n\n" );
				exit( 0 );
				break;
			case 1:
				printf( "\nEnter port name:" );
				scanf( "%s", port );

				init_port( port );
				break;
                        case 2:				
				result = enter_command_mode( );

				if( result == 0 )
					printf( "\nSuccessfully entered command mode.\n" );
				else
					printf( "\nResult = %d\n", result );

				break;
			case 3:
				result = exit_command_mode( );

                                if( result == 0 )
                                        printf( "\nSuccessfully exited command mode.\n" );
                                else
                                        printf( "\nResult = %d\n", result );
                                
                                break;
			case 4:
				result = get_ip( buffer );
				
				if( result == 0 )
					printf( "\nIP Address: %s\n", buffer);
				else
					printf( "\nResult = %d\n", result );
				break;
                        default:
                                printf( "\nInvalid Choice. Please try again.\n" );
				break;
                }//END SWITCH

		//printf( "\nChoice = %d\n", choice ); //DEBUG

		display( );
	}// END WHILE
}//-----End-----int main( int argc, char * argv[] )-----------------------
