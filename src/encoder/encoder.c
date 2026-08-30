#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "encoder/bit_writer.h"

#include "sutils/darray.h"
#include "sutils/print.h"
#include "sutils/arrlen.h"

// Different sizes of QR codes are called **versions**
// There are 40 versions available and the smallest is 21x21
// the biggest is 177x177. Each version is 4 pixels larger
// than the previous version.

// QR Codes use Reed-Solomon error correction

typedef enum ErrCorrLevel
{
    ErrCorrLevel_L, // 7%
    ErrCorrLevel_M, // 15%
    ErrCorrLevel_Q, // 25%
    ErrCorrLevel_H,  // 30%
    ErrCorrLevelCount
} ErrCorrLevel;

typedef enum Modes
{
    MODES_NUMERIC,
    MODES_ALPHANUMERIC,
    MODES_BYTE,
    MODES_KANJI,
    MODES_COUNT
} Modes;


// Full table: https://www.thonky.com/qr-code-tutorial/character-capacities
// I only consider alphanumeric mode
int char_cap_by_vers_n_mode[][4] = {
    {25, 20, 16, 10},
    {47, 38, 29, 20},
    {77, 61, 47, 25}
};

int num_data_bits_by_vers_n_mode[][4] = {
    {19, 16, 13, 9}
};

int find_smallest_version(size_t len, ErrCorrLevel lvl)
{
    for (int i = 0; i < arrlen(char_cap_by_vers_n_mode); i++)
    {
        if (len <= char_cap_by_vers_n_mode[i][lvl])
            return i;
    }
    return -1;
}

// Maximal capacity for 40-L (version 40 with error correction level L)
// is 4296 alphanumeric characters

// Is placed at the beginning, after it must be the character count
// indicator. The length of the character count indicatoGr depends on
// the encoding mode and the QR code verison and sometimes you're
// required to pad it on the left with 0's
unsigned int mode_indicator[MODES_COUNT] = {
    0b0001, // Numeric Mode
    0b0010, // Alphanumeric Mode
    0b0100, // Byte Mode
    0b1000, // Kanji Mode
};

// The length is in bits!
// TODO: Addd support for other guys
int character_count_indicator_length(int version, Modes mode)
{
    if (version >= 1 && version <= 9)
    {
        switch (mode)
        {
        case MODES_NUMERIC:
            return 10;
        case MODES_ALPHANUMERIC:
            return 9;
        }
    } else if (version <= 26)
    {
        switch (mode)
        {
        case MODES_NUMERIC:
            return 12;
        case MODES_ALPHANUMERIC:
            return 11;
        }

    } else if (version <= 40)
    {
        switch (mode)
        {
        case MODES_NUMERIC:
            return 14;
        case MODES_ALPHANUMERIC:
            return 13;
        }
    } else {
        return -1;
    }
}

// Alphanumeric table: 
// https://www.thonky.com/qr-code-tutorial/alphanumeric-table
int alphanumToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;

    switch (c)
    {
    case ' ': return 36;
    case '$': return 37;
    case '%': return 38;
    case '*': return 39;
    case '+': return 40;
    case '-': return 41;
    case '.': return 42;
    case '/': return 43;
    case ':': return 44;
    default:  return -1;
    }
}

ErrCorrLevel parse_ecl_arg(char lvl)
{
    switch (lvl)
    {
    case 'L':
    {
        return ErrCorrLevel_L;
    }
    case 'M':
    {
        return ErrCorrLevel_M;
    }
    case 'Q':
    {
        return ErrCorrLevel_Q;
    }
    case 'H':
    {
        return ErrCorrLevel_H;
    }
    default:
        return ErrCorrLevelCount;
    }
}

void encodeAlphanumeric(BitWriter *writer, const char *src, size_t len)
{
    size_t i = 0;
    for (; i + 1 < len; i += 2)
    {
        int c1 = alphanumToInt(src[i]);
        int c2 = alphanumToInt(src[i + 1]);
        int r = c1 * 45 + c2;
        bitWriterWrite64(writer, r, 11);
    }

    if (i < len)
    {
        int c = alphanumToInt(src[i]);
        bitWriterWrite64(writer, c, 6);
    }
}

void encodeTermination(BitWriter *writer, int numCurrBits, int numDataBits)
{
    int d = numDataBits - numCurrBits;
    int written = d >= 4 ? 4 : d;
    bitWriterWrite64(writer, 0, written);
}

void encodeAlignment(BitWriter *writer)
{
    int written = 8 - writer->idx;
    if (written == 8)
        return;
    bitWriterWrite64(writer, 0, written);
}

const uint8_t paddings[] = { 0xEC, 0x11 };

void encodePadding(BitWriter *writer, int numDataBits)
{
    int numCurrBits = writer->len * 8 + writer->idx;
    
    int padIdx = 0;
    while (numCurrBits != numDataBits)
    {
        int remaining = numDataBits - numCurrBits;
        int written = remaining >= 8 ? 8 : remaining;
        bitWriterWrite64(writer, paddings[padIdx], written);
        numCurrBits += written;
        padIdx ^= 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        eprintfln("Not enough arguments were supplied!");
        printfln("Usage: ./encoder L/M/Q/H \"{text}\"");
        exit(EXIT_FAILURE);
    }
    
    ErrCorrLevel ecl = parse_ecl_arg(*argv[1]);
    if (ecl == ErrCorrLevelCount)
    {
        eprintfln("Invalid argument for error correction level!");
        printfln("Available error correction levels are: L; M; Q; H");
        exit(EXIT_FAILURE);
    }

    const char *src = argv[2];
    size_t srcLen = strlen(src);
    if (srcLen == 0)
    {
        eprintfln("Invalid size of the data given");
        printfln("Please pass a non-empty text to encode");
        exit(EXIT_FAILURE);
    }

    int version = find_smallest_version(srcLen, ErrCorrLevel_Q) + 1;
    int charCountBitsLen = character_count_indicator_length(version, MODES_ALPHANUMERIC);
    long long mask = (1 << charCountBitsLen) - 1;
    int maskedLen = srcLen & mask;
    
    BitWriter writer;
    bitWriterInit(&writer);
    bitWriterWrite64(&writer, mode_indicator[MODES_ALPHANUMERIC], 4);
    bitWriterWrite64(&writer, srcLen, charCountBitsLen);
    encodeAlphanumeric(&writer, src, srcLen);
    int numCurrBits = writer.len * 8 + writer.idx - 1;
    int numDataBits = num_data_bits_by_vers_n_mode[version - 1][ecl] * 8;
    encodeTermination(&writer, numCurrBits, numDataBits);
    encodeAlignment(&writer);
    encodePadding(&writer, numDataBits);
    
    bitWriterPrint(&writer);

    exit(EXIT_SUCCESS);
}
