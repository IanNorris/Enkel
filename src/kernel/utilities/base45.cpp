/*
Copyright 2021 kf03w5t5741l (wallabythree)

Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in 
 the Software without restriction, including without limitation the rights to 
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
 of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/
// Modifications move to remove dynamic allocation and pow.

#include <stddef.h>

#include "kernel/utilities/base45.h"

static const char charset[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
    'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    ' ', '$', '%', '*', '+', '-', '.', '/', ':'
};

static unsigned int powInt(unsigned int base, unsigned int exp)
{
	unsigned int result = 1;
	
	for(unsigned int index = 0; index < exp; index++)
	{
		result *= base;
	}

	return result;
}

/**
 * @brief Encode a set of raw bytes in Base45
 * 
 * @param dst Destination char array
 * @param src Source char array
 * @param size Bytes to encode
 * @param written Pointer to bytes written to destination
 * @return int BASE45_STATUS
 */
static int encode(char* dst, const char* src, int size, size_t* written)
{
    *written = 0;

    unsigned int sum = 0;
    for (int i = 0; i < size; i++) {
        unsigned char c = src[size - i - 1];
        sum += (unsigned int) (c * powInt(256, i));
    }

    int i = 0;
    while (sum > 0) {
        dst[i] = charset[sum % 45];
        sum /= 45;
        (*written)++;
        i++;
    }

    return BASE45_OK;
}

int Base45_encode(char* dest, size_t destLength, const char* input, size_t length)
{
	size_t requiredLength = length / 2 * 3 + length % 2 * 2 + 1;

    if (destLength < requiredLength) {
        return -BASE45_MEMORY_ERROR;
    }

    size_t i = 0;
    size_t offset = 0;

    while (i < length) {
        size_t size;

        // consume two raw bytes at a time, except if only one is left
        if (length - i > 1) {
            size = 2;
        } else {
            size = 1;
        }

        size_t written = 0;

        int encode_status = encode(
            dest + offset,
            input + i,
            size,
            &written
        );

        if (encode_status != BASE45_OK) {
            dest = NULL;
            return -encode_status;
        }

        // add leading zeroes if required
        while (written < size + 1) {
            char* zero = dest + offset + written;
            *zero = charset[0];
            written++;
        }
        offset += written;

        i += size;
    }

    return requiredLength-1;
}

