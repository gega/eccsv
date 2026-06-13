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

typedef int ( *eccsv_cb_t ) ( eccsv_fieldp_t field, int col, size_t len, ECCSV_CTXTYPE ctx );

int eccsv_parse( eccsv_fieldp_t line, eccsv_cb_t cb, ECCSV_CTXTYPE ctx );


#ifdef ECCSV_IMPLEMENTATION

int eccsv_parse( eccsv_fieldp_t str, eccsv_cb_t cb, ECCSV_CTXTYPE ctx )
{
  int col = 0;
  eccsv_fieldp_t base = str;

  if ( NULL == str )
    return ( -3 );
  while ( '\n' == *str || '\r' == *str )
    str++;
  if ( *str == '\0' )
    return ( str - base );
  for ( ;; )
  {
    if ( *str == '"' )
    {
#if ECCSV_MUTABLE
      char *f = ++str,
          *w = f;

      for ( ;; )
      {
        char c = *str++;

        if ( !c )
          return ( -1 );
        if ( c == '"' )
        {
          if ( *str == '"' )
          {
            *w++ = '"';
            ++str;
            continue;
          }
          break;
        }
        *w++ = c;
      }
      if ( 0 != cb( f, col++, ( size_t )( w - f ), ctx ) )
        return ( -4 );
#else
      const char *f = ++str;
      size_t len = 0;

      for ( ;; )
      {
        char c = *str++;

        if ( !c )
          return ( -1 );
        if ( c == '"' )
        {
          if ( *str == '"' )
          {
            ++str;
            len += 2;
            continue;
          }
          break;
        }
        ++len;
      }
      if ( 0 != cb( f, col++, len, ctx ) )
        return ( -4 );
#endif
      if ( *str == ECCSV_SEP )
      {
        ++str;
        continue;
      }
      while ( '\n' == *str || '\r' == *str )
        str++;
      if ( !*str )
        return ( str - base );
      return ( -2 );
    }

    {
      eccsv_fieldp_t f = str;

      while ( *str && *str != ECCSV_SEP && *str != '\r' && *str != '\n' )
        ++str;
      if ( 0 != cb( f, col++, ( size_t )( str - f ), ctx ) )
        return ( -4 );
      if ( *str == ECCSV_SEP )
      {
        ++str;
        continue;
      }
      while ( '\n' == *str || '\r' == *str )
        str++;
      return ( str - base );
    }
  }
}
#endif
