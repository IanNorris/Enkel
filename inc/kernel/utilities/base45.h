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

#ifndef BASE45_H
#define BASE45_H

enum BASE45_STATUS {
    BASE45_OK,
    BASE45_INVALID_LENGTH,
    BASE45_INVALID_CHAR,
    BASE45_INVALID_VALUE,
    BASE45_MEMORY_ERROR
};

/**
 * @brief Encode data as Base45
 * 
 * @param dest destination buffer of encoded output
 * @param destLength The destination buffer size in bytes
 * @param input pointer to a char array with raw bytes
 * @param length input length
 * @return BASE45_STATUS
 */
int Base45_encode(char* dest, size_t destLength, const char* input, size_t length);

#endif