/*
   Copyright 2020 Xavier Del Campo Romero

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "serializer.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

static bool little_endian(void)
{
    return (const union {char c; int b;}){.c = 1}.b;
}

enum serializer_err serialize(const char *format, void *dst, size_t sz, const void *src, const size_t src_sz)
{
    if (!format || !dst || !sz || !src)
    {
        return SERIALIZER_ERR_INVALID_ARG;
    }

    return SERIALIZER_OK;
}

enum token
{
    TOKEN_8BIT,
    TOKEN_LE16BIT,
    TOKEN_BE16BIT,
    TOKEN_LE32BIT,
    TOKEN_BE32BIT,
    TOKEN_NOT_READY,
    TOKEN_ERROR
};

enum state
{
    ENDIAN_SPECIFIER_OR_8BIT_OR_SLASH,
    CHARACTER_E,
    MULTI_BYTE_NUM
};

enum endianness
{
    ENDIANESS_UNKNOWN,
    LITTLE_ENDIAN,
    BIG_ENDIAN
};

static enum token get_spec_8bit(const char c, enum state *const state, enum endianness *const endianness)
{
    switch (c)
    {
        case '/':
            return TOKEN_NOT_READY;

        case '1':
            return TOKEN_8BIT;

        case 'l':
            *state = CHARACTER_E;
            *endianness = LITTLE_ENDIAN;
            return TOKEN_NOT_READY;

        case 'b':
            *state = CHARACTER_E;
            *endianness = BIG_ENDIAN;
            return TOKEN_NOT_READY;

        default:
            break;
    }

    return TOKEN_ERROR;
}

static enum token get_ech(const char c, enum state *const state, enum endianness *const endianness)
{
    if (c == 'e')
    {
        *state = MULTI_BYTE_NUM;
        return TOKEN_NOT_READY;
    }

    return TOKEN_ERROR;
}

static enum token get_multibyte(const char c, enum state *const state, enum endianness *const endianness)
{
    switch (c)
    {
        case '2':

            switch (*endianness)
            {
                case LITTLE_ENDIAN:
                    return TOKEN_LE16BIT;

                case BIG_ENDIAN:
                    return TOKEN_BE16BIT;

                default:
                    break;
            }

            break;

        case '4':

            switch (*endianness)
            {
                case LITTLE_ENDIAN:
                    return TOKEN_LE32BIT;

                case BIG_ENDIAN:
                    return TOKEN_BE32BIT;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    return TOKEN_ERROR;
}

static enum token get_token(const char c, enum state *const state, enum endianness *const endianness)
{
    static enum token (*const get[])(char c, enum state *state, enum endianness *endianness) =
    {
        [ENDIAN_SPECIFIER_OR_8BIT_OR_SLASH] = get_spec_8bit,
        [CHARACTER_E] = get_ech,
        [MULTI_BYTE_NUM] = get_multibyte
    };

    return get[*state](c, state, endianness);
}

static void read8(void *const dst, const void *const src)
{
    *(uint8_t *)dst = *(const uint8_t *)src;
}

static size_t swap32(void *dst, const void *const src)
{
    *(uint8_t *)dst++ = *((const uint8_t *)src + 3);
    *(uint8_t *)dst++ = *((const uint8_t *)src + 2);
    *(uint8_t *)dst++ = *((const uint8_t *)src + 1);
    *(uint8_t *)dst = *(const uint8_t *)src;
}

static void readbe32(void *const dst, const void *const src)
{
    enum
    {
        SZ = sizeof (uint32_t)
    };

    if (little_endian())
    {
        swap32(dst, src);
    }
    else
    {
        memmove(dst, src, SZ);
    }
}

static void readle32(void *const dst, const void *const src)
{
    enum
    {
        SZ = sizeof (uint32_t)
    };

    if (little_endian())
    {
        memmove(dst, src, SZ);
    }
    else
    {
        swap32(dst, src);
    }
}

static size_t swap16(void *dst, const void *const src)
{
    *(uint8_t *)dst++ = *((const uint8_t *)src + 1);
    *(uint8_t *)dst = *(const uint8_t *)src;
}

static void readbe16(void *const dst, const void *const src)
{
    enum
    {
        SZ = sizeof (uint16_t)
    };

    if (little_endian())
    {
        swap16(dst, src);
    }
    else
    {
        memmove(dst, src, SZ);
    }
}

static void readle16(void *const dst, const void *const src)
{
    enum
    {
        SZ = sizeof (uint16_t)
    };

    if (little_endian())
    {
        memmove(dst, src, SZ);
    }
    else
    {
        swap16(dst, src);
    }
}

enum serializer_err deserialize(const char *format,
                                void *const dst,
                                const size_t sz,
                                const void *const src,
                                const size_t src_sz)
{
    if (!format || !dst || !sz || !src)
    {
        return SERIALIZER_ERR_INVALID_ARG;
    }
    else
    {
        enum state state = 0;
        enum endianness endianness = ENDIANESS_UNKNOWN;
        char c;
        size_t in_sz = 0, out_sz = 0;

        while (c = *format)
        {
            const enum token token = get_token(c, &state, &endianness);

            switch (token)
            {
                default:
                {
                    static const size_t sizes[] =
                    {
                        [TOKEN_8BIT] = sizeof (uint8_t),
                        [TOKEN_LE16BIT] = sizeof (uint16_t),
                        [TOKEN_BE16BIT] = sizeof (uint16_t),
                        [TOKEN_LE32BIT] = sizeof (uint32_t),
                        [TOKEN_BE32BIT] = sizeof (uint32_t)
                    };

                    const size_t st = sizes[token];

                    static void (*const read[])(void *dst, const void *src) =
                    {
                        [TOKEN_8BIT] = read8,
                        [TOKEN_LE16BIT] = readle16,
                        [TOKEN_BE16BIT] = readbe16,
                        [TOKEN_LE32BIT] = readle32,
                        [TOKEN_BE32BIT] = readbe32
                    };

                    const size_t padding = out_sz % st;

                    if (padding)
                    {
                        out_sz += st - padding;
                    }

                    if (in_sz + st > src_sz)
                        return SERIALIZER_ERR_IN_OVERFLOW;
                    else if (out_sz + st <= sz)
                    {
                        read[token]((uint8_t *)dst + out_sz, (const uint8_t *)src + in_sz);
                        in_sz += st;
                        out_sz += st;
                        state = 0;
                    }
                    else
                        return SERIALIZER_ERR_OUT_OVERFLOW;
                }
                    break;

                case TOKEN_ERROR:
                    return SERIALIZER_ERR_FORMAT;

                case TOKEN_NOT_READY:
                    break;
            }

            format++;
        }

        if (!out_sz)
        {
            return SERIALIZER_ERR_FORMAT;
        }
    }

    return SERIALIZER_OK;
}
