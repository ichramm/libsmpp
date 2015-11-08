/*
 * Copyright (C) 1999-2001 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 *
 * This file was contributed by Jeroen @ Mobile Tidings (http://mobiletidings.com)
 */
#ifndef OPENSMPP_ICONV_GSM7_H__
#define OPENSMPP_ICONV_GSM7_H__

/* Our own notion of wide character, as UCS-4, according to ISO-10646-1. */
typedef unsigned int ucs4_t;

#ifndef RET_ILUNI
/* Return code if invalid. (xxx_wctomb) */
# define RET_ILUNI      -1
#endif

#ifndef RET_ILSEQ
/* Return code if invalid. (xxx_mbtowc) */
# define RET_ILSEQ      -1
#endif

#ifndef RET_TOOSMALL
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
# define RET_TOOSMALL   -2
#endif

#ifndef RET_TOOFEW
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
# define RET_TOOFEW(n)  (-2-(n))
#endif

#ifdef  __cplusplus
extern "C"
{
#endif



int gsm7_encode(const unsigned char *from, int fromSize, unsigned char *ret, int retSize);


//bueno, lo voy debiendo, seria desarmar lo anterior
int gsm7_decode(const unsigned char *from, int fromSize, unsigned char *ret, int retSize);


int gsm7_mbtowc (ucs4_t *pwc, const unsigned char *s, int n);


int gsm7_wctomb (unsigned char *r, ucs4_t wc, int n);


#ifdef  __cplusplus
}
#endif


#endif // OPENSMPP_ICONV_GSM7_H__
