#include <M5Unified.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <AudioOutput.h>
#include <AudioOutputI2S.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>

namespace {

// Screen modes
enum ScreenMode {
  SCREEN_LIST,
  SCREEN_PLAY
};

// Play states
enum PlayState {
  STATE_STOPPED,
  STATE_PLAYING,
  STATE_PAUSED
};

// Audio output destinations
enum AudioOutputMode {
  OUT_INTERNAL_SPK,
  OUT_EXTERNAL_DAC
};

// Cyberpunk Palette (RGB565)
constexpr uint16_t kBg = 0x0810;       // Cyber Space Dark Violet (R=1, G=4, B=16)
constexpr uint16_t kGrid = 0x1030;     // Subtle cybernetic grid lines
constexpr uint16_t kPink = 0xF81F;     // Neon Pink (borders/tags)
constexpr uint16_t kCyan = 0x07FF;     // Glowing Neon Cyan (active track, highlights)
constexpr uint16_t kYellow = 0xFFE0;   // Neon Yellow (gauges, metrics)
constexpr uint16_t kWhite = 0xFFFF;    // Pure White
constexpr uint16_t kDim = 0x7BEF;      // Muted slate gray (inactive elements)

// SD Card SPI Pinout (G5, G4, G6, G7)
constexpr int kPinSD_CLK  = 5;
constexpr int kPinSD_MISO = 4;
constexpr int kPinSD_MOSI = 6;
constexpr int kPinSD_CS   = 7;

// 5-Way Tactile Switch Pins (GPIO G1, G2, G3, G8, G43)
constexpr int kPinSW_UP     = 1;
constexpr int kPinSW_DOWN   = 2;
constexpr int kPinSW_LEFT   = 3;
constexpr int kPinSW_RIGHT  = 8;
constexpr int kPinSW_CENTER = 43;

// I2S Headphone Jack Pins (BCLK=9, LRCK=10, DIN=44)
constexpr int kI2S_BCLK = 9;
constexpr int kI2S_LRCK = 10;
constexpr int kI2S_DIN  = 44;

struct TrackEntry {
  String name;
  String path;
};

// Application Global State
constexpr int kMaxFiles = 128;
TrackEntry g_entries[kMaxFiles];
int g_entry_count = 0;
int g_entry_index = 0;

ScreenMode g_screen = SCREEN_LIST;
volatile PlayState g_play_state = STATE_STOPPED;
uint8_t g_volume = 128; // 0 to 255

bool g_sd_mounted = false;
String g_status_msg = "IDLE";
uint32_t g_scroll_offset = 0;
float g_vinyl_angle = 0.0f;

AudioOutputMode g_output_mode = OUT_INTERNAL_SPK;
bool g_dac_auto_detected = false;

// Elapsed Playback Timing
uint32_t g_track_start_ms = 0;
uint32_t g_track_paused_ms = 0;
uint32_t g_track_elapsed_ms = 0;

// Thread safety lock for Core 0 audio decoder access
volatile bool g_audio_locked = false;
volatile bool g_finished_trigger = false;

// Audio engine pointer definitions
AudioFileSourceSD* g_file = nullptr;
AudioFileSourceID3* g_id3 = nullptr;
AudioGeneratorMP3* g_mp3 = nullptr;
AudioGeneratorWAV* g_wav = nullptr;
AudioOutput* g_out = nullptr; // Polymorphic base class pointer

// Double-buffering canvas sprite
M5Canvas g_canvas(&M5.Display);
TaskHandle_t g_audio_task_handle = nullptr;

// Custom Audio Output class that routes decoded I2S samples to M5Unified's speaker API
class AudioOutputM5Speaker : public AudioOutput
{
  public:
    AudioOutputM5Speaker(m5::Speaker_Class* m5sound, uint8_t virtual_sound_channel = 0)
    {
      _m5sound = m5sound;
      _virtual_ch = virtual_sound_channel;
    }
    virtual ~AudioOutputM5Speaker(void) {};
    virtual bool begin(void) override { return true; }
    virtual bool ConsumeSample(int16_t sample[2]) override
    {
      if (_tri_buffer_index < tri_buf_size)
      {
        _tri_buffer[_tri_index][_tri_buffer_index  ] = sample[0];
        _tri_buffer[_tri_index][_tri_buffer_index+1] = sample[1];
        _tri_buffer_index += 2;

        return true;
      }

      flush();
      return false;
    }
    virtual void flush(void) override
    {
      if (_tri_buffer_index)
      {
        int hz = (this->hertz > 0) ? this->hertz : 44100;
        _m5sound->playRaw(_tri_buffer[_tri_index], _tri_buffer_index, hz, true, 1, _virtual_ch);
        _tri_index = _tri_index < 2 ? _tri_index + 1 : 0;
        _tri_buffer_index = 0;
      }
    }
    virtual bool stop(void) override
    {
      flush();
      _m5sound->stop(_virtual_ch);
      return true;
    }

