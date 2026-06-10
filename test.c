#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ctx;
#define ECCSV_CTXTYPE struct ctx *
#define ECCSV_IMPLEMENTATION
#include "eccsv.h"

static const char *csv[] = {
  "\"aa\na\",bbb,",             	// 0
  "aaa,   \"bbb\" ,ccc",        	// 1
  "  aaa  ,\"b\"\",,,,b\", ccc  ",      // 2
  "aaa,b\"bb,",                 	// 3
  "aaa,\" ccc \", ccc ,XX",     	// 4
  ",,",                         	// 5
  NULL
};

static const char *fields[] = {
  "aa\na",
  "bbb",
  "",
  "aaa",
  "   \"bbb\" ",
  "ccc",
  "  aaa  ",
  "b\",,,,b",
  " ccc  ",
  "aaa",
  "b\"bb",
  "",
  "aaa",
  " ccc ",
  " ccc ",
  "XX",
  "",
  "",
  "",
  NULL
};

/* 
 * generated with:
 * find csv-test-data/json -type f|grep -v /bad|grep -v /header|grep -v utf8|awk '{print "echo -n \"\\\"$(basename " $1 " .json)\\\", \" ; jq -r \"flatten | map(@json) | join(\\\", \\\")\" " $1 "; echo , NULL, "}'|bash
 */
const char *testfiles[] = {
  "trailing-newline", "foo", "bar", "baz", "1", "2", "3", NULL,
  "quotes-with-newline", "foo", "bar", "baz", "1", "No man is an island,\nEntire of itself", "3", NULL,
  "quotes-empty", "foo", "bar", "baz", "1", "", "3", NULL,
  "one-column", "foo", "1", NULL,
  "quotes-with-comma", "foo", "bar", "baz", "1", "Luke, I am your father.", "3", NULL,
  "quotes-with-space", "foo", "bar", "baz", "1", "Field with spaces", "3", NULL,
  "trailing-newline-one-field", "foo", "1", NULL,
  "quotes-with-escaped-quote", "foo", "bar", "baz", "1", "The \" must be escaped", "3", NULL,
  "all-empty", "", "", NULL,
  "simple-crlf", "foo", "bar", "baz", "1", "2", "3", NULL,
  "empty-one-column", "foo", "", NULL,
  "leading-space", "foo", "bar", "baz", "1", " leading space", "3", NULL,
  "simple-lf", "foo", "bar", "baz", "1", "2", "3", NULL,
  "empty-field", "foo", "bar", "baz", "1", "", "3", NULL,
  "trailing-space", "foo", "bar", "baz", "1", "trailing space ", "3", NULL,

  NULL
};

struct ctx
{
  int r;
  int j;
  int i;
  const char **expected;
};

void print_escaped( const char *s, size_t len, char *postfix )
{
  while ( *s && len > 0 )
  {
    switch ( *s )
    {
      case '\n':
        printf( "\\n" );
        break;
      case '\r':
        printf( "\\r" );
        break;
      case '\t':
        printf( "\\t" );
        break;
      case '\f':
        printf( "\\f" );
        break;
      case '\v':
        printf( "\\v" );
        break;
      case ' ':
        printf( "·" );
        break;                  // optional
      default:
        if ( ( unsigned char )*s < 32 || ( unsigned char )*s == 127 )
          printf( "\\x%02X", ( unsigned char )*s );
        else
          putchar( *s );
    }
    s++;
    len--;
  }
  printf( postfix );
}

int cb( eccsv_fieldp_t f, int col, size_t len, ECCSV_CTXTYPE c )
{
  printf( "\tcsv %d: #%d %s [%ld] '", c->j, c->i, ( memcmp( c->expected[c->r], f, len ) == 0 ? " OK " : "FAIL" ), len );
  print_escaped( f, len, "' =? '" );
  print_escaped( c->expected[c->r], strlen( c->expected[c->r] ), "'\n" );
  c->i++;
  c->r++;
  return(0);
}

#define CSV_PATH "csv-test-data/csv/"
static char *loadfile( const char *name )
{
  char n[260];
  char *ret = NULL;
  snprintf( n, sizeof( n ), "%s%s.csv", CSV_PATH, name );
  FILE *f = fopen( n, "rb" );
  if ( NULL != f )
  {
    fseek( f, 0, SEEK_END );
    size_t len = ftell( f );
    fseek( f, 0, SEEK_SET );
    ret = malloc( len + 1 );
    fread( ret, len, 1, f );
    ret[len] = '\0';
    fclose( f );
  }
  return ( ret );
}

int main( void )
{
  struct ctx ctx;

  // internal tests
  memset( &ctx, 0, sizeof( ctx ) );
  ctx.expected = fields;
  for ( ctx.j = ctx.r = 0; csv[ctx.j] != NULL; ctx.j++ )
  {
    char *h = strdup( csv[ctx.j] );
    ctx.i = 0;
    printf( "CASE #%d\n", ctx.j );
    int st = eccsv_parse( h, cb, &ctx );
    if ( st >= 0 )
      printf( "OK\n\n" );
    else
      printf( "FAIL %d\n\n", st );
    free( h );
  }

  // test files
  memset( &ctx, 0, sizeof( ctx ) );
  ctx.expected = testfiles;
  while ( NULL != testfiles[ctx.r] )
  {
    char *file = loadfile( testfiles[ctx.r] );
    char *h = file;
    if ( NULL != file )
    {
      int st = 1;
      printf( "TEST '%s':\n", testfiles[ctx.r] );
      ++ctx.r;
      while ( *h != '\0' )
      {
        printf( "  LINE %d '", ctx.j );
        print_escaped( h, strlen( h ), "'\n" );
        st = eccsv_parse( h, cb, &ctx );
        h += st;
        if ( st < 0 )
          break;
      }
      if ( st < 0 ) printf( "FAIL\n\n" );
      else          printf( "OK\n\n" );
      free( file );
    } else
      printf( "TEST '%s' MISSING!\n", testfiles[ctx.r] );
    while ( NULL != testfiles[ctx.r] ) ++ctx.r;
    ++ctx.r;
    ++ctx.j;
  }

  return ( 0 );
}
