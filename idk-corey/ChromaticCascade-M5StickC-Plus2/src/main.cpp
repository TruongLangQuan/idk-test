/*
M5StickC Plus2 Audio Spectrum Display, Oscilloscope, and Tuner
Converted from M5StickC Plus1 original by KIRA Ryouta

Features:
- Audio Spectrum Display 
- Oscilloscope
- Tuner
- Button A to switch modes

Copyright 2020 KIRA Ryouta
Ported to M5StickC Plus2 2024

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma GCC optimize ("O3")
#include <M5StickCPlus2.h>
#include <driver/i2s.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "dywapitchtrack.h"
#include "fix_fft.h"

// M5StickC Plus2 has same I2S mic pins as Plus1
#define PIN_CLK  0
#define PIN_DATA 34
#define SAMPLES 1024 // Must be a power of 2
#define READ_LEN (2 * SAMPLES)

// M5StickC Plus2 display: Use full screen properly
#define TFT_WIDTH 240
#define TFT_HEIGHT 135
#define BANDS 8
#define BANDS_WIDTH ( TFT_WIDTH / BANDS )
#define BANDS_PADDING 2
#define BAR_WIDTH ( BANDS_WIDTH - BANDS_PADDING )
#define NOISE_FLOOR 1
#define AMPLIFIER (1 << 2)
#define MAGNIFY 3
#define RSHIFT 13
#define RSHIFT2 1
#define OSC_NOISEFLOOR 100
#define OSC_SAMPLES DYWAPT_SAMPLESIZE
#define OSC_SKIPCOUNT (OSC_SAMPLES/SAMPLES)
#define OSC_EXTRASKIP 0

#define MAXBUFSIZE OSC_SAMPLES

struct eqBand {
  const char *freqname;
  int peak;
  int lastpeak;
  uint16_t lastval;
};

enum : uint8_t {
  ModeSpectrumBars,
  ModeOscilloscope,
  ModeTuner,
  ModeAudioCircles,
  ModeAudioPlasma,
  ModeAudioFireworks,
  ModeAudioWaves,
  ModeAudioSpiral,
  ModeWaterRipples,
  ModeAudioPlasmaField,
  ModeAudioStarField,
  ModeFastPlasma,
  ModeAudioRain,
  ModeExplodingFireworks,
  ModeLightningStorm,
  ModeMatrixRain,
  ModeSnowEffect,
  ModeLavaLamp,
  ModeAuroraBorealis,
  ModeKaleidoscope,
  ModeParticleTornado,
  ModeSoundVisualizerBars,
  ModeCount,
};

static uint8_t runmode = 0;

static volatile bool semaphore = false;
static volatile bool needinit = true;
static bool showText = true; // Global text display toggle

static eqBand audiospectrum[BANDS] = {
  // freqname,peak,lastpeak,lastval,
  { ".1k", 0 },
  { ".2k", 0 },
  { ".5k", 0 },
  { " 1k", 0 },
  { " 2k", 0 },
  { " 4k", 0 },
  { " 8k", 0 },
  { "16k", 0 }
};
 
static int vTemp[2][MAXBUFSIZE];
static uint8_t curbuf = 0;
static uint16_t colormap[TFT_HEIGHT];//color palette for the band meter(pre-fill in setup)

static dywapitchtracker pitchTracker;
static LGFX_Sprite sprite(&M5.Lcd);

static int bufposcount = 0;

static const char *notestr[12] = {
  "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "
};

static uint16_t oscbuf[2][OSC_SAMPLES];
#if OSC_EXTRASKIP > 0
static int skipcount = 0;
#endif

// Function declarations
void looptask(void *);
void initMode();
void showSpectrumBars();
void displayBand(int band, int dsize);
byte getBand(int i);
float calcNumSamples(float f);
void showFreq(float freq);
void showOscilloscope();
void showTuner();
void showAudioCircles();
void showAudioPlasma();
void showAudioFireworks();
void showAudioWaves();
void showAudioSpiral();
void showWaterRipples();
void showAudioPlasmaField();
void showAudioStarField();
void showFastPlasma();
void showAudioRain();
void showExplodingFireworks();
void showLightningStorm();
void showMatrixRain();
void showSnowEffect();
void showLavaLamp();
void showAuroraBorealis();
void showKaleidoscope();
void showParticleTornado();
void showSoundVisualizerBars();
uint16_t getColorFromHSV(float h, float s, float v);
void calculateAudioBands(int bands[8]);

void i2sInit(){
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  DYWAPT_SAMPLERATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, //is fixed at 12bit,stereo,MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };
   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA; 
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, DYWAPT_SAMPLERATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}
 
void setup() {
  M5.begin();
  setCpuFrequencyMhz(80);
  M5.Lcd.setRotation(1); // Proper landscape mode for Plus2 full screen
  M5.Lcd.fillScreen(BLACK);

  sprite.createSprite(TFT_WIDTH, TFT_HEIGHT);
  sprite.setTextSize(1);

  i2sInit();
 
  for(uint8_t i=0;i<TFT_HEIGHT;i++) {
    float r = TFT_HEIGHT - i;
    float g = i;
    float mag = (r > g)? 255./r : 255./g;
    r *= mag;
    g *= mag;
    colormap[i] = M5.Lcd.color565((uint8_t)r,(uint8_t)g,0);
  }

  int core = 1 - xPortGetCoreID();
  xTaskCreatePinnedToCore(looptask,"calctask",32768,NULL,1,NULL,core);
}

void initMode() {
  M5.Lcd.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);
  switch (runmode) {
    case ModeSpectrumBars:
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(LIGHTGREY);
      for (byte band = 0; band < BANDS; band++) {
        M5.Lcd.setCursor(BANDS_WIDTH*band + 2, 0);
        M5.Lcd.print(audiospectrum[band].freqname);
      }
      break;

    case ModeOscilloscope:
      {
        sprite.setTextColor(GREEN);
        dywapitch_inittracking(&pitchTracker);
      }
      break;

    case ModeTuner:
      sprite.setTextSize(1);
      break;

    case ModeAudioCircles:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Audio Circles");
      break;

    case ModeAudioPlasma:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(MAGENTA);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Audio Plasma");
      break;

    case ModeAudioFireworks:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Audio Fireworks");
      break;

    case ModeAudioWaves:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Audio Waves");
      break;

    case ModeAudioSpiral:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(PURPLE);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Audio Spiral");
      break;

    case ModeWaterRipples:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Water Ripples");
      break;

    case ModeAudioPlasmaField:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(MAGENTA);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Plasma Field");
      break;

    case ModeAudioStarField:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Star Field");
      break;

    case ModeFastPlasma:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("FAST Plasma");
      break;

    case ModeAudioRain:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Audio Rain");
      break;

    case ModeExplodingFireworks:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Exploding FW");
      break;

    case ModeLightningStorm:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Lightning Storm");
      break;

    case ModeMatrixRain:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Matrix Rain");
      break;

    case ModeSnowEffect:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Snow Effect");
      break;

    case ModeLavaLamp:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Lava Lamp");
      break;

    case ModeAuroraBorealis:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Aurora Borealis");
      break;

    case ModeKaleidoscope:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(MAGENTA);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Kaleidoscope");
      break;

    case ModeParticleTornado:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Particle Tornado");
      break;

    case ModeSoundVisualizerBars:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Sound Visualizer");
      break;
  }
}

void showSpectrumBars(){
  int *vTemp_ = vTemp[curbuf^1];

  Fixed15FFT::apply_window(vTemp_);
  Fixed15FFT::calc_fft(vTemp_, vTemp_ + SAMPLES);

  int values[BANDS] = {};
  for (int i = 2; i < (SAMPLES/2); i++){ 
    // Don't use sample 0 and only first SAMPLES/2 are usable. 
    // Each array element represents a frequency and its value the amplitude.
    int ampsq = vTemp_[i] * vTemp_[i] + vTemp_[i + SAMPLES] * vTemp_[i + SAMPLES];
    if (ampsq > NOISE_FLOOR) {
      byte bandNum = getBand(i);
      if(bandNum != 8 && ampsq > values[bandNum]) {
        values[bandNum] = ampsq;
      }
    }
  }
  for (byte band = 0; band < BANDS; band++) {
    int log2 = FIX_LOG2<15-RSHIFT>(values[band]);
    if (log2 > -AMPLIFIER) {
      displayBand(band, ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2);
    } else {
      displayBand(band, 0);
    }
  }
  for (byte band = 0; band < BANDS; band++) {
    if (audiospectrum[band].peak > 0) {
      audiospectrum[band].peak -= 2;
    }
    if(audiospectrum[band].peak <= 0) {
      audiospectrum[band].peak = 0;
    }
    // only draw if peak changed
    if(audiospectrum[band].lastpeak != audiospectrum[band].peak) {
      // delete last peak
      uint16_t hpos = BANDS_WIDTH*band + (BANDS_PADDING/2);
      M5.Lcd.drawFastHLine(hpos,TFT_HEIGHT-audiospectrum[band].lastpeak,BAR_WIDTH,BLACK);
      audiospectrum[band].lastpeak = audiospectrum[band].peak;
      uint16_t ypos = TFT_HEIGHT - audiospectrum[band].peak;
      M5.Lcd.drawFastHLine(hpos, ypos, BAR_WIDTH, colormap[ypos]);
    }
  } 
}

void displayBand(int band, int dsize){
  uint16_t hpos = BANDS_WIDTH*band + (BANDS_PADDING/2);
  if (dsize < 0) dsize = 0;
  if(dsize>TFT_HEIGHT-15) {
    dsize = TFT_HEIGHT-15; // leave space for text
  }
  if(dsize < audiospectrum[band].lastval) {
    // lower value, delete some lines
    M5.Lcd.fillRect(hpos, TFT_HEIGHT-audiospectrum[band].lastval,
                    BAR_WIDTH, audiospectrum[band].lastval - dsize,BLACK);
  }
  for (int s = 0; s <= dsize; s=s+4){
    uint16_t ypos = TFT_HEIGHT - s;
    M5.Lcd.drawFastHLine(hpos, ypos, BAR_WIDTH, colormap[ypos]);
  }
  if (dsize > audiospectrum[band].peak){audiospectrum[band].peak = dsize;}
  audiospectrum[band].lastval = dsize;
}

byte getBand(int i) {
  if (i >= 2   && i < 4  ) return 0;  // 125Hz
  if (i >= 4   && i < 8  ) return 1;  // 250Hz
  if (i >= 8   && i < 16 ) return 2;  // 500Hz
  if (i >= 16  && i < 32 ) return 3;  // 1000Hz
  if (i >= 32  && i < 64 ) return 4;  // 2000Hz
  if (i >= 64  && i < 128) return 5;  // 4000Hz
  if (i >= 128 && i < 256) return 6;  // 8000Hz
  if (i >= 256 && i < 512) return 7;  // 16000Hz
  return 8;
}

float calcNumSamples(float f) {
  if (f == 0.0) return SAMPLES/2;
  float s = (float)(DYWAPT_SAMPLERATE * 2) / f;
  if (s > (float)OSC_SAMPLES) {
    do {
      s *= 0.5;
    } while (s > (float)OSC_SAMPLES);
  }
  return s;
}

void showFreq(float freq) {
  if (freq > 0) {
    char strbuf[16];
    sprintf(strbuf, "%8.2fHz", freq);
    sprite.drawString(strbuf, 0, 0, 1);
    float fnote = log2(freq)*12 - 36.376316562f;
    int note = fnote + 0.5f;
    if (note >= 0) {
      sprite.setCursor(TFT_WIDTH/2 - 4, 0);
      sprite.print(notestr[note % 12]);
      sprite.print(note / 12 - 1);
      float cent = (fnote - note) * 100;
      sprintf(strbuf, "%.1fcents", cent);
      sprite.drawRightString(strbuf, TFT_WIDTH, 0, 1);
    }
  }
}

void showOscilloscope()
{
  uint16_t i,j;
  uint16_t *oscbuf_ = oscbuf[curbuf^1];
  int *vTemp_ = vTemp[curbuf ^ 1];
  float freq = dywapitch_computepitch(&pitchTracker, vTemp_);
#if OSC_EXTRASKIP > 0
  if (skipcount < OSC_EXTRASKIP) {
    ++skipcount;
    return;
  }
  skipcount = 0;
#endif
  uint16_t s = calcNumSamples(freq);
  float mx = (float)TFT_WIDTH / s;
  float my;
  uint16_t maxV = 0;
  uint16_t minV = 65535;
  uint16_t offset = 0;
  for (i = 0; i < s; ++i) {
    if (maxV < oscbuf_[i]) maxV = oscbuf_[i];
    if (minV > oscbuf_[i]) {
      minV = oscbuf_[i];
      if (i + s <= OSC_SAMPLES) offset = i;
    }
  }
  if (maxV - minV > OSC_NOISEFLOOR) {
    my = (float)(TFT_HEIGHT-20) / (maxV - minV);
  }
  else {
    my = (float)(TFT_HEIGHT-20) / OSC_NOISEFLOOR;
    minV = (((int)maxV + (int)minV) >> 1) - OSC_NOISEFLOOR/2;
  }
  sprite.fillSprite(BLACK);
  uint16_t y = TFT_HEIGHT - (oscbuf_[offset] - minV) * my;
  for (i = 1; i < s; ++i) {
    uint16_t y2 = TFT_HEIGHT - (oscbuf_[offset + i] - minV) * my;
    sprite.drawLine((uint16_t)((i-1) * mx), y,
                    (uint16_t)(i * mx), y2, LIGHTGREY);
    y = y2;
  }
  showFreq(freq);
  sprite.pushSprite(0,0);
}

void showTuner() {
  int *vTemp_ = vTemp[curbuf ^ 1];
  float freq = dywapitch_computepitch(&pitchTracker, vTemp_);
  float fnote;
  int note;
  if (freq > 0) {
    fnote = log2(freq)*12 - 36.376316562;
    note = fnote + 0.5;
  }
  else {
    note = -1;
  }
  uint32_t bgcolor, fgcolor;
  if (note >= 0) {
    float cent = (fnote - note) * 100;
    if (abs(cent) < 2.) {
      bgcolor = GREEN;
      fgcolor = BLACK;
    }
    else {
      bgcolor = DARKGREY;
      fgcolor = BLACK;
    }
    sprite.fillSprite(bgcolor);
    sprite.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, bgcolor);
    sprite.setTextColor(fgcolor);
    sprite.drawRect(2, 36, TFT_WIDTH-3, TFT_HEIGHT-40, fgcolor);
    sprite.drawRect(TFT_WIDTH/2 + 1, 36, 1, TFT_HEIGHT - 40, fgcolor);
    sprite.fillCircle(((float)TFT_WIDTH/2 + 1) + cent * ((float)(TFT_WIDTH-3)/100), (TFT_HEIGHT+34)/2, 5, fgcolor);
    char strbuf[8];
    sprintf(strbuf, "%s%d", notestr[note % 12], note / 12 - 1);
    sprite.drawCentreString(strbuf, TFT_WIDTH/2, 3, 4);
  }
  else {
    sprite.fillSprite(DARKGREY);
    sprite.drawRect(2, 36, TFT_WIDTH-3, TFT_HEIGHT-40, BLACK);
    sprite.drawLine(TFT_WIDTH/2 + 1, 36, TFT_WIDTH/2 + 1, TFT_HEIGHT - 4, BLACK);
  }
  sprite.pushSprite(0,0);
}

void looptask(void *) {
  while (1) {
    if (needinit) {
      initMode();
      needinit = false;
    }
    if (semaphore) {
      switch(runmode) {
        case ModeSpectrumBars:
          showSpectrumBars();
          break;

        case ModeOscilloscope:
          showOscilloscope();
          break;

        case ModeTuner:
          showTuner();
          break;

        case ModeAudioCircles:
          showAudioCircles();
          break;

        case ModeAudioPlasma:
          showAudioPlasma();
          break;

        case ModeAudioFireworks:
          showAudioFireworks();
          break;

        case ModeAudioWaves:
          showAudioWaves();
          break;

        case ModeAudioSpiral:
          showAudioSpiral();
          break;

        case ModeWaterRipples:
          showWaterRipples();
          break;

        case ModeAudioPlasmaField:
          showAudioPlasmaField();
          break;

        case ModeAudioStarField:
          showAudioStarField();
          break;

        case ModeFastPlasma:
          showFastPlasma();
          break;

        case ModeAudioRain:
          showAudioRain();
          break;

        case ModeExplodingFireworks:
          showExplodingFireworks();
          break;

        case ModeLightningStorm:
          showLightningStorm();
          break;

        case ModeMatrixRain:
          showMatrixRain();
          break;

        case ModeSnowEffect:
          showSnowEffect();
          break;

        case ModeLavaLamp:
          showLavaLamp();
          break;

        case ModeAuroraBorealis:
          showAuroraBorealis();
          break;

        case ModeKaleidoscope:
          showKaleidoscope();
          break;

        case ModeParticleTornado:
          showParticleTornado();
          break;

        case ModeSoundVisualizerBars:
          showSoundVisualizerBars();
          break;
      }
      semaphore = false;
    }
    else {
      vTaskDelay(10);
    }
  }
}

void loop() {
  M5.update();
  if (M5.BtnA.wasReleased()) {
    ++runmode;
    if (runmode >= ModeCount) runmode = 0;

    bufposcount = 0;
    needinit = true;
  }
  
  // Button B toggles text display on/off
  if (M5.BtnB.wasReleased()) {
    showText = !showText; // Toggle text display
  }
  uint16_t i,j;
  j = bufposcount * SAMPLES;
  uint16_t *adcBuffer = &oscbuf[curbuf][j];
  size_t bytesread;
  i2s_read(I2S_NUM_0,(char*)adcBuffer,READ_LEN,&bytesread,portMAX_DELAY);
  int32_t dc = 0;
  for (int i = 0; i < SAMPLES; ++i) {
    dc += adcBuffer[i];
  }
  dc /= SAMPLES;

  switch(runmode) {
    case ModeSpectrumBars:
      for (int i = 0; i < SAMPLES; ++i) {
        
        vTemp[curbuf][i] = (int)adcBuffer[i] - dc;
        vTemp[curbuf][i + SAMPLES] = 0;
      }
      curbuf ^= 1;
      semaphore = true;
      break;
    case ModeOscilloscope:
      for (i = 0; i < SAMPLES; ++i) {
        vTemp[curbuf][i + j] = (int)adcBuffer[i] - dc;
      }
      if (++bufposcount >= OSC_SKIPCOUNT) {
        bufposcount = 0;
        curbuf ^= 1;
        semaphore = true;
      }
      break;
    case ModeTuner:
      j = bufposcount * SAMPLES;
      for (i = 0; i < SAMPLES; ++i) {
        vTemp[curbuf][i + j] = (int)adcBuffer[i] - dc;
      }
      if (++bufposcount >= OSC_SKIPCOUNT) {
        bufposcount = 0;
        curbuf ^= 1;
        semaphore = true;
      }
      break;
    case ModeAudioCircles:
    case ModeAudioPlasma:
    case ModeAudioFireworks:
    case ModeAudioWaves:
    case ModeAudioSpiral:
    case ModeWaterRipples:
    case ModeAudioPlasmaField:
    case ModeAudioStarField:
    case ModeFastPlasma:
    case ModeAudioRain:
    case ModeExplodingFireworks:
    case ModeLightningStorm:
    case ModeMatrixRain:
    case ModeSnowEffect:
    case ModeLavaLamp:
    case ModeAuroraBorealis:
    case ModeKaleidoscope:
    case ModeParticleTornado:
    case ModeSoundVisualizerBars:
      for (int i = 0; i < SAMPLES; ++i) {
        vTemp[curbuf][i] = (int)adcBuffer[i] - dc;
        vTemp[curbuf][i + SAMPLES] = 0;
      }
      curbuf ^= 1;
      semaphore = true;
      break;
  }
}

// HSV to RGB color conversion for psychedelic effects
uint16_t getColorFromHSV(float h, float s, float v) {
  float c = v * s;
  float x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
  float m = v - c;
  
  float r, g, b;
  if (h >= 0 && h < 60) {
    r = c; g = x; b = 0;
  } else if (h >= 60 && h < 120) {
    r = x; g = c; b = 0;
  } else if (h >= 120 && h < 180) {
    r = 0; g = c; b = x;
  } else if (h >= 180 && h < 240) {
    r = 0; g = x; b = c;
  } else if (h >= 240 && h < 300) {
    r = x; g = 0; b = c;
  } else {
    r = c; g = 0; b = x;
  }
  
  r = (r + m) * 255;
  g = (g + m) * 255;
  b = (b + m) * 255;
  
  return M5.Lcd.color565((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

// Helper function to calculate audio bands (reused by all effects)
void calculateAudioBands(int bands[8]) {
  int *vTemp_ = vTemp[curbuf^1];
  
  // Use same FFT calculations as the spectrum analyzer
  Fixed15FFT::apply_window(vTemp_);
  Fixed15FFT::calc_fft(vTemp_, vTemp_ + SAMPLES);

  // Calculate audio levels for different frequency bands
  for (int i = 0; i < 8; i++) bands[i] = 0;
  
  for (int i = 2; i < (SAMPLES/2); i++){ 
    int ampsq = vTemp_[i] * vTemp_[i] + vTemp_[i + SAMPLES] * vTemp_[i + SAMPLES];
    if (ampsq > NOISE_FLOOR) {
      byte bandNum = getBand(i);
      if(bandNum < 8 && ampsq > bands[bandNum]) {
        bands[bandNum] = ampsq;
      }
    }
  }
}

// Audio-reactive circles - simple but effective
void showAudioCircles() {
  static float hue = 0;
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Clear screen
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Audio Reactive");
  }
  
  // Draw audio-reactive circles from center (full screen)
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  for (int band = 0; band < 8; band++) {
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        int intensity = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
        intensity = constrain(intensity, 5, 80);
        
        // Multiple circles for effect
        for (int ring = 0; ring < 3; ring++) {
          int radius = (intensity / 3) + (band * 8) + (ring * 15);
          if (radius > 5 && radius < 100) {
            // Different hue for each band
            uint16_t color = getColorFromHSV(hue + (band * 45) + (ring * 15), 1.0f, 0.8f - ring * 0.2f);
            M5.Lcd.drawCircle(centerX, centerY, radius, color);
          }
        }
      }
    }
  }
  
  // Draw some baseline circles even with no audio
  for (int ring = 0; ring < 2; ring++) {
    int radius = 10 + ring * 20;
    uint16_t color = getColorFromHSV(hue + ring * 180, 0.5f, 0.3f);
    M5.Lcd.drawCircle(centerX, centerY, radius, color);
  }
  
  // Update hue for color cycling
  hue += 2;
  if (hue >= 360) hue = 0;
}

// Audio-reactive plasma field
void showAudioPlasma() {
  static float phase = 0;
  static float audioPhase = 0;
  static unsigned long lastUpdate = 0;
  static float lastAudioIntensity = 0;
  
  unsigned long startTime = millis();
  Serial.print("Plasma start: ");
  Serial.println(startTime);
  
  int bands[8];
  calculateAudioBands(bands);
  
  Serial.print("After calculateAudioBands: ");
  Serial.println(millis() - startTime);
  
  // Calculate overall audio intensity
  float audioIntensity = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioIntensity += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  audioIntensity = constrain(audioIntensity / 50.0f, 0.2f, 5.0f); // Increased sensitivity
  
  // Much more sensitive and responsive
  lastAudioIntensity = lastAudioIntensity * 0.7f + audioIntensity * 0.3f;
  
  // Clear screen every frame for immediate response
  M5.Lcd.fillScreen(BLACK);
  
  Serial.print("After fillScreen: ");
  Serial.println(millis() - startTime);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Audio Plasma");
  }
  // Draw plasma field (center focused, full screen) - RESTORED FROM TUNED VERSION
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Full screen plasma with proper resolution
  for (int y = 12; y < TFT_HEIGHT - 2; y += 2) {
    for (int x = 2; x < TFT_WIDTH - 2; x += 2) {
      // Original working plasma calculation with proper scaling
      float dx = x - centerX;
      float dy = y - centerY;
      float dist = sqrt(dx*dx + dy*dy);
      
      float value = sin(dist * 0.1 + phase) + 
                   sin(x * 0.05 + audioPhase) + 
                   sin(y * 0.04 + phase * 0.7);
      value *= lastAudioIntensity;
      
      // Convert to color with full screen morphing effect
      float hue = fmod((value + 2) * 60 + phase * 20, 360);
      uint16_t color = getColorFromHSV(hue, 1.0f, constrain(lastAudioIntensity * 0.8f, 0.2f, 1.0f));
      
      M5.Lcd.fillRect(x, y, 2, 2, color);
    }
  }
  
  phase += 0.08f + lastAudioIntensity * 0.05f;
  audioPhase += lastAudioIntensity * 0.2f;
  
  Serial.print("Plasma total time: ");
  Serial.println(millis() - startTime);
  Serial.println("---");
}

// Audio-reactive fireworks  
void showAudioFireworks() {
  static float sparkles[20][4]; // x, y, life, hue
  static bool initialized = false;
  static float baseHue = 0;
  
  if (!initialized) {
    for (int i = 0; i < 20; i++) {
      sparkles[i][2] = 0; // life = 0 means inactive
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Clear screen with fade effect
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Audio Fireworks");
  }
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Trigger new sparkles based on audio
  for (int band = 0; band < 8; band++) {
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        int intensity = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
        if (intensity > 20 && random(100) < 30) {
          // Find inactive sparkle
          for (int i = 0; i < 20; i++) {
            if (sparkles[i][2] <= 0) {
              // Launch new sparkle from center
              float angle = random(0, 628) / 100.0f; // 0 to 2π
              float distance = random(30, 100); // Use more screen space
              sparkles[i][0] = centerX + cos(angle) * distance; // x
              sparkles[i][1] = centerY + sin(angle) * distance; // y  
              sparkles[i][2] = random(30, 80); // life
              sparkles[i][3] = baseHue + band * 45; // hue based on frequency band
              break;
            }
          }
        }
      }
    }
  }
  
  // Update and draw sparkles
  for (int i = 0; i < 20; i++) {
    if (sparkles[i][2] > 0) {
      // Draw sparkle
      int x = sparkles[i][0];
      int y = sparkles[i][1];
      float life = sparkles[i][2];
      float hue = sparkles[i][3];
      
      if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
        float brightness = life / 80.0f;
        uint16_t color = getColorFromHSV(fmod(hue, 360), 1.0f, brightness);
        
        // Draw sparkle with size based on life
        int size = constrain(life / 20, 1, 4);
        for (int dx = -size/2; dx <= size/2; dx++) {
          for (int dy = -size/2; dy <= size/2; dy++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < TFT_WIDTH && py >= 12 && py < TFT_HEIGHT) {
              M5.Lcd.drawPixel(px, py, color);
            }
          }
        }
      }
      
      // Update sparkle
      sparkles[i][2] -= 1.5f; // fade out
      
      // Add slight movement
      sparkles[i][0] += random(-1, 2);
      sparkles[i][1] += random(-1, 2);
    }
  }
  
  baseHue += 1;
  if (baseHue >= 360) baseHue = 0;
}

// Audio-reactive sine waves - smooth flowing curves
void showAudioWaves() {
  static float phase = 0;
  static float hue = 0;
  
  int bands[8];
  calculateAudioBands(bands);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Audio Waves");
  }
  int centerY = TFT_HEIGHT / 2;
  
  // Draw multiple layered sine waves for different frequency bands
  for (int band = 0; band < 8; band++) {
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        float amplitude = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
        amplitude = constrain(amplitude / 3.0f, 2.0f, 25.0f);
        
        // Each band gets different wave parameters
        float frequency = 0.02f + band * 0.005f;
        float bandPhase = phase + band * 0.5f;
        uint16_t waveColor = getColorFromHSV(hue + band * 45, 1.0f, 0.8f);
        
        // Draw smooth sine wave
        uint16_t lastY = centerY + amplitude * sin(bandPhase);
        for (int x = 1; x < TFT_WIDTH - 1; x++) {
          uint16_t currentY = centerY + amplitude * sin(x * frequency + bandPhase);
          currentY = constrain(currentY, 12, TFT_HEIGHT - 5);
          lastY = constrain(lastY, 12, TFT_HEIGHT - 5);
          
          M5.Lcd.drawLine(x - 1, lastY, x, currentY, waveColor);
          lastY = currentY;
        }
      }
    }
  }
  
  // Always show a base wave even without audio
  float baseAmplitude = 8;
  uint16_t baseColor = getColorFromHSV(hue, 0.6f, 0.4f);
  uint16_t lastY = centerY + baseAmplitude * sin(phase);
  for (int x = 1; x < TFT_WIDTH - 1; x++) {
    uint16_t currentY = centerY + baseAmplitude * sin(x * 0.03f + phase);
    M5.Lcd.drawLine(x - 1, lastY, x, currentY, baseColor);
    lastY = currentY;
  }
  
  phase += 0.15f;
  hue += 1;
  if (hue >= 360) hue = 0;
}

// Audio-reactive spiral waves - hypnotic spirals from center
void showAudioSpiral() {
  static float phase = 0;
  static float hue = 0;
  
  int bands[8];
  calculateAudioBands(bands);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Audio Spiral");
  }
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Draw multiple spirals for different frequency bands
  for (int band = 0; band < 8; band++) {
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        float intensity = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
        intensity = constrain(intensity / 4.0f, 1.0f, 15.0f);
        
        // Each band creates a spiral with different parameters
        float spiralTightness = 0.3f + band * 0.1f;
        uint16_t spiralColor = getColorFromHSV(hue + band * 30, 1.0f, 0.7f);
        
        // Draw spiral
        float lastX = centerX, lastY = centerY;
        for (float angle = 0; angle < 8 * PI; angle += 0.2f) {
          float radius = intensity * (1 + sin(angle * spiralTightness + phase + band));
          float x = centerX + radius * cos(angle + phase * 0.5f);
          float y = centerY + radius * sin(angle + phase * 0.5f);
          
          if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
            if (angle > 0.2f) {
              M5.Lcd.drawLine(lastX, lastY, x, y, spiralColor);
            }
            lastX = x;
            lastY = y;
          }
        }
      }
    }
  }
  
  // Base spiral always visible
  uint16_t baseColor = getColorFromHSV(hue, 0.5f, 0.3f);
  float lastX = centerX, lastY = centerY;
  for (float angle = 0; angle < 6 * PI; angle += 0.3f) {
    float radius = 5 * (1 + sin(angle * 0.2f + phase));
    float x = centerX + radius * cos(angle + phase * 0.3f);
    float y = centerY + radius * sin(angle + phase * 0.3f);
    
    if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT && angle > 0.3f) {
      M5.Lcd.drawLine(lastX, lastY, x, y, baseColor);
    }
    lastX = x;
    lastY = y;
  }
  
  phase += 0.08f;
  hue += 2;
  if (hue >= 360) hue = 0;
}

// Water ripples - audio creates expanding circles
void showWaterRipples() {
  static float ripples[10][3]; // x, y, age
  static bool initialized = false;
  static float hue = 0;
  
  if (!initialized) {
    for (int i = 0; i < 10; i++) {
      ripples[i][2] = -1; // inactive
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Water Ripples");
  }
  // Create new ripples based on audio
  for (int band = 0; band < 8; band++) {
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        int intensity = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
        if (intensity > 15 && random(100) < 20) {
          // Find inactive ripple slot
          for (int i = 0; i < 10; i++) {
            if (ripples[i][2] < 0) {
              ripples[i][0] = random(40, TFT_WIDTH - 40); // x
              ripples[i][1] = random(25, TFT_HEIGHT - 15); // y
              ripples[i][2] = 0; // age
              break;
            }
          }
        }
      }
    }
  }
  
  // Update and draw ripples
  for (int i = 0; i < 10; i++) {
    if (ripples[i][2] >= 0) {
      float age = ripples[i][2];
      float radius = age * 3;
      float brightness = constrain(1.0f - age / 30.0f, 0.1f, 1.0f);
      
      if (radius < 80 && brightness > 0.1f) {
        uint16_t rippleColor = getColorFromHSV(hue + i * 36, 0.8f, brightness);
        
        // Draw expanding circle
        M5.Lcd.drawCircle(ripples[i][0], ripples[i][1], radius, rippleColor);
        if (radius > 5) {
          M5.Lcd.drawCircle(ripples[i][0], ripples[i][1], radius - 2, rippleColor);
        }
      }
      
      ripples[i][2] += 1.2f;
      if (ripples[i][2] > 35) {
        ripples[i][2] = -1; // deactivate
      }
    }
  }
  
  hue += 0.5f;
  if (hue >= 360) hue = 0;
}

// Audio-reactive plasma field - recreate the awesome fast bright plasma
void showAudioPlasmaField() {
  static float phase = 0;
  static float audioPhase = 0;
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate overall audio intensity
  float audioIntensity = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioIntensity += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  audioIntensity = constrain(audioIntensity / 50.0f, 0.2f, 5.0f);
  
  // Clear screen every frame for immediate updates
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Plasma Field");
  }
  // Draw plasma field - much faster rendering
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Bigger pixels, fewer calculations for speed
  for (int y = 12; y < TFT_HEIGHT - 2; y += 6) {
    for (int x = 2; x < TFT_WIDTH - 2; x += 6) {
      // Simpler, faster plasma calculation
      float dx = (x - centerX) * 0.02f;
      float dy = (y - centerY) * 0.02f;
      
      float value = sin(dx + phase) + 
                   cos(dy + audioPhase) + 
                   sin((dx + dy) * 0.5f + phase * 1.3f);
      
      // Always visible plasma with audio enhancement
      float baseIntensity = 1.5f; // Always bright
      float totalIntensity = baseIntensity + (audioIntensity * 2.0f);
      
      // Convert to bright colors
      float hue = fmod((value * 60) + phase * 50, 360);
      float saturation = 1.0f;
      float brightness = constrain(totalIntensity * 0.4f, 0.6f, 1.0f);
      
      uint16_t color = getColorFromHSV(hue, saturation, brightness);
      
      // Draw bigger pixels for speed and visibility
      M5.Lcd.fillRect(x, y, 5, 5, color);
    }
  }
  
  // Continuous animation regardless of audio
  phase += 0.12f; // Always animating
  audioPhase += 0.08f + audioIntensity * 0.1f;
}

// Audio-reactive star field - stars moving toward you from center
void showAudioStarField() {
  static float stars[100][4]; // x, y, z, brightness
  static bool initialized = false;
  static float speed = 1.0f;
  
  if (!initialized) {
    // Initialize stars randomly in 3D space
    for (int i = 0; i < 100; i++) {
      stars[i][0] = random(-1000, 1000); // x
      stars[i][1] = random(-1000, 1000); // y  
      stars[i][2] = random(1, 1000);     // z (distance)
      stars[i][3] = random(50, 255);     // brightness
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio speed multiplier
  float audioSpeed = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioSpeed += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  speed = 1.0f + constrain(audioSpeed / 100.0f, 0, 8.0f);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Star Field");
  }
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Update and draw stars
  for (int i = 0; i < 100; i++) {
    // Move star toward camera (decrease z)
    stars[i][2] -= speed;
    
    // If star is too close, reset it far away
    if (stars[i][2] <= 1) {
      stars[i][0] = random(-1000, 1000);
      stars[i][1] = random(-1000, 1000);
      stars[i][2] = random(500, 1000);
      stars[i][3] = random(50, 255);
    }
    
    // Project 3D position to 2D screen
    float screenX = centerX + (stars[i][0] / stars[i][2]) * 100;
    float screenY = centerY + (stars[i][1] / stars[i][2]) * 100;
    
    // Only draw stars that are on screen
    if (screenX >= 0 && screenX < TFT_WIDTH && screenY >= 12 && screenY < TFT_HEIGHT) {
      // Star size and brightness based on distance
      float distance = stars[i][2];
      int starSize = constrain(map(distance, 1, 1000, 4, 1), 1, 4);
      float brightness = constrain(stars[i][3] / distance * 100, 0.2f, 1.0f);
      
      // Color based on audio and distance
      float hue = fmod(distance * 0.1f + speed * 50, 360);
      uint16_t starColor = getColorFromHSV(hue, 0.7f, brightness);
      
      // Draw star with trails for fast movement
      if (speed > 3.0f && starSize > 1) {
        // Draw motion trail
        float trailX = screenX + (stars[i][0] / stars[i][2]) * speed * 2;
        float trailY = screenY + (stars[i][1] / stars[i][2]) * speed * 2;
        if (trailX >= 0 && trailX < TFT_WIDTH && trailY >= 12 && trailY < TFT_HEIGHT) {
          M5.Lcd.drawLine(screenX, screenY, trailX, trailY, starColor);
        }
      }
      
      // Draw the star
      if (starSize == 1) {
        M5.Lcd.drawPixel(screenX, screenY, starColor);
      } else {
        M5.Lcd.fillRect(screenX - starSize/2, screenY - starSize/2, starSize, starSize, starColor);
      }
    }
  }
}

// Audio-reactive star field - stars moving toward you from center


// BLAZING FAST PLASMA - NO BULLSHIT VERSION
void showFastPlasma() {
  static float t = 0;
  static int audioLevel = 0;
  
  // Get audio instantly - no FFT bullshit
  int bands[8];
  calculateAudioBands(bands);
  audioLevel = 0;
  for (int i = 0; i < 4; i++) audioLevel += bands[i] >> 8;
  audioLevel = constrain(audioLevel, 10, 200);
  
  // NO screen clearing - just overdraw everything
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("FAST Plasma");
  }
  
  // FAST plasma - huge pixels, minimal math
  for (int y = 15; y < TFT_HEIGHT; y += 8) {
    for (int x = 0; x < TFT_WIDTH; x += 8) {
      // Super simple plasma math
      float v = sin((x + t) * 0.02f) + cos((y + t * 0.7f) * 0.03f);
      v += audioLevel * 0.01f;
      
      // Fast color - no HSV conversion
      int r = constrain(128 + 127 * sin(v + t), 0, 255);
      int g = constrain(128 + 127 * sin(v + t + 2), 0, 255); 
      int b = constrain(128 + 127 * sin(v + t + 4), 0, 255);
      
      uint16_t color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      
      // Big ass pixels
      M5.Lcd.fillRect(x, y, 8, 8, color);
    }
  }
  
  t += 0.3f + audioLevel * 0.01f;
}


// Audio-reactive rain effect
void showAudioRain() {
  static float raindrops[50][3]; // x, y, speed
  static bool initialized = false;
  static float audioSpeed = 1.0f;
  static float hue = 180; // Start with blue
  
  if (!initialized) {
    for (int i = 0; i < 50; i++) {
      raindrops[i][0] = random(0, TFT_WIDTH);   // x
      raindrops[i][1] = random(-50, TFT_HEIGHT); // y
      raindrops[i][2] = random(1, 4);          // speed
    }
    initialized = true;
  }
  
  // Get audio for speed and color
  int bands[8];
  calculateAudioBands(bands);
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) audioLevel += bands[i] >> 6;
  audioSpeed = 1.0f + constrain(audioLevel / 50.0f, 0, 5.0f);
  
  // Change hue based on audio - blue to red spectrum
  hue = 240 - constrain(audioLevel / 2.0f, 0, 120); // 240=blue, 120=green, 0=red
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Audio Rain");
  }
  // Update and draw raindrops
  for (int i = 0; i < 50; i++) {
    // Move raindrop down
    raindrops[i][1] += raindrops[i][2] * audioSpeed;
    
    // Reset if off screen
    if (raindrops[i][1] > TFT_HEIGHT) {
      raindrops[i][0] = random(0, TFT_WIDTH);
      raindrops[i][1] = random(-20, -5);
      raindrops[i][2] = random(1, 4);
    }
    
    // Draw raindrop with trail
    int x = raindrops[i][0];
    int y = raindrops[i][1];
    if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
      uint16_t color = getColorFromHSV(hue, 0.8f, 0.9f);
      
      // Draw rain line (2-3 pixels tall)
      M5.Lcd.drawPixel(x, y, color);
      if (y > 12) M5.Lcd.drawPixel(x, y-1, color);
      if (y > 13) {
        uint16_t fadeColor = getColorFromHSV(hue, 0.8f, 0.5f);
        M5.Lcd.drawPixel(x, y-2, fadeColor);
      }
    }
  }
}

// Exploding fireworks - dots that actually explode
void showExplodingFireworks() {
  static float fireworks[20][6]; // x, y, vx, vy, life, type (0=rising, 1=exploded)
  static float explosions[100][5]; // x, y, vx, vy, life
  static bool initialized = false;
  static float hue = 0;
  
  if (!initialized) {
    for (int i = 0; i < 20; i++) {
      fireworks[i][4] = -1; // inactive
    }
    for (int i = 0; i < 100; i++) {
      explosions[i][4] = -1; // inactive
    }
    initialized = true;
  }
  
  // Get audio for launching new fireworks
  int bands[8];
  calculateAudioBands(bands);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Exploding FW");
  }
  // Launch new fireworks based on audio
  for (int band = 0; band < 8; band++) {
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        int intensity = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
        if (intensity > 20 && random(100) < 15) {
          // Find inactive firework slot
          for (int i = 0; i < 20; i++) {
            if (fireworks[i][4] < 0) {
              fireworks[i][0] = random(20, TFT_WIDTH - 20); // x
              fireworks[i][1] = TFT_HEIGHT - 5;            // y (bottom)
              fireworks[i][2] = random(-20, 20) / 10.0f;   // vx
              fireworks[i][3] = -random(30, 60) / 10.0f;   // vy (upward)
              fireworks[i][4] = 100;                       // life
              fireworks[i][5] = 0;                         // rising type
              break;
            }
          }
        }
      }
    }
  }
  
  // Update fireworks
  for (int i = 0; i < 20; i++) {
    if (fireworks[i][4] > 0) {
      if (fireworks[i][5] == 0) { // Rising firework
        fireworks[i][0] += fireworks[i][2]; // x += vx
        fireworks[i][1] += fireworks[i][3]; // y += vy
        fireworks[i][3] += 0.1f;            // gravity
        fireworks[i][4] -= 2;               // lose life
        
        // Draw rising firework
        int x = fireworks[i][0];
        int y = fireworks[i][1];
        if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
          uint16_t color = getColorFromHSV(hue + i * 30, 1.0f, 0.8f);
          M5.Lcd.fillRect(x-1, y-1, 2, 2, color);
        }
        
        // Explode when velocity changes or life runs out
        if (fireworks[i][3] > 0 || fireworks[i][4] < 50) {
          // Create explosion particles
          for (int j = 0; j < 100; j++) {
            if (explosions[j][4] < 0) { // Find empty slot
              for (int k = 0; k < 8 && j < 100; k++, j++) {
                if (explosions[j][4] < 0) {
                  float angle = (k * 45) * PI / 180; // 8 directions
                  float speed = random(10, 40) / 10.0f;
                  explosions[j][0] = fireworks[i][0];           // x
                  explosions[j][1] = fireworks[i][1];           // y
                  explosions[j][2] = cos(angle) * speed;        // vx
                  explosions[j][3] = sin(angle) * speed;        // vy
                  explosions[j][4] = random(60, 120);           // life
                }
              }
              break;
            }
          }
          fireworks[i][4] = -1; // deactivate firework
        }
      }
    }
  }
  
  // Update explosion particles
  for (int i = 0; i < 100; i++) {
    if (explosions[i][4] > 0) {
      explosions[i][0] += explosions[i][2]; // x += vx
      explosions[i][1] += explosions[i][3]; // y += vy
      explosions[i][3] += 0.05f;             // gravity
      explosions[i][4] -= 1;                 // lose life
      
      // Draw explosion particle
      int x = explosions[i][0];
      int y = explosions[i][1];
      if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
        float brightness = explosions[i][4] / 120.0f;
        uint16_t color = getColorFromHSV(hue + i * 10, 1.0f, brightness);
        M5.Lcd.drawPixel(x, y, color);
      }
    }
  }
  
  hue += 1;
  if (hue >= 360) hue = 0;
}

// Lightning Storm - Bass-reactive lightning bolts and flashes
void showLightningStorm() {
  static float lightning[10][5]; // x1, y1, x2, y2, life
  static bool initialized = false;
  static int flashTimer = 0;
  static bool isFlashing = false;
  static float hue = 0;
  
  if (!initialized) {
    for (int i = 0; i < 10; i++) {
      lightning[i][4] = -1; // inactive
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Focus on bass frequencies (bands 0-2) for lightning triggers
  int bassLevel = 0;
  for (int i = 0; i < 3; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        bassLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  
  // Screen flash effect for strong bass
  if (bassLevel > 100 && !isFlashing) {
    isFlashing = true;
    flashTimer = 15; // Flash duration
  }
  
  if (isFlashing) {
    // White flash background
    M5.Lcd.fillScreen(WHITE);
    flashTimer--;
    if (flashTimer <= 0) {
      isFlashing = false;
    }
  } else {
    M5.Lcd.fillScreen(BLACK);
  }
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(isFlashing ? BLACK : WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Lightning Storm");
  }
  // Create new lightning bolts on strong bass
  if (bassLevel > 80 && random(100) < 40) {
    for (int i = 0; i < 10; i++) {
      if (lightning[i][4] < 0) {
        // Create jagged lightning bolt from top
        lightning[i][0] = random(20, TFT_WIDTH - 20); // x1 (top)
        lightning[i][1] = 15; // y1 (top)
        lightning[i][2] = lightning[i][0] + random(-40, 40); // x2 (bottom, jagged)
        lightning[i][3] = random(TFT_HEIGHT/2, TFT_HEIGHT - 10); // y2 (bottom)
        lightning[i][4] = random(20, 40); // life
        break;
      }
    }
  }
  
  // Update and draw lightning bolts
  for (int i = 0; i < 10; i++) {
    if (lightning[i][4] > 0) {
      float life = lightning[i][4];
      float brightness = constrain(life / 30.0f, 0.3f, 1.0f);
      
      // Lightning color - blue-white to yellow
      uint16_t boltColor;
      if (isFlashing) {
        boltColor = BLUE; // Dark bolt on white background
      } else {
        boltColor = getColorFromHSV(60 - life, 0.8f, brightness); // Blue to yellow
      }
      
      // Draw main bolt
      M5.Lcd.drawLine(lightning[i][0], lightning[i][1], 
                      lightning[i][2], lightning[i][3], boltColor);
      
      // Add branches for realism
      if (life > 15) {
        int midX = (lightning[i][0] + lightning[i][2]) / 2;
        int midY = (lightning[i][1] + lightning[i][3]) / 2;
        int branchX = midX + random(-30, 30);
        int branchY = midY + random(10, 40);
        
        if (branchX >= 0 && branchX < TFT_WIDTH && branchY < TFT_HEIGHT) {
          M5.Lcd.drawLine(midX, midY, branchX, branchY, boltColor);
        }
      }
      
      // Fade out
      lightning[i][4] -= 2;
    }
  }
  
  hue += 0.5f;
  if (hue >= 360) hue = 0;
}

// Matrix Rain - Digital characters falling like The Matrix
void showMatrixRain() {
  static char matrix[30][20]; // character grid
  static float speeds[30]; // fall speed for each column
  static int lengths[30]; // trail length for each column
  static bool initialized = false;
  static float hue = 120; // Start green
  
  if (!initialized) {
    for (int col = 0; col < 30; col++) {
      speeds[col] = random(1, 4) * 0.5f;
      lengths[col] = random(5, 15);
      for (int row = 0; row < 20; row++) {
        // Mix of numbers, letters, and Matrix-like chars
        int charType = random(0, 4);
        if (charType == 0) {
          matrix[col][row] = '0' + random(0, 10); // Numbers
        } else if (charType == 1) {
          matrix[col][row] = 'A' + random(0, 26); // Letters
        } else {
          // Matrix-style chars (using available ASCII)
          char matrixChars[] = {'@', '#', '$', '%', '&', '*', '+', '=', '?'};
          matrix[col][row] = matrixChars[random(0, 9)];
        }
      }
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio influence on speed and new character generation
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  float speedMultiplier = 1.0f + constrain(audioLevel / 100.0f, 0, 3.0f);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Matrix Rain");
  }
  // Update and draw matrix columns
  static float positions[30] = {0}; // Column positions
  
  for (int col = 0; col < 30; col++) {
    int x = col * 8; // 8 pixels per character column
    
    if (x >= TFT_WIDTH) continue;
    
    // Move column down
    positions[col] += speeds[col] * speedMultiplier;
    
    // Reset column if it's moved off screen
    if (positions[col] > TFT_HEIGHT + lengths[col] * 8) {
      positions[col] = -lengths[col] * 8;
      
      // Generate new characters when column resets (audio reactive)
      if (audioLevel > 20) {
        for (int row = 0; row < 20; row++) {
          if (random(100) < 30) { // 30% chance to change character
            int charType = random(0, 4);
            if (charType == 0) {
              matrix[col][row] = '0' + random(0, 10);
            } else if (charType == 1) {
              matrix[col][row] = 'A' + random(0, 26);
            } else {
              char matrixChars[] = {'@', '#', '$', '%', '&', '*', '+', '=', '?'};
              matrix[col][row] = matrixChars[random(0, 9)];
            }
          }
        }
      }
    }
    
    // Draw falling characters in this column
    for (int i = 0; i < lengths[col]; i++) {
      int y = positions[col] + i * 8;
      
      if (y >= 12 && y < TFT_HEIGHT - 8) {
        // Fade effect - brighter at the head of the trail
        float brightness = constrain(1.0f - (i / (float)lengths[col]), 0.2f, 1.0f);
        
        // Color shifts with audio - green to cyan
        float currentHue = 120 + (audioLevel * 0.3f);
        if (currentHue > 180) currentHue = 180;
        
        uint16_t color = getColorFromHSV(currentHue, 1.0f, brightness);
        
        // Draw the character
        M5.Lcd.setTextColor(color);
        M5.Lcd.setCursor(x, y);
        M5.Lcd.print(matrix[col][i % 20]);
      }
    }
  }
}

// Snow Effect - Gentle falling snow that speeds up with audio
void showSnowEffect() {
  static float snowflakes[80][4]; // x, y, speed, size
  static bool initialized = false;
  static float windOffset = 0;
  
  if (!initialized) {
    for (int i = 0; i < 80; i++) {
      snowflakes[i][0] = random(0, TFT_WIDTH);   // x
      snowflakes[i][1] = random(-100, TFT_HEIGHT); // y
      snowflakes[i][2] = random(5, 15) / 10.0f;  // speed (0.5-1.5)
      snowflakes[i][3] = random(1, 4);           // size (1-3 pixels)
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio influence on snow speed and wind
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  float speedMultiplier = 1.0f + constrain(audioLevel / 80.0f, 0, 4.0f);
  float windStrength = constrain(audioLevel / 100.0f, 0, 2.0f);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Snow Effect");
  }
  // Update wind effect
  windOffset += windStrength * 0.1f;
  
  // Update and draw snowflakes
  for (int i = 0; i < 80; i++) {
    // Move snowflake down
    snowflakes[i][1] += snowflakes[i][2] * speedMultiplier;
    
    // Add gentle side-to-side wind motion
    float windSway = sin(windOffset + i * 0.5f) * windStrength;
    snowflakes[i][0] += windSway * 0.5f;
    
    // Reset if off screen
    if (snowflakes[i][1] > TFT_HEIGHT) {
      snowflakes[i][0] = random(0, TFT_WIDTH);
      snowflakes[i][1] = random(-30, -5);
      snowflakes[i][2] = random(5, 15) / 10.0f;
      snowflakes[i][3] = random(1, 4);
    }
    
    // Keep snowflakes on screen horizontally
    if (snowflakes[i][0] < 0) snowflakes[i][0] = TFT_WIDTH - 1;
    if (snowflakes[i][0] >= TFT_WIDTH) snowflakes[i][0] = 0;
    
    // Draw snowflake
    int x = snowflakes[i][0];
    int y = snowflakes[i][1];
    int size = snowflakes[i][3];
    
    if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
      // White snowflakes with slight blue tint based on audio
      uint16_t snowColor = WHITE;
      if (audioLevel > 30) {
        // Add blue tint when there's audio
        snowColor = getColorFromHSV(200, constrain(audioLevel / 200.0f, 0, 0.3f), 1.0f);
      }
      
      // Draw snowflake based on size
      if (size == 1) {
        M5.Lcd.drawPixel(x, y, snowColor);
      } else if (size == 2) {
        M5.Lcd.fillRect(x-1, y-1, 2, 2, snowColor);
      } else {
        // Large snowflake - draw a simple cross pattern
        M5.Lcd.drawPixel(x, y, snowColor);
        M5.Lcd.drawPixel(x-1, y, snowColor);
        M5.Lcd.drawPixel(x+1, y, snowColor);
        M5.Lcd.drawPixel(x, y-1, snowColor);
        M5.Lcd.drawPixel(x, y+1, snowColor);
      }
    }
  }
}

// Lava Lamp - Morphing blob shapes that react to sound
void showLavaLamp() {
  static float blobs[8][5]; // x, y, size, vx, vy
  static bool initialized = false;
  static float hue = 0;
  static float bubbleTimer = 0;
  
  if (!initialized) {
    for (int i = 0; i < 8; i++) {
      blobs[i][0] = random(20, TFT_WIDTH - 20);  // x
      blobs[i][1] = random(30, TFT_HEIGHT - 30); // y
      blobs[i][2] = random(10, 30);              // size
      blobs[i][3] = random(-20, 20) / 100.0f;    // vx (velocity x)
      blobs[i][4] = random(-20, 20) / 100.0f;    // vy (velocity y)
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio influence on blob behavior
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  
  float audioInfluence = constrain(audioLevel / 50.0f, 0.5f, 3.0f);
  bubbleTimer += audioInfluence * 0.1f;
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Lava Lamp");
  }
  // Update blob positions and sizes
  for (int i = 0; i < 8; i++) {
    // Move blobs slowly
    blobs[i][0] += blobs[i][3] * audioInfluence;
    blobs[i][1] += blobs[i][4] * audioInfluence;
    
    // Pulsate size with audio
    float baseSizeVariation = sin(bubbleTimer + i) * 5;
    float audioSizeBoost = audioLevel * 0.2f;
    blobs[i][2] = 15 + baseSizeVariation + audioSizeBoost;
    blobs[i][2] = constrain(blobs[i][2], 8, 40);
    
    // Bounce off walls
    if (blobs[i][0] <= blobs[i][2]/2 || blobs[i][0] >= TFT_WIDTH - blobs[i][2]/2) {
      blobs[i][3] = -blobs[i][3];
    }
    if (blobs[i][1] <= 15 + blobs[i][2]/2 || blobs[i][1] >= TFT_HEIGHT - blobs[i][2]/2) {
      blobs[i][4] = -blobs[i][4];
    }
    
    // Keep blobs in bounds
    blobs[i][0] = constrain(blobs[i][0], blobs[i][2]/2, TFT_WIDTH - blobs[i][2]/2);
    blobs[i][1] = constrain(blobs[i][1], 15 + blobs[i][2]/2, TFT_HEIGHT - blobs[i][2]/2);
  }
  
  // Draw blobs with lava lamp colors
  for (int i = 0; i < 8; i++) {
    int x = blobs[i][0];
    int y = blobs[i][1];
    int radius = blobs[i][2] / 2;
    
    // Lava colors - red to orange to yellow based on audio and position
    float blobHue = 0 + (audioLevel * 0.5f) + (i * 10); // Red to orange spectrum
    if (blobHue > 60) blobHue = 60; // Cap at yellow
    
    float brightness = 0.8f + (audioLevel / 200.0f);
    brightness = constrain(brightness, 0.6f, 1.0f);
    
    uint16_t blobColor = getColorFromHSV(blobHue, 1.0f, brightness);
    
    // Draw filled circle for blob
    M5.Lcd.fillCircle(x, y, radius, blobColor);
    
    // Add bright center for depth
    if (radius > 5) {
      uint16_t centerColor = getColorFromHSV(blobHue, 0.7f, 1.0f);
      M5.Lcd.fillCircle(x, y, radius/2, centerColor);
    }
    
    // Add small highlight for 3D effect
    if (radius > 8) {
      M5.Lcd.fillCircle(x - radius/3, y - radius/3, radius/4, WHITE);
    }
  }
  
  // Add rising bubbles effect with strong audio
  if (audioLevel > 100 && random(100) < 20) {
    // Draw small rising bubble
    int bubbleX = random(20, TFT_WIDTH - 20);
    int bubbleY = random(TFT_HEIGHT - 30, TFT_HEIGHT - 10);
    uint16_t bubbleColor = getColorFromHSV(30, 0.8f, 0.9f); // Orange bubble
    M5.Lcd.fillCircle(bubbleX, bubbleY, 3, bubbleColor);
  }
  
  hue += 0.2f;
  if (hue >= 360) hue = 0;
}

// Aurora Borealis - Wavy northern lights patterns
void showAuroraBorealis() {
  static float phase = 0;
  static float hue = 180; // Start with blue-green
  static float waveOffset = 0;
  static bool initialized = false;
  
  if (!initialized) {
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio influence on aurora behavior
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  
  float waveIntensity = 1.0f + constrain(audioLevel / 80.0f, 0, 3.0f);
  float colorShift = constrain(audioLevel / 100.0f, 0, 60.0f); // Shift hue with audio
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Aurora Borealis");
  }
  
  // Draw multiple aurora layers
  for (int layer = 0; layer < 4; layer++) {
    float layerPhase = phase + layer * 0.5f;
    float layerHeight = 30 + layer * 15; // Different heights for each layer
    float amplitude = 15 * waveIntensity * (1.0f - layer * 0.2f); // Decreasing amplitude
    
    // Aurora colors: Blue-green to purple-pink spectrum
    float layerHue = hue + colorShift + layer * 30;
    if (layerHue >= 360) layerHue -= 360;
    
    float brightness = 0.8f - layer * 0.15f; // Fade upper layers
    brightness = constrain(brightness, 0.3f, 1.0f);
    
    uint16_t auroraColor = getColorFromHSV(layerHue, 0.8f, brightness);
    
    // Draw wavy aurora band
    uint16_t lastY = layerHeight + amplitude * sin(layerPhase);
    for (int x = 1; x < TFT_WIDTH - 1; x++) {
      float frequency = 0.03f + layer * 0.005f; // Different frequencies per layer
      uint16_t currentY = layerHeight + amplitude * sin(x * frequency + layerPhase);
      
      // Keep within screen bounds
      currentY = constrain(currentY, 25, TFT_HEIGHT - 15);
      lastY = constrain(lastY, 25, TFT_HEIGHT - 15);
      
      // Draw multiple lines for thickness and glow effect
      for (int thickness = -2; thickness <= 2; thickness++) {
        int drawY = currentY + thickness;
        int drawLastY = lastY + thickness;
        
        // Better bounds checking - ensure both points are well within screen
        drawY = constrain(drawY, 25, TFT_HEIGHT - 15);
        drawLastY = constrain(drawLastY, 25, TFT_HEIGHT - 15);
        
        // Fade effect for glow
        float glowBrightness = brightness * (1.0f - abs(thickness) * 0.3f);
        uint16_t glowColor = getColorFromHSV(layerHue, 0.8f, glowBrightness);
        
        M5.Lcd.drawLine(x - 1, drawLastY, x, drawY, glowColor);
      }
      lastY = currentY;
    }
  }
  
  // Add sparkle effects for strong audio
  if (audioLevel > 80) {
    for (int i = 0; i < 5; i++) {
      int sparkleX = random(10, TFT_WIDTH - 10);
      int sparkleY = random(20, TFT_HEIGHT / 2);
      float sparkleHue = hue + colorShift + random(0, 60);
      if (sparkleHue >= 360) sparkleHue -= 360;
      
      uint16_t sparkleColor = getColorFromHSV(sparkleHue, 1.0f, 1.0f);
      M5.Lcd.fillRect(sparkleX - 1, sparkleY - 1, 2, 2, sparkleColor);
    }
  }
  
  phase += 0.05f + audioLevel * 0.001f; // Audio affects wave speed
  hue += 0.3f; // Slow color cycling
  if (hue >= 360) hue = 0;
}

// Kaleidoscope - Rotating geometric patterns
void showKaleidoscope() {
  static float rotation = 0;
  static float hue = 0;
  static bool initialized = false;
  
  if (!initialized) {
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio influence on rotation and complexity
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  
  float rotationSpeed = 0.02f + constrain(audioLevel / 200.0f, 0, 0.08f);
  float patternComplexity = 1.0f + constrain(audioLevel / 100.0f, 0, 2.0f);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(MAGENTA);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Kaleidoscope");
  }
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Draw multiple kaleidoscope segments (6-fold symmetry)
  for (int segment = 0; segment < 6; segment++) {
    float segmentAngle = (segment * 60) * PI / 180 + rotation;
    
    // Draw geometric patterns in each segment
    for (int pattern = 0; pattern < 3; pattern++) {
      float patternAngle = segmentAngle + pattern * 0.5f * patternComplexity;
      float radius = 20 + pattern * 15 + audioLevel * 0.1f;
      radius = constrain(radius, 10, 60);
      
      // Calculate pattern positions
      float x1 = centerX + cos(patternAngle) * radius;
      float y1 = centerY + sin(patternAngle) * radius;
      
      float x2 = centerX + cos(patternAngle + PI/3) * radius * 0.7f;
      float y2 = centerY + sin(patternAngle + PI/3) * radius * 0.7f;
      
      float x3 = centerX + cos(patternAngle - PI/3) * radius * 0.7f;
      float y3 = centerY + sin(patternAngle - PI/3) * radius * 0.7f;
      
      // Colors based on audio and pattern
      float patternHue = hue + segment * 60 + pattern * 30 + audioLevel * 0.5f;
      while (patternHue >= 360) patternHue -= 360;
      
      float brightness = 0.8f - pattern * 0.2f + (audioLevel / 200.0f);
      brightness = constrain(brightness, 0.4f, 1.0f);
      
      uint16_t patternColor = getColorFromHSV(patternHue, 1.0f, brightness);
      
      // Draw triangular pattern
      if (x1 >= 0 && x1 < TFT_WIDTH && y1 >= 12 && y1 < TFT_HEIGHT) {
        M5.Lcd.drawLine(x1, y1, x2, y2, patternColor);
        M5.Lcd.drawLine(x2, y2, x3, y3, patternColor);
        M5.Lcd.drawLine(x3, y3, x1, y1, patternColor);
        
        // Fill center with bright dot
        M5.Lcd.fillCircle(x1, y1, 2, patternColor);
      }
    }
    
    // Add radial lines for more complexity
    if (audioLevel > 50) {
      for (int radial = 0; radial < 2; radial++) {
        float radialAngle = segmentAngle + radial * PI/6;
        float startRadius = 10;
        float endRadius = 40 + audioLevel * 0.2f;
        
        float startX = centerX + cos(radialAngle) * startRadius;
        float startY = centerY + sin(radialAngle) * startRadius;
        float endX = centerX + cos(radialAngle) * endRadius;
        float endY = centerY + sin(radialAngle) * endRadius;
        
        float radialHue = hue + segment * 60 + 180; // Complementary colors
        while (radialHue >= 360) radialHue -= 360;
        
        uint16_t radialColor = getColorFromHSV(radialHue, 0.8f, 0.6f);
        M5.Lcd.drawLine(startX, startY, endX, endY, radialColor);
      }
    }
  }
  
  // Central bright spot
  uint16_t centerColor = getColorFromHSV(hue, 1.0f, 1.0f);
  M5.Lcd.fillCircle(centerX, centerY, 3 + audioLevel * 0.05f, centerColor);
  
  rotation += rotationSpeed;
  hue += 1.0f + audioLevel * 0.02f; // Audio affects color cycling speed
  if (hue >= 360) hue = 0;
}

// Particle Tornado - Swirling vortex of colored particles
void showParticleTornado() {
  static float particles[100][6]; // x, y, vx, vy, life, angle
  static bool initialized = false;
  static float tornadoRotation = 0;
  static float hue = 0;
  
  if (!initialized) {
    for (int i = 0; i < 100; i++) {
      particles[i][4] = -1; // inactive
    }
    initialized = true;
  }
  
  int bands[8];
  calculateAudioBands(bands);
  
  // Calculate audio influence on tornado behavior
  int audioLevel = 0;
  for (int i = 0; i < 8; i++) {
    if (bands[i] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[i]);
      if (log2 > -AMPLIFIER) {
        audioLevel += ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
  }
  
  float tornadoStrength = 1.0f + constrain(audioLevel / 80.0f, 0, 4.0f);
  float spawnRate = constrain(audioLevel / 50.0f, 0.2f, 1.0f);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Particle Tornado");
  }
  int centerX = TFT_WIDTH / 2;
  int centerY = TFT_HEIGHT / 2;
  
  // Spawn new particles from the bottom
  if (random(100) < spawnRate * 100) {
    for (int i = 0; i < 100; i++) {
      if (particles[i][4] < 0) {
        // Start particles at bottom with slight randomness
        particles[i][0] = centerX + random(-20, 20); // x
        particles[i][1] = TFT_HEIGHT - 5;            // y
        particles[i][2] = 0;                         // vx (will be calculated)
        particles[i][3] = 0;                         // vy (will be calculated)
        particles[i][4] = random(80, 150);           // life
        particles[i][5] = random(0, 628) / 100.0f;   // angle (0 to 2π)
        break;
      }
    }
  }
  
  // Update and draw particles
  for (int i = 0; i < 100; i++) {
    if (particles[i][4] > 0) {
      float x = particles[i][0];
      float y = particles[i][1];
      float life = particles[i][4];
      
      // Calculate distance from center
      float dx = x - centerX;
      float dy = y - centerY;
      float distance = sqrt(dx * dx + dy * dy);
      
      // Tornado physics - spiral upward motion
      float angle = atan2(dy, dx) + tornadoRotation * tornadoStrength;
      float spiralRadius = distance * 0.98f; // Gradually spiral inward
      
      // Move particle in spiral pattern
      particles[i][2] = cos(angle) * tornadoStrength * 0.5f; // vx
      particles[i][3] = -tornadoStrength * 0.8f;             // vy (upward)
      
      // Apply velocity
      particles[i][0] += particles[i][2];
      particles[i][1] += particles[i][3];
      
      // Add some randomness for organic look
      particles[i][0] += random(-1, 2) * 0.5f;
      particles[i][1] += random(-1, 2) * 0.3f;
      
      // Particle colors based on height and audio
      float heightRatio = 1.0f - (y / (float)TFT_HEIGHT);
      float particleHue = hue + heightRatio * 120 + audioLevel * 0.5f;
      while (particleHue >= 360) particleHue -= 360;
      
      float brightness = constrain(life / 100.0f, 0.3f, 1.0f);
      uint16_t particleColor = getColorFromHSV(particleHue, 1.0f, brightness);
      
      // Draw particle with size based on life
      int size = constrain(life / 40, 1, 3);
      if (x >= 0 && x < TFT_WIDTH && y >= 12 && y < TFT_HEIGHT) {
        if (size == 1) {
          M5.Lcd.drawPixel(x, y, particleColor);
        } else {
          M5.Lcd.fillRect(x - size/2, y - size/2, size, size, particleColor);
        }
        
        // Add trail effect for larger particles
        if (size > 1 && life > 50) {
          uint16_t trailColor = getColorFromHSV(particleHue, 1.0f, brightness * 0.5f);
          M5.Lcd.drawPixel(x - particles[i][2], y - particles[i][3], trailColor);
        }
      }
      
      // Age particle
      particles[i][4] -= 1;
      
      // Deactivate if too old or off screen
      if (particles[i][4] <= 0 || y < 10) {
        particles[i][4] = -1;
      }
    }
  }
  
  // Draw tornado center effect
  if (audioLevel > 50) {
    for (int radius = 2; radius <= 8; radius += 2) {
      float centerHue = hue + tornadoRotation * 100;
      while (centerHue >= 360) centerHue -= 360;
      
      uint16_t centerColor = getColorFromHSV(centerHue, 1.0f, 0.6f - radius * 0.1f);
      M5.Lcd.drawCircle(centerX, centerY, radius, centerColor);
    }
  }
  
  tornadoRotation += 0.05f + audioLevel * 0.002f;
  hue += 1.0f + audioLevel * 0.02f;
  if (hue >= 360) hue = 0;
}

// Sound Visualizer Bars - Modern frequency bars with glow effects
void showSoundVisualizerBars() {
  static float peakHold[8] = {0}; // Peak hold for each band
  static float peakDecay[8] = {0}; // Peak decay timers
  static float smoothBars[8] = {0}; // Smoothed bar values
  static float hue = 0;
  
  int bands[8];
  calculateAudioBands(bands);
  
  M5.Lcd.fillScreen(BLACK);
  
  // Draw title (if text enabled)
  if (showText) {
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.print("Sound Visualizer");
  }
  // Calculate bar dimensions
  int barWidth = (TFT_WIDTH - 16) / 8; // 8 bars with padding
  int maxBarHeight = TFT_HEIGHT - 20; // Leave space for title
  
  // Process each frequency band
  for (int band = 0; band < 8; band++) {
    int rawValue = 0;
    if (bands[band] > 0) {
      int log2 = FIX_LOG2<15-RSHIFT>(bands[band]);
      if (log2 > -AMPLIFIER) {
        rawValue = ((log2 + AMPLIFIER) * MAGNIFY) >> RSHIFT2;
      }
    }
    
    // Smooth the bar values for fluid animation
    float targetHeight = constrain(rawValue * 0.8f, 0, maxBarHeight);
    smoothBars[band] = smoothBars[band] * 0.85f + targetHeight * 0.15f;
    
    // Peak hold logic
    if (smoothBars[band] > peakHold[band]) {
      peakHold[band] = smoothBars[band];
      peakDecay[band] = 30; // Hold peak for 30 frames
    } else if (peakDecay[band] > 0) {
      peakDecay[band]--;
    } else {
      peakHold[band] = peakHold[band] * 0.95f; // Slow decay
    }
    
    // Calculate bar position
    int barX = 8 + band * barWidth + (barWidth - (barWidth - 2)) / 2;
    int actualBarWidth = barWidth - 2; // Leave 2px spacing
    
    // Draw main bar with gradient effect
    for (int y = 0; y < smoothBars[band]; y += 2) {
      float heightRatio = (float)y / maxBarHeight;
      
      // Color gradient: Blue (bottom) -> Green -> Yellow -> Red (top)
      float gradientHue;
      if (heightRatio < 0.33f) {
        gradientHue = 240 - heightRatio * 180; // Blue to cyan
      } else if (heightRatio < 0.66f) {
        gradientHue = 120; // Green
      } else {
        gradientHue = 60 - (heightRatio - 0.66f) * 60; // Yellow to red
      }
      
      // Add audio-reactive hue shift
      gradientHue += hue;
      while (gradientHue >= 360) gradientHue -= 360;
      
      float brightness = 0.6f + heightRatio * 0.4f;
      uint16_t barColor = getColorFromHSV(gradientHue, 1.0f, brightness);
      
      int barY = TFT_HEIGHT - y - 2;
      M5.Lcd.fillRect(barX, barY, actualBarWidth, 2, barColor);
    }
    
    // Draw glow effect around bars
    if (smoothBars[band] > 10) {
      // Left glow
      for (int glow = 1; glow <= 2; glow++) {
        if (barX - glow >= 0) {
          float glowHeight = smoothBars[band] * (1.0f - glow * 0.3f);
          float glowHue = hue + band * 45;
          while (glowHue >= 360) glowHue -= 360;
          
          uint16_t glowColor = getColorFromHSV(glowHue, 0.8f, 0.3f / glow);
          
          for (int y = 0; y < glowHeight; y += 3) {
            M5.Lcd.drawPixel(barX - glow, TFT_HEIGHT - y - 2, glowColor);
          }
        }
      }
      
      // Right glow
      for (int glow = 1; glow <= 2; glow++) {
        if (barX + actualBarWidth + glow < TFT_WIDTH) {
          float glowHeight = smoothBars[band] * (1.0f - glow * 0.3f);
          float glowHue = hue + band * 45;
          while (glowHue >= 360) glowHue -= 360;
          
          uint16_t glowColor = getColorFromHSV(glowHue, 0.8f, 0.3f / glow);
          
          for (int y = 0; y < glowHeight; y += 3) {
            M5.Lcd.drawPixel(barX + actualBarWidth + glow, TFT_HEIGHT - y - 2, glowColor);
          }
        }
      }
    }
    
    // Draw peak hold indicator
    if (peakHold[band] > 5) {
      float peakHue = hue + band * 45 + 180; // Complementary color
      while (peakHue >= 360) peakHue -= 360;
      
      uint16_t peakColor = getColorFromHSV(peakHue, 1.0f, 1.0f);
      int peakY = TFT_HEIGHT - peakHold[band] - 2;
      
      M5.Lcd.fillRect(barX, peakY - 1, actualBarWidth, 2, peakColor);
      
      // Peak glow
      if (peakDecay[band] > 0) {
        uint16_t peakGlowColor = getColorFromHSV(peakHue, 1.0f, 0.5f);
        M5.Lcd.fillRect(barX, peakY - 2, actualBarWidth, 1, peakGlowColor);
        M5.Lcd.fillRect(barX, peakY + 1, actualBarWidth, 1, peakGlowColor);
      }
    }
    
    // Draw frequency labels at bottom
    if (band % 2 == 0) { // Only show every other label to avoid crowding
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(barX, TFT_HEIGHT - 10);
      M5.Lcd.setTextSize(1);
      
      const char* freqLabels[] = {"63", "125", "250", "500", "1K", "2K", "4K", "8K"};
      M5.Lcd.print(freqLabels[band]);
    }
  }
  
  hue += 0.5f;
  if (hue >= 360) hue = 0;
}