    const int16_t* getBuffer(void) const { return _tri_buffer[(_tri_index + 2) % 3]; }

  protected:
    m5::Speaker_Class* _m5sound;
    uint8_t _virtual_ch;
    static constexpr size_t tri_buf_size = 1536;
    int16_t _tri_buffer[3][tri_buf_size];
    size_t _tri_buffer_index = 0;
    size_t _tri_index = 0;
};

// Probes the PCM5102A DIN pin (GPIO 44) to auto-detect its presence.
// Uses internal weak pull-up. Connected PCM5102A internal pull-down pulls it LOW.
bool detectExternalDAC() {
  pinMode(kI2S_DIN, INPUT_PULLUP);
  delay(15);
  bool detected = (digitalRead(kI2S_DIN) == LOW);
  pinMode(kI2S_DIN, INPUT); // Reset pin state
  return detected;
}

// Helper to determine file formats
bool isWavFile(const String& path) {
  String lower = path;
  lower.toLowerCase();
  return lower.endsWith(".wav");
}

bool isMp3File(const String& path) {
  String lower = path;
  lower.toLowerCase();
  return lower.endsWith(".mp3");
}

// Clean text scroller for long names
String getScrollText(const String& text, int max_chars, int frame_offset) {
  if (text.length() <= (size_t)max_chars) return text;
  String temp = text + "    //    " + text;
  int start = (frame_offset / 10) % (text.length() + 10);
  return temp.substring(start, start + max_chars);
}

// Convert milliseconds to formatted minutes and seconds MM:SS
String formatTime(uint32_t ms) {
  uint32_t seconds = ms / 1000;
  uint32_t minutes = seconds / 60;
  seconds %= 60;
  char buf[8];
  snprintf(buf, sizeof(buf), "%02u:%02u", minutes, seconds);
  return String(buf);
}

// Release and delete ESP8266Audio resources safely
void cleanupAudio() {
  g_audio_locked = true;
  
  if (g_mp3) {
    g_mp3->stop();
    delete g_mp3;
    g_mp3 = nullptr;
  }
  if (g_wav) {
    g_wav->stop();
    delete g_wav;
    g_wav = nullptr;
  }
  if (g_id3) {
    g_id3->close();
    delete g_id3;
    g_id3 = nullptr;
  }
  if (g_file) {
    g_file->close();
    delete g_file;
    g_file = nullptr;
  }
  if (g_out) {
    delete g_out;
    g_out = nullptr;
  }
  
  g_audio_locked = false;
}

// Stop audio playback completely
void stopTrack() {
  g_play_state = STATE_STOPPED;
  // Small delay to allow the audio task to step out of its loop safely
  delay(10);
  cleanupAudio();
  g_track_elapsed_ms = 0;
  g_status_msg = "STOPPED";
}

// Start audio track at current index
bool startTrack(const String& path) {
  stopTrack();
  
  g_audio_locked = true;
  
  g_file = new AudioFileSourceSD(path.c_str());
  if (!g_file || !g_file->isOpen()) {
    g_status_msg = "FILE OPEN FAIL";
    cleanupAudio();
    return false;
  }

  if (g_output_mode == OUT_EXTERNAL_DAC) {
    // Disable M5Unified's speaker to release I2S port 0
    M5.Speaker.end();
    
    // Initialize external I2S DAC output
    AudioOutputI2S* out_i2s = new AudioOutputI2S();
    out_i2s->SetPinout(kI2S_BCLK, kI2S_LRCK, kI2S_DIN);
    g_out = out_i2s;
  } else {
    // Enable M5Unified's speaker
    M5.Speaker.begin();
    M5.Speaker.setVolume(g_volume);
    
    // Route samples through the custom M5Speaker output class
    g_out = new AudioOutputM5Speaker(&M5.Speaker);
  }

  g_out->SetGain((float)g_volume / 255.0f * 2.0f); // map volume scale

  if (isMp3File(path)) {
    g_id3 = new AudioFileSourceID3(g_file);
    g_mp3 = new AudioGeneratorMP3();
    if (!g_mp3->begin(g_id3, g_out)) {
      g_status_msg = "MP3 INIT FAIL";
      cleanupAudio();
      return false;
    }
  } else if (isWavFile(path)) {
    g_wav = new AudioGeneratorWAV();
    if (!g_wav->begin(g_file, g_out)) {
      g_status_msg = "WAV INIT FAIL";
      cleanupAudio();
      return false;
    }
  } else {
    g_status_msg = "UNSUPPORTED FMT";
    cleanupAudio();
    return false;
  }

  g_play_state = STATE_PLAYING;
  g_track_start_ms = millis();
  g_track_elapsed_ms = 0;
  g_status_msg = "PLAYING";
  
  g_audio_locked = false;
  return true;
}

// Toggle play/pause
void togglePause() {
  if (g_play_state == STATE_PLAYING) {
    g_play_state = STATE_PAUSED;
    g_track_paused_ms = millis() - g_track_start_ms;
    g_status_msg = "PAUSED";
  } else if (g_play_state == STATE_PAUSED) {
    g_play_state = STATE_PLAYING;
    g_track_start_ms = millis() - g_track_paused_ms;
    g_status_msg = "PLAYING";
  }
}

// Volume sync
void updateVolume() {
  if (g_out) {
    float gain = (float)g_volume / 255.0f * 2.0f;
    g_out->SetGain(gain);
  }
  if (g_output_mode == OUT_INTERNAL_SPK) {
    M5.Speaker.setVolume(g_volume);
  }
  g_status_msg = "VOL: " + String(g_volume * 100 / 255) + "%";
}

// Advance to next track
void playNextTrack() {
  if (g_entry_count == 0) return;
  g_entry_index = (g_entry_index + 1) % g_entry_count;
  startTrack(g_entries[g_entry_index].path);
}

// Go to previous track
void playPrevTrack() {
  if (g_entry_count == 0) return;
  g_entry_index = (g_entry_index - 1 + g_entry_count) % g_entry_count;
  startTrack(g_entries[g_entry_index].path);
}

// Mount the SD card and find valid files
bool initSDCard() {
  SPI.begin(kPinSD_CLK, kPinSD_MISO, kPinSD_MOSI, kPinSD_CS);
  if (!SD.begin(kPinSD_CS, SPI, 4000000)) {
    g_sd_mounted = false;
    g_status_msg = "SD CARD NOT DETECTED";
    return false;
  }
  g_sd_mounted = true;
  return true;
}

// Search root dir for music files
void scanFiles() {
  g_entry_count = 0;
  g_entry_index = 0;
  
  if (!g_sd_mounted) return;

  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    g_status_msg = "DIR ROOT FAIL";
    return;
  }

