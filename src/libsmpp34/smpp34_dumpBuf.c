/*
 * Copyright (C) 2006 Movilgate SRL.
 * File  : smpp34_dumpBuf.c
 * Author: Raul Tremsal <rtremsal@movilgate.com>
 *
 * This file is part of libsmpp34 (c-open-smpp3.4 library).
 *
 * The libsmpp34 library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>

#ifdef __linux__
#include <netinet/in.h>
#endif

#include "smpp34.h"
#include "smpp34_structs.h"
#include "smpp34_params.h"

#if _MSC_VER > 1000
#pragma warning(disable: 4996) // '_snprintf': This function or variable may be unsafe. Consider using _snprintf_s instead.
#endif

/* FUNCTIONS ******************************************************************/
int
smpp34_dumpBuf(uint8_t *dest, int destL, uint8_t *src, int srcL)
{

    int i;
    int j;
    int size;
    int ind = 3;
    char *buffer = NULL;
    int lefterror = 0;

    size   = srcL;
    buffer = (char *) src;

	smpp34_err_init();
    lefterror = smpp34_error_size;

    /* dump buffer character by character until size is reached */
    for(i = 0; i < size; i++){
        switch( i % 16 ) {
            case 0:
                dest += sprintf( (char *)dest, "%*c%02X ", ind, ' ', (uint8_t)buffer[i]);
                break;

            case 7:
                dest += sprintf((char *)dest, "%02X   ", (uint8_t)buffer[i]);
                break;

            case 15:
                dest += sprintf((char *)dest, "%02X   ", (uint8_t)buffer[i]);
                for(j = (i - 15); j <= i; j++) {
                    if ( (buffer[j] < ' ') || (buffer[j] > '~') )
                        dest += sprintf((char *)dest, ".");
                    else
                        dest += sprintf((char *)dest, "%c", buffer[j]);
                    if ( (j % 16) == 7 )
                        dest += sprintf((char *)dest, " ");
                }
                dest += sprintf((char *)dest, "\n");
                break;

            default:
                dest += sprintf( (char *)dest, "%02X ", (uint8_t)buffer[i]);
                break;
        }
    };

    /* if the line is not completed, we have to fill it */
    if ( (size % 16) != 0 ) {
        for (i = (size % 16); i < 16; i++) {
            dest += sprintf((char *)dest, "   ");
            if ( (i % 16) == 7 )
                dest += sprintf((char *)dest, "  ");
        }
        dest += sprintf((char *)dest, "  ");
        for (j = size - (size % 16); j < size; j++) {
            /* check if character is printable */
            if ( (buffer[j] < ' ') || (buffer[j] > '~') )
                dest += sprintf((char *)dest, ".");
            else
                dest += sprintf((char *)dest, "%c", (char) buffer[j]);
            if ( (j % 16) == 7 )
                dest += sprintf((char *)dest, " ");
        }
        dest += sprintf((char *)dest, "\n");
    }

    *dest = '\0';
    return( 0 );
};


