#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "encoder/bit_writer.h"
#include "sutils/darray.h"

void bitWriterInit(BitWriter *writer)
{
    writer->idx = 0;
    writer->len = 0;
    writer->cap = 64;
    writer->items = malloc(writer->cap);
    memset(writer->items, 0, writer->cap);
}

void bitWriterWrite64(
        BitWriter *writer, 
        uint64_t data, 
        unsigned int bitCount
) {
    assert(bitCount <= 64);

    while (bitCount > 0)
    {
        if (writer->idx == 0)
        {
            da_push(writer, 0);
        }

        unsigned int written = 8 - writer->idx; 
        if (written > bitCount)
        {
            written = bitCount;
        }

        for (int i = 0; i < written; i++)
        {
            unsigned int shift = bitCount - 1 - i;
            uint8_t bit = (data >> shift) & 1;

            writer->items[writer->len - 1] |= bit << (writer->idx + i);
        }

        bitCount -= written;
        writer->idx += written;

        if (writer->idx == 8)
            writer->idx = 0;   
    }
}

static void printByte(uint8_t byte)
{
    for (int i = 0; i < 8; i++)
    {
        uint8_t bit = (byte >> i) & 1;
        printf("%d", bit);
    }
}

void bitWriterPrint(BitWriter *writer)
{
    for (size_t i = 0; i < writer->len; i++)
    {
        printByte(writer->items[i]);
    }
}


