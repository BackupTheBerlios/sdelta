/*

This is Kyle Sallee's implementation of Mark Adler's 
excellent and fast 32-bit cyclic redundancy check.
An alternate example and longer explanation of adler32
is available at http://www.ietf.org/rfc/rfc1950.txt

*/

#include "adler32.h"

u_int32_t adler32(unsigned char *b, u_int32_t s) {
  u_int32_t	s1, s2;
  DWORD		w;

  s1 = 1;
  s2 = 0;

  for ( ; s > 0 ; s--, b++)  {
      s1   += *b;
      s2   += s1;
/*    s1   += *b;  if  ( s1 >= 65521 )  s1 -= 65521;
      s2   += s1;  if  ( s2 >= 65521 )  s2 -= 65521;
*/
  }

/*  while  ( s1 >= 65521 )  s1 -= 65521;  */
  while  ( s2 >= 65521 )  s2 -= 65521;


  w.word.low   =  ( u_int16_t )s1;
  w.word.high  =  ( u_int16_t )s2;

  return  w.dword;
}