  File f = root.openNextFile();
  while (f && g_entry_count < kMaxFiles) {
    String name = f.name();
    // Strip parent path if necessary
    int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.substring(slash + 1);

    if (name.length() > 0 && !f.isDirectory()) {
      if (isMp3File(name) || isWavFile(name)) {
        g_entries[g_entry_count].name = name;
        g_entries[g_entry_count].path = String("/") + name;
        g_entry_count++;
      }
    }
    f = root.openNextFile();
  }
  root.close();
  g_status_msg = "FOUND " + String(g_entry_count) + " TRACKS";
}

// Thread-safe audio background decoder running on Core 0
void audioTask(void* parameter) {
  while (true) {
    if (g_play_state == STATE_PLAYING && !g_audio_locked) {
      bool active = false;
      
      // Process chunks to keep buffer full without audio dropouts
      for (int i = 0; i < 8; i++) {
        if (g_audio_locked) break;
        
        if (g_mp3 && g_mp3->isRunning()) {
          if (!g_mp3->loop()) {
            g_mp3->stop();
          }
          active = true;
        } else if (g_wav && g_wav->isRunning()) {
          if (!g_wav->loop()) {
            g_wav->stop();
          }
          active = true;
        }
      }
      
      if (!active && !g_audio_locked) {
        g_play_state = STATE_STOPPED;
        g_finished_trigger = true; // request track transition on Core 1
      }
    }
    
    // Yield to avoid watchdog reset and let ESP core housekeeping complete
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// Draw base cyberpunk themed grid background
void drawGridAndDeck() {
  g_canvas.fillScreen(kBg);
  
  // Cybernetic grids
  for (int y = 20; y < 120; y += 22) {
    g_canvas.drawFastHLine(0, y, 240, kGrid);
  }
  for (int x = 20; x < 240; x += 30) {
    g_canvas.drawFastVLine(x, 18, 100, kGrid);
  }
  
  // Neon pink interface bounding frame
  g_canvas.drawFastHLine(0, 17, 240, kPink);
  g_canvas.drawFastHLine(0, 118, 240, kPink);
  
  // Deck header info
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kCyan, kBg);
  g_canvas.drawString("// DECK-AUDIO S3 v1.1", 6, 3);
  
  // Battery status readout
  int bat = M5.Power.getBatteryLevel();
  g_canvas.setTextDatum(top_right);
  g_canvas.setTextColor(kYellow, kBg);
  g_canvas.printf("BAT: %d%%", bat);
}

// Render list screen
void drawListScreen() {
  drawGridAndDeck();
  
  // Draw file menu panel on left
  g_canvas.drawRoundRect(4, 22, 164, 92, 4, kPink);
  
  if (g_entry_count == 0) {
    g_canvas.setTextDatum(middle_center);
    g_canvas.setTextColor(kPink, kBg);
    g_canvas.drawString("NO AUDIO FILES IN SD", 86, 68);
  } else {
    // List scrolling logic
    constexpr int visible_count = 5;
    int start = g_entry_index - visible_count / 2;
    if (start < 0) start = 0;
    int end = start + visible_count;
    if (end > g_entry_count) {
      end = g_entry_count;
      start = end - visible_count;
      if (start < 0) start = 0;
    }
    
    int y_pos = 26;
    for (int i = start; i < end; ++i) {
      bool selected = (i == g_entry_index);
      if (selected) {
        // glowing background selection highlight
        g_canvas.fillRoundRect(8, y_pos - 1, 156, 14, 3, 0x03FF); // dim neon cyan
        g_canvas.drawRoundRect(8, y_pos - 1, 156, 14, 3, kCyan);
        g_canvas.setTextColor(kWhite);
      } else {
        g_canvas.setTextColor(kDim);
      }
      
      String label = getScrollText(g_entries[i].name, 22, selected ? g_scroll_offset : 0);
      g_canvas.setTextDatum(top_left);
      g_canvas.drawString(label, 14, y_pos);
      y_pos += 17;
    }
  }
  
  // Draw Dashboard widget on the right side
  g_canvas.drawRoundRect(172, 22, 64, 92, 4, kCyan);
  
  g_canvas.setTextDatum(top_center);
  g_canvas.setTextColor(kPink);
  g_canvas.drawString("OUTPUT", 204, 26);
  
  g_canvas.setTextColor(kWhite);
  if (g_output_mode == OUT_EXTERNAL_DAC) {
    g_canvas.setTextColor(kCyan);
    g_canvas.drawString("EXT DAC", 204, 38);
  } else {
    g_canvas.setTextColor(kYellow);
    g_canvas.drawString("INT SPK", 204, 38);
  }
  
  g_canvas.setTextColor(kWhite);
  g_canvas.drawString("INDEX", 204, 52);
  g_canvas.setTextColor(kYellow);
  g_canvas.printf("%02d/%02d", g_entry_count > 0 ? (g_entry_index + 1) : 0, g_entry_count);
  
  // Volume level indicator vertical bar
  g_canvas.drawRect(188, 76, 32, 8, kDim);
  int vol_w = (g_volume * 28) / 255;
  g_canvas.fillRect(190, 78, vol_w, 4, kCyan);
  g_canvas.setTextDatum(top_center);
  g_canvas.setTextColor(kDim);
  g_canvas.drawString("VOL", 204, 86);
  
  // Bottom action hints
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kCyan);
  g_canvas.drawString("[PWR] OUT  [UP/DN] NAV  [L/R] VOL", 6, 122);
}

