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

#if __STDC_VERSION__ < 201112L
#error C11 support is mandatory for serializer
#endif

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
    TOKEN_LE64BIT,
    TOKEN_BE64BIT,
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

        case '8':

            switch (*endianness)
            {
                case LITTLE_ENDIAN:
                    return TOKEN_LE64BIT;

                case BIG_ENDIAN:
                    return TOKEN_BE64BIT;

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

static void read8(void *const dst, const void *const src, const size_t sz)
{
    *(uint8_t *)dst = *(const uint8_t *)src;
}

static void swap64(uint8_t *dst, const uint8_t *const src)
{
    *dst++ = *(src + 7);
    *dst++ = *(src + 6);
    *dst++ = *(src + 5);
    *dst++ = *(src + 4);
    *dst++ = *(src + 3);
    *dst++ = *(src + 2);
    *dst++ = *(src + 1);
    *dst = *src;
}

static void readbe64(void *const dst, const void *const src, const size_t sz)
{
    if (little_endian())
    {
        swap64(dst, src);
    }
    else
    {
        memmove(dst, src, sz);
    }
}

static void readle64(void *const dst, const void *const src, const size_t sz)
{
    if (little_endian())
    {
        memmove(dst, src, sz);
    }
    else
    {
        swap64(dst, src);
    }
}

static void swap32(uint8_t *dst, const uint8_t *const src)
{
    *dst++ = *(src + 3);
    *dst++ = *(src + 2);
    *dst++ = *(src + 1);
    *dst = *src;
}

static void readbe32(void *const dst, const void *const src, const size_t sz)
{
    if (little_endian())
    {
        swap32(dst, src);
    }
    else
    {
        memmove(dst, src, sz);
    }
}

static void readle32(void *const dst, const void *const src, const size_t sz)
{
    if (little_endian())
    {
        memmove(dst, src, sz);
    }
    else
    {
        swap32(dst, src);
    }
}

static void swap16(uint8_t *dst, const uint8_t *const src)
{
    *dst++ = *(src + 1);
    *dst = *src;
}

static void readbe16(void *const dst, const void *const src, const size_t sz)
{
    if (little_endian())
    {
        swap16(dst, src);
    }
    else
    {
        memmove(dst, src, sz);
    }
}

static void readle16(void *const dst, const void *const src, const size_t sz)
{
    if (little_endian())
    {
        memmove(dst, src, sz);
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

        while ((c = *format))
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
                        [TOKEN_BE32BIT] = sizeof (uint32_t),
                        [TOKEN_LE64BIT] = sizeof (uint64_t),
                        [TOKEN_BE64BIT] = sizeof (uint64_t)
                    };

                    static const size_t aligns[] =
                    {
                        [TOKEN_8BIT] = _Alignof (uint8_t),
                        [TOKEN_LE16BIT] = _Alignof (uint16_t),
                        [TOKEN_BE16BIT] = _Alignof (uint16_t),
                        [TOKEN_LE32BIT] = _Alignof (uint32_t),
                        [TOKEN_BE32BIT] = _Alignof (uint32_t),
                        [TOKEN_LE64BIT] = _Alignof (uint64_t),
                        [TOKEN_BE64BIT] = _Alignof (uint64_t)
                    };

                    static void (*const read[])(void *dst, const void *src, size_t sz) =
                    {
                        [TOKEN_8BIT] = read8,
                        [TOKEN_LE16BIT] = readle16,
                        [TOKEN_BE16BIT] = readbe16,
                        [TOKEN_LE32BIT] = readle32,
                        [TOKEN_BE32BIT] = readbe32,
                        [TOKEN_LE64BIT] = readle64,
                        [TOKEN_BE64BIT] = readbe64
                    };

                    const size_t st = sizes[token];
                    const size_t padding = (uintptr_t)((const uint8_t *)dst + out_sz) % aligns[token];

                    if (padding)
                    {
                        out_sz += st - padding;
                    }

                    if (in_sz + st > src_sz)
                        return SERIALIZER_ERR_IN_OVERFLOW;
                    else if (out_sz + st <= sz)
                    {
                        read[token]((uint8_t *)dst + out_sz, (const uint8_t *)src + in_sz, st);
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
