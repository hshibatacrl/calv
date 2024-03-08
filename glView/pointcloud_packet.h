#ifndef POINTCLOUD_PACKET_H
#define POINTCLOUD_PACKET_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdint>

#define PC_MAGIC 0xC31144AA
// bits for pc_packet_header_t, type
#define PC_ORG (0x0100) // optional, pc_origin_t, use zeros if not provided
#define PC_TXT (0x0200) // optional, pc_text_t

#pragma pack(push,1)

typedef struct
{
    uint32_t magic;
    uint32_t length;    // all packet length
    uint32_t type;
    int64_t frameId;
    int64_t time;
    uint16_t data[6];
} pc_packet_header_t;   // 40bytes

typedef struct
{
    double xyz[3];
} pc_origin_t;         // 24 bytes

typedef struct
{
    uint16_t length;
} pc_text_t;         // 2 + length bytes

typedef struct
{
    uint32_t nPoints;
    uint32_t format;
} pc_payload_t;        // 4 + nPoints x

//bits for pc_payload_t format
#define PC_XYZ (0x0001) // required 4 x float
#define PC_RGB (0x0002) // optional 3 x float
#define PC_AMP (0x0004) // optional 1 x float
#define PC_RNG (0x0008) // optional 1 x float

#pragma pack(pop)

#endif // POINTCLOUD_PACKET_H
