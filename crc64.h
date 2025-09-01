//
// Copyright 2024 Hans-Juergen Lange <hjl@simulated-universe.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in the
// Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#ifndef CRC64_H
#define CRC64_H


#include <cstdint>
#include <cstdlib>
#include <string>

class Crc64 {
public:
    Crc64() = default;
    bool operator==(Crc64 const& c2) { return crc == c2.crc; }
    bool operator!=(Crc64 const& c2) { return crc != c2.crc; }
    uint64_t calc(void const* block, size_t len);
    uint64_t calc(const std::string& text) { return (calc(text.c_str(), text.size())); }
private:
    void compute(void const* data, uint32_t len) {
        unsigned char* cdata = (unsigned char *) data;

        while (len-- > 0) {
          int tab_index = ((int) (crc >> 56) ^ *cdata++) & 0xFF;
          crc = crc64_table[tab_index] ^ (crc << 8);
      }
    }
    void init() { crc = 0xffffffffffffffff; }
    void fin() { crc  ^= 0xffffffffffffffff; }
private:
    static uint64_t crc64_table[256];
    uint64_t crc;
};

#endif /* CRC64_H */
