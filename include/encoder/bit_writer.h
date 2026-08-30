#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct BitWriter
{
    uint8_t *items; // bytes
    size_t len, cap;
    uint8_t idx; // bit index within the byte
} BitWriter;

void bitWriterInit(BitWriter *writer);
void bitWriterWrite64(
        BitWriter *writer, 
        uint64_t data, 
        unsigned int bitCount
);
void bitWriterPrint(BitWriter *writer);