// Render "Now Playing" playback screen
void drawPlayScreen() {
  drawGridAndDeck();
  
  // Now playing widget panels
  // Left: Rotating Vinyl
  g_canvas.drawRoundRect(4, 22, 100, 92, 4, kPink);
  
  // Center of vinyl
  int cx = 54;
  int cy = 68;
  int radius = 32;
  
  g_canvas.drawCircle(cx, cy, radius, kDim);
  g_canvas.drawCircle(cx, cy, 12, kPink);
  g_canvas.drawCircle(cx, cy, 2, kCyan);
  
  // Rotating spokes
  if (g_play_state == STATE_PLAYING) {
    g_vinyl_angle += 0.05f;
    if (g_vinyl_angle >= 2.0f * PI) g_vinyl_angle -= 2.0f * PI;
  }
  
  for (int i = 0; i < 4; i++) {
    float angle = g_vinyl_angle + (i * PI / 2.0f);
    int x1 = cx + cos(angle) * radius;
    int y1 = cy + sin(angle) * radius;
    g_canvas.drawLine(cx, cy, x1, y1, i % 2 == 0 ? kCyan : kPink);
  }
  
  // Right: Metadata & Equalizer
  g_canvas.drawRoundRect(108, 22, 128, 92, 4, kCyan);
  
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kPink);
  g_canvas.drawString("TRACK INFO:", 114, 26);
  
  g_canvas.setTextColor(kWhite);
  String current_name = g_entry_count > 0 ? g_entries[g_entry_index].name : "NO STREAM ACTIVE";
  g_canvas.drawString(getScrollText(current_name, 16, g_scroll_offset), 114, 38);
  
  // State Tag
  g_canvas.drawRect(114, 52, 48, 12, kPink);
  g_canvas.setTextDatum(top_center);
  if (g_play_state == STATE_PLAYING) {
    g_canvas.setTextColor(kCyan);
    g_canvas.drawString("PLAYING", 138, 54);
  } else if (g_play_state == STATE_PAUSED) {
    g_canvas.setTextColor(kYellow);
    g_canvas.drawString("PAUSED", 138, 54);
  } else {
    g_canvas.setTextColor(kDim);
    g_canvas.drawString("STOPPED", 138, 54);
  }
  
  // Time readout
  g_canvas.setTextDatum(top_right);
  g_canvas.setTextColor(kYellow);
  g_canvas.drawString(formatTime(g_track_elapsed_ms), 230, 54);
  
  // Output status label
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kDim);
  if (g_output_mode == OUT_EXTERNAL_DAC) {
    g_canvas.drawString("OUT: EXT DAC", 114, 66);
  } else {
    g_canvas.drawString("OUT: INT SPK", 114, 66);
  }
  
  // Dynamic Spectrum Bar Visualizer (dancing equalizers)
  for (int i = 0; i < 6; i++) {
    int bar_h = 2;
    if (g_play_state == STATE_PLAYING) {
      bar_h = random(4, 24);
    }
    int bx = 114 + (i * 18);
    g_canvas.fillRect(bx, 105 - bar_h, 12, bar_h, kCyan);
    g_canvas.drawRect(bx, 105 - bar_h, 12, bar_h, kPink);
  }
  
  // Bottom hints
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kCyan);
  g_canvas.drawString("[CTR] PAUSE  [L/R] TRACK  [B] BACK", 6, 122);
}

