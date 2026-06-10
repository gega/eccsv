# eccsv

A tiny single-header CSV parser for C.

`eccsv` is designed primarily for embedded systems and configuration files, but it can be used anywhere a lightweight CSV parser is needed.

## Features

* Single header library
* No heap allocation
* No external dependencies
* Callback-based API
* Supports quoted fields
* Supports escaped quotes (`""`)
* Supports embedded commas inside quoted fields
* Supports embedded newlines inside quoted fields
* Supports empty fields and trailing empty fields
* Supports LF and CRLF line endings
* Optional in-place unescaping for writable buffers
* Compile-time configuration

The parser aims to be RFC 4180 compatible for common CSV files while keeping code size and complexity small. It intentionally avoids features that are not typically useful for embedded applications, such as UTF-8 validation.

## Design Goals

* Small code size
* No dynamic memory allocation
* Easy integration into existing projects
* Suitable for ROM/flash-backed data
* Reasonable CSV interoperability
* Zero-copy parsing

## Integration

In one source file:

```c
#define ECCSV_IMPLEMENTATION
#include "eccsv.h"
```

In all other files:

```c
#include "eccsv.h"
```

## Basic Usage

```c
#include <stdio.h>

static void csv_cb(char *field, int col, size_t len, void *ctx)
{
    printf("column %d: %.*s\n", col, (int)len, field);
}

int main(void)
{
    char line[] = "foo,\"bar,baz\",123";

    eccsv_parse(line, csv_cb, NULL);

    return 0;
}
```

## API

```c
typedef void (*csv_cb_t)(
    eccsv_fieldp_t field,
    int col,
    size_t len,
    ECCSV_CTXTYPE ctx
);

int eccsv_parse(
    eccsv_fieldp_t s,
    csv_cb_t cb,
    ECCSV_CTXTYPE ctx
);
```

### Parameters

#### `s`

Pointer to a CSV record.

#### `cb`

Callback invoked once for each field.

#### `ctx`

User-defined context pointer passed unchanged to every callback invocation.

### Callback Arguments

#### `field`

Pointer to the field contents.

#### `col`

Zero-based column index.

#### `len`

Field length in bytes.

The field is not guaranteed to be NUL terminated. Use `len` when processing or copying data.

#### `ctx`

User context supplied to `eccsv_parse()`.

## Return Value

Returns the number of bytes consumed from the input buffer.

The returned length includes any line terminator (`\n`, `\r`, or `\r\n`) if present.

Negative values indicate errors:

| Value | Meaning                                |
| ----- | -------------------------------------- |
| -1    | Unterminated quoted field              |
| -2    | Invalid characters after closing quote |
| -3    | NULL input pointer                     |

## Configuration

### Field Separator

Default:

```c
#define ECCSV_SEP ','
```

Example:

```c
#define ECCSV_SEP ';'
#include "eccsv.h"
```

### Mutable Mode

Default:

```c
#define ECCSV_MUTABLE 1
```

When enabled, quoted fields are unescaped in-place.

Example:

```csv
"a""b"
```

becomes:

```text
a"b
```

The input buffer must therefore be writable.

### Immutable Mode

```c
#define ECCSV_MUTABLE 0
```

In immutable mode the parser never modifies the input buffer.

This allows parsing data stored in ROM, flash memory, or string literals.

Escaped quotes are not decoded in this mode. The callback receives a pointer into the original CSV data.

### Custom Context Type

Default:

```c
#define ECCSV_CTXTYPE void *
```

Example:

```c
struct parser_context;

#define ECCSV_CTXTYPE struct parser_context *
#include "eccsv.h"
```

This avoids casts inside callbacks.

## RFC Compatibility

The parser supports:

* Quoted fields
* Escaped quotes (`""`)
* Embedded commas in quoted fields
* Embedded newlines in quoted fields
* Empty fields
* Trailing empty fields
* CRLF and LF line endings

The focus is practical interoperability and small size rather than complete implementation of every edge case described by CSV-related standards.
