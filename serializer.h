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

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Tokens used by this library:
 * '1': 8-bit value.
 * 'leN': little endian N-byte value.
 * 'beN': big endian N-byte value.
 * Where N: 2 or 4.
 * Tokens must be placed without spaces or other symbols.
 * For example, "11le2be41" means:
 *  - 2 8-bit values.
 *  - 1 little-endian 16-bit value.
 *  - 1 big-endian 32-bit value.
 *  - 1 8-bit value.
 * For increased readability, optional slashes can be placed
 * between tokens. For example: "1/1/le2/be4/1". */

/**
 * Error code list used by this library.
 */
enum serializer_err
{
   SERIALIZER_OK, /**< Serialize/deserialize operation was successful. */
   SERIALIZER_ERR_INVALID_ARG, /**< An invalid argument has been given. */
   SERIALIZER_ERR_FORMAT, /**< An error occured while parsing format string. */
   SERIALIZER_ERR_IN_OVERFLOW, /**< Input data holds less bytes than specified by format. */
   SERIALIZER_ERR_OUT_OVERFLOW /**< Input data does not fit into destination buffer. */
};

/**
 * Dumps source buffer with given format and alignment into destination buffer.
 * @param format Data structure format as null-terminated string.
 * @param dst Destination buffer.
 * @param sz Size of the destination buffer in bytes.
 * @param src Source buffer.
 * @param src_sz Size of the source buffer in bytes.
 * @return Returns one of the error codes from @ref serializer_err .
 * @see Possible tokens for the format string on the comments above.
 * @attention Buffer contents are undefined if an error occurs.
 */
enum serializer_err serialize(const char *format, void *dst, size_t sz, const void *src, size_t src_sz);

/**
 * Dumps source buffer with given format into destination buffer with given alignment.
 * @param format Data structure format as null-terminated string.
 * @param dst Destination buffer.
 * @param sz Size of the destination buffer in bytes.
 * @param src Source buffer.
 * @param src_sz Size of the source buffer in bytes.
 * @return Returns one of the error codes from @ref serializer_err .
 * @see Possible tokens for the format string on the comments above.
 * @attention Buffer contents are undefined if an error occurs.
 */
enum serializer_err deserialize(const char *format, void *dst, size_t sz, const void *src, size_t src_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SERIALIZER_H */
