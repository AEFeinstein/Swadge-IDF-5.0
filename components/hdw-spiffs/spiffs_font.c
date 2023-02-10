//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <string.h>

#include <esp_log.h>

#include "hdw-spiffs.h"
#include "spiffs_font.h"

//==============================================================================
// Functions
//==============================================================================

/**
 * @brief Load a font from ROM to RAM. Fonts are bitmapped image files that have
 * a single height, all ASCII characters, and a width for each character.
 * PNGs placed in the assets folder before compilation will be automatically
 * flashed to ROM
 *
 * @param name The name of the font to load. The ::font_t is not allocated by this function
 * @param font A handle to load the font to
 * @return true if the font was loaded successfully
 *         false if the font failed to load and should not be used
 */
bool loadFont(const char* name, font_t* font)
{
    // Read font from file
    uint8_t* buf  = NULL;
    size_t bufIdx = 0;
    uint8_t chIdx = 0;
    size_t sz;
    if (!spiffsReadFile(name, &buf, &sz, true))
    {
        ESP_LOGE("FONT", "Failed to read %s", name);
        return false;
    }

    // Read the data into a font struct
    font->h = buf[bufIdx++];

    // Read each char
    while (bufIdx < sz)
    {
        // Get an easy refence to this character
        font_ch_t* this = &font->chars[chIdx++];

        // Read the width
        this->w = buf[bufIdx++];

        // Figure out what size the char is
        int pixels = font->h * this->w;
        int bytes  = (pixels / 8) + ((pixels % 8 == 0) ? 0 : 1);

        // Allocate space for this char and copy it over
        this->bitmap = (uint8_t*)malloc(sizeof(uint8_t) * bytes);
        memcpy(this->bitmap, &buf[bufIdx], bytes);
        bufIdx += bytes;
    }

    // Zero out any unused chars
    while (chIdx <= '~' - ' ' + 1)
    {
        font->chars[chIdx].bitmap = NULL;
        font->chars[chIdx++].w    = 0;
    }

    // Free the SPIFFS data
    free(buf);

    return true;
}

/**
 * @brief Free the memory allocated for a font
 *
 * @param font The font handle to free memory from
 */
void freeFont(font_t* font)
{
    // using uint8_t instead of char because a char will overflow to -128 after the last char is freed (\x7f)
    for (uint8_t idx = 0; idx <= '~' - ' ' + 1; idx++)
    {
        if (font->chars[idx].bitmap != NULL)
        {
            free(font->chars[idx].bitmap);
        }
    }
}
