#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "Jura.h"
#include "Jura_0.h"

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
	
	char* imageDataWithoutHeader = Jura_0_tga_data +  18;
	uint32_t* imageDataWithoutHeaderU32 = (uint32_t*)imageDataWithoutHeader;

	for(int page = 0; page < 4; page++)
	{
		uint32_t mask = 0xFF << (page * 8);
		// Note: we assume the framebuffer model is RGB with 32-bit pixels.
		for (size_t i = 0; i < 256; i++) {
			for (size_t j = 0; j < 256; j++) {
				uint32_t *fb_ptr = framebuffer->address;
				uint32_t glyph_col = (imageDataWithoutHeaderU32[(j * 256) + i] & mask) >> (page * 8);
				glyph_col |= glyph_col << 8 | glyph_col;
				glyph_col |= glyph_col << 16 | glyph_col;
				glyph_col |= glyph_col << 24 | glyph_col;
				fb_ptr[j * (framebuffer->pitch / 4) + i + (page * 256)] = glyph_col;
			}
		}
	}

    // We're done, just hang...
    hcf();
}
