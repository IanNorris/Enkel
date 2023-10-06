#pragma once

struct TGAHeader
{
    uint8_t  idLength;        // ID length (usually 0)
    uint8_t  colorMapType;    // Color map type (usually 0)
    uint8_t  imageType;       // Image type (e.g., uncompressed RGB)
    uint16_t colorMapStart;   // Starting index of the color map
    uint16_t colorMapLength;  // Length of the color map
    uint8_t  colorMapDepth;   // Bits per pixel in the color map
    uint16_t xOffset;         // X-axis offset of the image
    uint16_t yOffset;         // Y-axis offset of the image
    uint16_t width;           // Image width
    uint16_t height;          // Image height
    uint8_t  pixelDepth;      // Bits per pixel in the image
    uint8_t  imageDescriptor; // Additional information about the image
} __attribute__((packed));