// Read and process the 5-way joystick & physical button inputs
void processControls() {
  uint32_t now = millis();
  
  // Joystick values (LOW represents active press)
  bool up = (digitalRead(kPinSW_UP) == LOW);
  bool down = (digitalRead(kPinSW_DOWN) == LOW);
  bool left = (digitalRead(kPinSW_LEFT) == LOW);
  bool right = (digitalRead(kPinSW_RIGHT) == LOW);
  bool center = (digitalRead(kPinSW_CENTER) == LOW);
  
  // M5 onboard tactile controls
  bool btnA = M5.BtnA.wasPressed();
  bool btnB = M5.BtnB.wasPressed();
  bool btnPWR = M5.BtnPWR.wasPressed();
  
  // Throttle action limits
  static uint32_t last_dir_ms = 0;
  bool dir_allowed = (now - last_dir_ms > 180);
  
  static uint32_t last_center_ms = 0;
  bool center_allowed = (now - last_center_ms > 350);

  if (g_screen == SCREEN_LIST) {
    if (up && dir_allowed) {
      if (g_entry_count > 0) {
        g_entry_index = (g_entry_index - 1 + g_entry_count) % g_entry_count;
        g_scroll_offset = 0;
      }
      last_dir_ms = now;
    } else if (down && dir_allowed) {
      if (g_entry_count > 0) {
        g_entry_index = (g_entry_index + 1) % g_entry_count;
        g_scroll_offset = 0;
      }
      last_dir_ms = now;
    } else if (left && dir_allowed) {
      // Decrease Volume
      if (g_volume >= 16) g_volume -= 16;
      else g_volume = 0;
      updateVolume();
      last_dir_ms = now;
    } else if (right && dir_allowed) {
      // Increase Volume
      if (g_volume <= 239) g_volume += 16;
      else g_volume = 255;
      updateVolume();
      last_dir_ms = now;
    } else if ((center && center_allowed) || btnA) {
      // Play selected track
      if (g_entry_count > 0) {
        startTrack(g_entries[g_entry_index].path);
        g_screen = SCREEN_PLAY;
      }
      last_center_ms = now;
    } else if (btnB) {
      g_screen = SCREEN_PLAY;
    } else if (btnPWR) {
      // Toggle output destination dynamically
      g_output_mode = (g_output_mode == OUT_INTERNAL_SPK) ? OUT_EXTERNAL_DAC : OUT_INTERNAL_SPK;
      g_status_msg = (g_output_mode == OUT_EXTERNAL_DAC) ? "OUT: EXT DAC" : "OUT: INT SPK";
      
      // If a track is active, restart it with the new output channel
      if (g_play_state == STATE_PLAYING || g_play_state == STATE_PAUSED) {
        startTrack(g_entries[g_entry_index].path);
      }
    }
  } else if (g_screen == SCREEN_PLAY) {
    if (up && dir_allowed) {
      // Increase Volume
      if (g_volume <= 239) g_volume += 16;
      else g_volume = 255;
      updateVolume();
      last_dir_ms = now;
    } else if (down && dir_allowed) {
      // Decrease Volume
      if (g_volume >= 16) g_volume -= 16;
      else g_volume = 0;
      updateVolume();
      last_dir_ms = now;
    } else if (left && dir_allowed) {
      // Prev track
      playPrevTrack();
      g_scroll_offset = 0;
      last_dir_ms = now;
    } else if (right && dir_allowed) {
      // Next track
      playNextTrack();
      g_scroll_offset = 0;
      last_dir_ms = now;
    } else if ((center && center_allowed) || btnA) {
      togglePause();
      last_center_ms = now;
    } else if (btnB || btnPWR) {
      g_screen = SCREEN_LIST;
    }
  }
}

