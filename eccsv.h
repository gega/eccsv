#include <stddef.h>

#ifndef ECCSV_SEP
#define ECCSV_SEP ','
#endif

#ifndef ECCSV_MUTABLE
#define ECCSV_MUTABLE 1
#endif

#ifndef ECCSV_CTXTYPE
#define ECCSV_CTXTYPE void *
#endif

#if ECCSV_MUTABLE
typedef char *eccsv_fieldp_t;
#else
typedef const char *eccsv_fieldp_t;
#endif

typedef void ( *eccsv_cb_t ) ( eccsv_fieldp_t field, int col, size_t len, ECCSV_CTXTYPE ctx );

int eccsv_parse( eccsv_fieldp_t s, eccsv_cb_t cb, ECCSV_CTXTYPE ctx );


#ifdef ECCSV_IMPLEMENTATION

int eccsv_parse( eccsv_fieldp_t s, eccsv_cb_t cb, ECCSV_CTXTYPE ctx )
{
  int col = 0;
  eccsv_fieldp_t base = s;

  if ( NULL == s ) return ( -3 );
  while ( '\n' == *s || '\r' == *s ) s++;
  if ( *s == '\0' ) return ( s - base );
  for ( ;; )
  {
    if ( *s == '"' )
    {
#if ECCSV_MUTABLE
      char *f = ++s,
           *w = f;

      for ( ;; )
      {
        char c = *s++;

        if ( !c ) return ( -1 );
        if ( c == '"' )
        {
          if ( *s == '"' )
          {
            *w++ = '"';
            ++s;
            continue;
          }
          break;
        }
        *w++ = c;
      }
      cb( f, col++, ( size_t )( w - f ), ctx );
#else
      const char *f = ++s;
      size_t len = 0;

      for ( ;; )
      {
        char c = *s++;

        if ( !c ) return ( -1 );
        if ( c == '"' )
        {
          if ( *s == '"' )
          {
            ++s;
            len += 2;
            continue;
          }
          break;
        }
        ++len;
      }
      cb( f, col++, len, ctx );
#endif
      if ( *s == ECCSV_SEP )
      {
        ++s;
        continue;
      }
      while ( '\n' == *s || '\r' == *s ) s++;
      if ( !*s ) return ( s - base );
      return ( -2 );
    }

    {
      eccsv_fieldp_t f = s;

      while ( *s && *s != ECCSV_SEP && *s != '\r' && *s != '\n' ) ++s;
      cb( f, col++, ( size_t )( s - f ), ctx );
      if ( *s == ECCSV_SEP )
      {
        ++s;
        continue;
      }
      while ( '\n' == *s || '\r' == *s ) s++;
      return ( s - base );
    }
  }
}
#endif
