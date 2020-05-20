Serializer/deserializer library for C11
=======================================

This library hopes to be useful under certain circumstances where a portable
implementation is desired to read/write data streams yet endianness and
alignment requirements need to be taken into account e.g.: network protocols,
file formats, etc. As the title specifies, only a C11-compliant compiler is
required.

Only 8-bit and 16/32/64-bit little-endian and big-endian values are supported.
This means no support for bit fields, as some aspects of bit fields are
implementation-defined.

Usage
------

```{c}
#include <serializer.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
    struct
    {
        uint8_t a;
        uint16_t b;
        uint32_t c;
        uint16_t d;
        uint64_t e;
        uint64_t f;
    } ex;

    static const uint8_t data[] =
    {
        0x01, 0x33, 0xFF, 0xAC, 0xBB, 0xFA, 0xFA, 0xDE, 0xDD,
        0x12, 0x33, 0xFF, 0xAC, 0xBB, 0xFA, 0xFA, 0xDE,
        0x12, 0x33, 0xFF, 0xAC, 0xBB, 0xFA, 0xFA, 0xDE,
    };

    deserialize("1/le2/be4/be2/be8/le8", &ex, sizeof ex, data, sizeof data);
    printf("a=%X, b=%X, c=%X, d=%X, e=%lX, f=%lX\n",
            ex.a, ex.b, ex.c, ex.d, ex.e, ex.f);
    return 0;
}
```
Output
------
`a=1, b=FF33, c=ACBBFAFA, d=DEDD, e=1233FFACBBFAFADE, f=DEFAFABBACFF3312`

TODO
----
Only deserialization is implemented. Serialization will be implemented in the future.
