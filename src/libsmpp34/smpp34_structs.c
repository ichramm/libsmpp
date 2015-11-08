/*
 * Copyright (C) 2006 Movilgate SRL.
 * File  : smpp34_structs.c
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

/* GLOBALS ********************************************************************/
// GCC requires that variables should be initialized on the declaration

#if !defined(_WIN32)
__thread int smpp34_errno = 0;
__thread char smpp34_strerror[2048] = {0};
__thread char *ptrerror = NULL;
#endif

/* EXTERN *********************************************************************/
/* FUNCTIONS ******************************************************************/
