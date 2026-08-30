#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "sutils/print.h"

int main(int argc, char *argv[])
{
    int width, height, channels;
    unsigned char *pixels = stbi_load("qr-code.png", &width, &height, &channels, 1);
    if (pixels == NULL)
    {
        eprintfln("Failed to read the given file");
        exit(EXIT_FAILURE);
    }

    printfln("width: %d; height: %d", width, height);
   for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (pixels[x + y * width] > 128)
            {
                printf("@@");
            } else {
                printf("  ");
            }
        }
        printfln();
    }

    stbi_image_free(pixels);
    return 0;
}