// Error screen if SD card is not mounted
void drawErrorScreen() {
  g_canvas.fillScreen(kBg);
  
  // Neon bounding lines
  g_canvas.drawFastHLine(0, 17, 240, kPink);
  g_canvas.drawFastHLine(0, 118, 240, kPink);
  g_canvas.drawRoundRect(10, 26, 220, 84, 6, kPink);
  
  g_canvas.setTextDatum(top_center);
  g_canvas.setTextColor(kPink, kBg);
  g_canvas.drawString("HARDWARE SD MOUNT FAILED", 120, 36);
  
  g_canvas.setTextColor(kWhite, kBg);
  g_canvas.drawString("1. CHECK EXTERNAL SD MODULE WIRING", 120, 56);
  g_canvas.drawString("2. PRESS CENTER BUTTON TO RETRY", 120, 72);
  
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kCyan, kBg);
  g_canvas.drawString("// ERROR SYSTEM DECK", 6, 3);
  g_canvas.drawString("RETRY...", 6, 122);
}

} // namespace

void setup() {
  auto cfg = M5.config();
  cfg.fallback_board = m5::board_t::board_M5StickS3;
  M5.begin(cfg);
  
  M5.Display.setRotation(1); // Set landscape
  M5.Display.setBrightness(180);
  
  // Power boost for external components connected to Grove port or header pins (5V)
  M5.Power.setExtOutput(true);
  
  // Configure switch pins as INPUT_PULLUP
  pinMode(kPinSW_UP, INPUT_PULLUP);
  pinMode(kPinSW_DOWN, INPUT_PULLUP);
  pinMode(kPinSW_LEFT, INPUT_PULLUP);
  pinMode(kPinSW_RIGHT, INPUT_PULLUP);
  pinMode(kPinSW_CENTER, INPUT_PULLUP);

  // Initialize double-buffering canvas sprite
  g_canvas.createSprite(240, 135);

  // Auto-detect external I2S DAC presence via DIN pin weak pull-down
  g_dac_auto_detected = detectExternalDAC();
  if (g_dac_auto_detected) {
    g_output_mode = OUT_EXTERNAL_DAC;
    g_status_msg = "EXT DAC DETECTED";
  } else {
    g_output_mode = OUT_INTERNAL_SPK;
    g_status_msg = "INT SPEAKER ACTIVE";
  }
  
  // Attempt to mount card
  initSDCard();
  if (g_sd_mounted) {
    scanFiles();
  }

  // Pinned task to Core 0 for real-time MP3 & WAV audio streaming
  xTaskCreatePinnedToCore(
    audioTask,            // Task function pointer
    "AudioTask",          // Task name
    10240,                // Stack size in bytes
    NULL,                 // Parameter
    1,                    // Task priority
    &g_audio_task_handle, // Task handle
    0                     // Pinned Core (0)
  );
}

void loop() {
  M5.update();
  
  // Increment marquee scroll text timing frame counter
  g_scroll_offset++;

  // Handle track finished transition
  if (g_finished_trigger) {
    g_finished_trigger = false;
    playNextTrack();
  }

  if (!g_sd_mounted) {
    // If SD card is failed, show error recovery console
    drawErrorScreen();
    g_canvas.pushSprite(0, 0);
    
    // Check for manual retry via joystick center press
    if (digitalRead(kPinSW_CENTER) == LOW || M5.BtnA.wasPressed()) {
      delay(200); // simple debounce
      if (initSDCard()) {
        scanFiles();
      }
    }
    delay(16);
    return;
  }

  // Update track active timer count
  if (g_play_state == STATE_PLAYING) {
    g_track_elapsed_ms = millis() - g_track_start_ms;
  }

  // Handle tactile inputs
  processControls();
  
  // Render frame
  if (g_screen == SCREEN_LIST) {
    drawListScreen();
  } else {
    drawPlayScreen();
  }
  
  // Push the completed frame buffer to physical LCD
  g_canvas.pushSprite(0, 0);
  
  // Limit to roughly 60 FPS
  delay(16);
}
