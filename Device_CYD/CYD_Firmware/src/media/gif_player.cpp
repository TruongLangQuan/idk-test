#include "gif_player.h"
#include "../display/ui_common.h"
#include "../hardware/sdcard.h"
#include <AnimatedGIF.h>
#include <SD.h>

static AnimatedGIF gif;
static File gifFile;
static bool gif_playing = false;

// Draw callback for AnimatedGIF
void GIFDraw(GIFDRAW *pDraw) {
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth > 320) iWidth = 320;

    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line

    s = pDraw->pPixels;
    if (pDraw->ucHasTransparency) {
        uint8_t ucTransparent = pDraw->ucTransparent;
        int iCount = 0;
        uint16_t *pEnd = usTemp + iWidth;
        d = usTemp;
        while (iCount < iWidth) {
            if (*s == ucTransparent) {
                *d = tft.readPixel(pDraw->iX + iCount, y);
            } else {
                *d = usPalette[*s];
            }
            s++; d++; iCount++;
        }
    } else {
        s = pDraw->pPixels;
        for (x = 0; x < iWidth; x++) {
            usTemp[x] = usPalette[*s++];
        }
    }
    tft.pushImage(pDraw->iX, y, iWidth, 1, usTemp);
}

void* GIFOpenFile(const char *fname, int32_t *pSize) {
    gifFile = SD.open(fname);
    if (gifFile) {
        *pSize = gifFile.size();
        return (void *)&gifFile;
    }
    return NULL;
}

void GIFCloseFile(void *pHandle) {
    File *f = static_cast<File *>(pHandle);
    if (f) f->close();
}

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    if ((pFile->iSize - pFile->iPos) < iLen)
        iBytesRead = pFile->iSize - pFile->iPos;
    if (iBytesRead <= 0) return 0;
    iBytesRead = f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
}

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPos) {
    File *f = static_cast<File *>(pFile->fHandle);
    f->seek(iPos);
    pFile->iPos = f->position();
    return pFile->iPos;
}

void gif_player_init() {
    gif.begin(LITTLE_ENDIAN_PIXELS);
    gif_playing = false;
    
    if (!sd_mounted()) {
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.drawCentreString("SD Card Not Found", 160, 110, 2);
        return;
    }

    // Try to find a GIF in /media
    std::vector<String> files;
    sd_list_dir("/media", files);
    
    String target = "";
    for (const auto& f : files) {
        if (f.endsWith(".gif") || f.endsWith(".GIF")) {
            target = "/media/" + f;
            break;
        }
    }

    if (target != "") {
        if (gif.open(target.c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
            gif_playing = true;
            tft.fillScreen(COLOR_BG);
        }
    } else {
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.drawCentreString("No GIFs in /media", 160, 110, 2);
    }
}

void gif_player_update() {
    if (gif_playing) {
        int iDelay = 0;
        if (!gif.playFrame(true, &iDelay)) {
            // Loop GIF
            gif.reset();
        }
    }
}