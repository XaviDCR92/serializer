Serializer/deserializer library for C99
=======================================

This library hopes to be useful under certain circumstances where a portable
implementation is desired to read/write data streams yet endianness and
alignment requirements need to be taken into account e.g.: network protocols,
file formats, etc. As the title specifies, only a C99-compliant compiler is
required.

Only 8-bit and 16/32-bit little-endian and big-endian values are supported.
This means no support for bit fields, as some aspects of bit fields are
implementation-defined.

Usage
------

```{c}
#include "serializer.h"
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
    } ex;

    static const uint8_t data[] =
    {
        0x01, 0x33, 0xFF, 0xAC, 0xBB, 0xFA, 0xFA, 0xDE, 0xDE
    };

    deserialize("1/le2/be4/be2", &ex, sizeof ex, data, sizeof data);
    printf("a=%X, b=%X, c=%X, d=%X\n",
            ex.a, ex.b, ex.c, ex.d);
    return 0;
}
```
Output
------
`a=1, b=FF33, c=ACBBFAFA, d=DEDE`

TODO
----
Only deserialization is implemented. Serialization will be implemented in the future.

64-bit support.
