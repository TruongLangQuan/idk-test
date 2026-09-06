// Matrix Effect 101 - Universal M5StickC Plus1/Plus2 Port  
// FULL INTEGRATION: All 101 matrix effects with complete physics

#include <M5Unified.h>
#include <math.h>

// Universal M5StickC compatibility layer  
class GFXCompatibility {
public:
  int width() { return 135; }
  int height() { return 135; }
  
  void fillScreen(uint16_t color) { M5.Display.fillScreen(color); }
  void drawPixel(int16_t x, int16_t y, uint16_t color) { M5.Display.drawPixel(x, y, color); }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { M5.Display.drawLine(x0, y0, x1, y1, color); }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { M5.Display.drawRect(x, y, w, h, color); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { M5.Display.fillRect(x, y, w, h, color); }
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { M5.Display.drawCircle(x0, y0, r, color); }
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { M5.Display.fillCircle(x0, y0, r, color); }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { M5.Display.drawLine(x, y, x + w - 1, y, color); }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { M5.Display.drawLine(x, y, x, y + h - 1, color); }
  
  void setTextColor(uint16_t color) { M5.Display.setTextColor(color); }
  void setTextColor(uint16_t fg, uint16_t bg) { M5.Display.setTextColor(fg, bg); }
  void setTextSize(uint8_t size) { M5.Display.setTextSize(size); }
  void setCursor(int16_t x, int16_t y) { M5.Display.setCursor(x, y); }
  void print(const char* text) { M5.Display.print(text); }
  void print(char c) { M5.Display.print(c); }  // Add character support
  void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    M5.Display.print(buffer);
  }
  
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return M5.Display.color565(r, g, b); }
  void setRotation(uint8_t rotation) { M5.Display.setRotation(rotation); }
};

GFXCompatibility* gfx = new GFXCompatibility();

// Effect modes - 65 MATRIX VARIATIONS - NEW YEAR 2026! 🎉
enum EffectMode {
  MATRIX_CUSTOM = 0,    // Our custom matrix rain
  MATRIX_XSCREEN,       // Xscreensaver authentic matrix
  BINARY_RAIN,          // Binary rain (0s and 1s)
  GLMATRIX_3D,          // GLMatrix-inspired 3D effect
  MATRIX_MULTICOLOR,    // Multi-colored matrix rain
  MATRIX_SPEEDBURST,    // Speed burst - fast and slow columns
  MATRIX_NEONGREEN,     // Neon green gradient - bright to dark
  MATRIX_PULSE,         // Pulsing brightness (sine wave)
  MATRIX_GLITCH,        // Glitchy flickers and sparks
  MATRIX_BROKEN,        // Broken streams - pauses and skips
  MATRIX_RETRO,         // Retro amber/orange terminal style
  MATRIX_FIRE,          // Fire colors: red/orange/yellow
  MATRIX_ICE,           // Ice colors: blue/white/cyan  
  MATRIX_TOXIC,         // Toxic colors: sickly green/yellow
  MATRIX_CYBER,         // Cyberpunk: purple/pink/magenta
  MATRIX_STORM,         // Lightning flickers: white/blue/purple
  MATRIX_BLOOD,         // Dark red dripping effect
  MATRIX_GOLD,          // Golden rain: yellow/gold/amber
  MATRIX_VOID,          // Inverted: dark text on bright background
  MATRIX_PHANTOM,       // Ghost trails that fade upwards 👻
  MATRIX_ECHO,          // Temporal ghosting - frames persist 🔁
  MATRIX_WIND,          // Lateral column sway with wind 🌪️
  MATRIX_RIPPLE,        // Sine wave distortion field 🌊
  MATRIX_DEPTHFOG,      // Atmospheric fade - bottom foggy 🌫️
  MATRIX_WINDSTORM,     // Wind sway + storm colors ⛈️
  MATRIX_WINDFIRE,      // Wind sway + fire colors 🔥🌪️
  MATRIX_HEAVYWIND,     // 3x stronger wind effect 💨
  MATRIX_WINDICE,       // Wind sway + ice colors ❄️🌪️
  MATRIX_MICROWIND,     // Gentle breeze effect 🍃
  MATRIX_TIDAL,         // 3x stronger ripples 🌊🌊
  MATRIX_RIPPLEFIRE,    // Ripple waves + fire colors 🔥🌊
  MATRIX_VIBRATE,       // High-frequency small ripples 📳
  MATRIX_RIPPLESTORM,   // Ripple waves + storm colors ⚡🌊
  MATRIX_SLOWRIPPLE,    // Very slow hypnotic waves 🐌🌊
  MATRIX_DRIP_BLOOD,    // Viscous blood drips 🩸
  MATRIX_DRIP_HONEY,    // Thick honey drips 🍯
  MATRIX_DRIP_ACID,     // Corrosive acid drips ☢️
  MATRIX_DRIP_MERCURY,  // Heavy mercury drips 🌡️
  MATRIX_DRIP_WATER,    // Light water drips 💧
  MATRIX_FRACTURE_BINARY,    // Simple 1→2 column splits 💥
  MATRIX_FRACTURE_TRIPLE,    // Triple 1→3 splits ⚡
  MATRIX_FRACTURE_CASCADE,   // Chain reaction splits 🌊
  MATRIX_FRACTURE_ORGANIC,   // Natural tree-like splits 🌿
  MATRIX_FRACTURE_EXPLOSIVE, // Multi-scatter splits 💥
  MATRIX_MOLTEN,        // Liquid→fracture→re-merge cycles 🌋
  MATRIX_GLASS,         // Crystalline fractures with glints 💎
  MATRIX_GRAVITYWELL,   // Columns bend toward gravity points 🌌
  MATRIX_RESONANCE,     // Rhythmic sync/desync patterns 🎵
  MATRIX_BIOME,         // Multi-zone: liquid|fracture|storm 🌿
  MATRIX_PLASMA,        // Energy fields bend rain paths ⚡
  MATRIX_PLASMA_FIRE,   // Fire energy plasma fields 🔥⚡
  MATRIX_PLASMA_ICE,    // Frozen plasma fields ❄️⚡
  MATRIX_PLASMA_TOXIC,  // Radioactive plasma fields ☢️⚡
  MATRIX_PLASMA_STORM,  // Lightning plasma fields ⛈️⚡
  MATRIX_CASCADE,       // Fractures trigger chain reactions 🌊💥
  MATRIX_CASCADE_ICE,   // Brittle ice fractures ❄️💥
  MATRIX_CASCADE_FIRE,  // Explosive fire cascades 🔥💥
  MATRIX_CASCADE_TOXIC, // Corrosive acid cascades ☢️💥
  MATRIX_CASCADE_STORM, // Lightning chain reactions ⛈️💥
  MATRIX_ENTROPY,       // Order→chaos→collapse→rebirth cycle 🌀
  MATRIX_TIDE,          // Global breathing system - synchronized pulsing 🌊
  MATRIX_TIDE_FIRE,     // Fire-breathing tide - orange/red pulsing 🔥🌊
  MATRIX_TIDE_ICE,      // Ice-breathing tide - blue/white pulsing ❄️🌊
  MATRIX_TIDE_STORM,    // Storm-breathing tide - lightning pulsing ⛈️🌊
  MATRIX_TIDE_DEEP,     // Deep ocean tide - slow, massive waves 🌊🌊
  MATRIX_DRIFT,         // Natural order decay - columns slowly lose alignment 🌪️
  MATRIX_DRIFT_CHAOS,   // Fast chaotic drift - rapid alignment collapse 🌪️💥
  MATRIX_DRIFT_ORBIT,   // Orbital drift - columns circle invisible points 🌪️⭕
  MATRIX_DRIFT_GRAVITY, // Gravitational drift - columns attract/repel 🌪️🌌
  MATRIX_DRIFT_TIDE,    // Tidal drift - wave-like lateral motion 🌪️🌊
  MATRIX_RELIC,         // Ghost memory system - ancient columns refuse to disappear 👻
  MATRIX_RELIC_ANCIENT, // Very old, barely visible ghosts from distant past 🏺👻
  MATRIX_RELIC_RECENT,  // Fresh ghosts that died recently, more visible 💀👻
  MATRIX_RELIC_SWARM,   // Multiple ghost generations appear together 👻👻👻
  MATRIX_RELIC_ECHO,    // Ghost columns repeat their original death patterns 🔁👻
  MATRIX_SIGNAL,        // Columns briefly align into hidden patterns/symbols 🔍⚡
  MATRIX_SIGNAL_BINARY,  // Digital patterns: lines, grids, squares 🔍💻
  MATRIX_SIGNAL_ORGANIC, // Natural patterns: curves, spirals, waves 🔍🌿  
  MATRIX_SIGNAL_CRYPTIC, // Ancient symbols and mysterious glyphs 🔍📜
  MATRIX_SIGNAL_GLITCH,  // Corrupted patterns that flicker and distort 🔍⚡
  MATRIX_FIREWORKS,      // Pressure release - over-energized heads rupture and spray symbols 🎆
  MATRIX_FIREWORKS_FAN,  // Directional fan-shaped bursts 🎆🌪️
  MATRIX_FIREWORKS_CASCADE, // Chain reaction bursts trigger neighbors 🎆⛓️
  MATRIX_FIREWORKS_SLOWMO,  // Dramatic slow-motion explosive bursts 🎆⏰ 
  MATRIX_FIREWORKS_ICE,  // Ice crystal shards with brittle shattering 🎆❄️
  MATRIX_HISTORY_WEIGHT, // Spacetime memory - past paths bend new rain 🌌⏳
  MATRIX_HISTORY_FIRE,   // Fire spacetime - hot paths repel, cold paths attract 🔥🌌
  MATRIX_HISTORY_ICE,    // Frozen spacetime - brittle memory with ice crystal effects ❄️🌌
  MATRIX_HISTORY_STORM,  // Lightning spacetime - electric memory with storm colors ⚡🌌
  MATRIX_HISTORY_DEEP,   // Deep space - slow, massive gravitational effects with dark matter 🌌🌊
  MATRIX_RIPPLEWIND,     // Ripple waves + wind sway = organic flowing motion 🌊🌪️
  MATRIX_RIPPLEWIND_FIRE,   // Fire ripple waves + wind = burning ocean motion 🔥🌊🌪️
  MATRIX_RIPPLEWIND_ICE,    // Ice ripple waves + wind = frozen tidal currents ❄️🌊🌪️
  MATRIX_RIPPLEWIND_STORM,  // Electric ripple waves + wind = lightning storm seas ⚡🌊🌪️
  MATRIX_RIPPLEWIND_DEEP,   // Deep ripple waves + wind = abyssal ocean currents 🌊🌌🌪️
  MATRIX_RIPPLEFRACTURE,    // Ripple waves + column fracturing = shattering wave patterns 🌊💥
  MATRIX_FIREWORKSWIND,     // Firework particles blown by wind = swirling explosive trails 🎆🌪️
  MATRIX_FIREWORKSRIPPLE,   // Firework explosions create ripple shockwaves 🎆🌊
  MATRIX_FIREWORKSFRACTURE, // Firework explosions cause cascading fractures 🎆💥
  MATRIX_FIREWORKSENTROPY,  // Fireworks accelerate entropy decay cycles 🎆🌀
  MATRIX_TIDEWIND,          // Breathing wind currents - tide modulates wind patterns 🌊🌪️
  TOTAL_MODES = 101     // 101 modes - Beyond the century! 🎉
};

EffectMode currentMode = MATRIX_CUSTOM;
unsigned long lastTouch = 0;
const unsigned long touchDelay = 500;

// Auto-scroll functionality
bool autoScroll = false;
unsigned long lastModeSwitch = 0;
const unsigned long autoSwitchInterval = 30000; // 30 seconds
unsigned long buttonPressStart = 0;
bool buttonHeld = false;
const unsigned long longPressTime = 1000; // 1 second for long press

// Matrix Rain Configuration
#define MATRIX_COLS 30  // Adapted for M5StickC 135x135 (240/8 = 30)
#define MATRIX_ROWS 17  // Adapted for M5StickC 135x135 (135/8 = 17)  
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

// Matrix Rain Variables
struct MatrixColumn {
  int y;
  int speed;
  int length;
  char chars[MATRIX_ROWS];
  unsigned long lastUpdate;
};

MatrixColumn columns[MATRIX_COLS];

// Drip state tracking for viscous liquid effects
enum DripState {
  DRIP_NORMAL,    // Normal falling
  DRIP_STRETCHING, // Column stretching before drip
  DRIP_SNAPPING   // Quick snap motion
};

struct DripColumn {
  DripState state;
  int stretchLength;      // How much the column has stretched
  int normalSpeed;        // Base speed for this liquid
  int stretchThreshold;   // How far to stretch before snapping
  unsigned long stateStartTime; // When current state began
  bool hasDroplet;        // Does this column have a falling droplet?
  int dropletY;          // Position of detached droplet
  int dropletSpeed;      // Speed of falling droplet
};

DripColumn dripColumns[MATRIX_COLS];

// Fracture system for column splitting effects
#define MAX_CHILD_COLUMNS 8  // Maximum child columns per parent

struct ChildColumn {
  bool active;
  float x;                    // X position (can be fractional for smooth movement)
  int y;                     // Y position
  float xSpeed;              // Horizontal drift speed
  int ySpeed;                // Vertical fall speed
  int length;                // Trail length
  int generation;            // Split generation (0=original, 1=first split, etc.)
  unsigned long birthTime;   // When this child was created
  unsigned long lifespan;    // How long it should live (ms)
  char chars[MATRIX_ROWS];   // Character trail
};

struct FractureColumn {
  bool canSplit;             // Can this column split?
  int splitCount;            // How many times has it split?
  int maxSplits;             // Maximum splits allowed
  unsigned long lastSplit;   // When did it last split?
  ChildColumn children[MAX_CHILD_COLUMNS]; // Child columns from splits
  int activeChildren;        // Number of active child columns
};

FractureColumn fractureColumns[MATRIX_COLS];

// Advanced mode state tracking
struct AdvancedColumn {
  // MOLTEN mode state
  bool isMolten;
  unsigned long moltenStartTime;
  uint8_t moltenPhase; // 0=liquid, 1=fracture, 2=remerge
  
  // GLASS mode state  
  bool hasGlint;
  unsigned long glintTime;
  uint8_t glintIntensity;
  
  // GRAVITYWELL mode state
  float gravityX, gravityY;    // Gravity well position
  float bendAmount;            // How much this column bends
  
  // RESONANCE mode state
  float resonancePhase;        // Individual phase offset
  bool isResonant;             // Following group rhythm?
  unsigned long lastSync;      // Last synchronization time
  
  // BIOME mode state
  uint8_t biomeZone;           // 0=liquid, 1=storm, 2=fracture
};

AdvancedColumn advancedColumns[MATRIX_COLS];

// Plasma physics - invisible energy fields that bend column paths
#define MAX_PLASMA_NODES 4  // 4 invisible energy nodes

struct PlasmaNode {
  float x, y;              // Node position on screen
  float strength;          // Field strength (positive = attract, negative = repel)
  float pulsePhase;        // Pulsing energy phase
  float driftVx, driftVy;  // Slow drift velocity
  unsigned long birthTime; // When this node was created
  bool active;             // Whether this node is active
};

struct PlasmaColumn {
  float bendX;             // Current X bend offset
  float velocity;          // Bend velocity for smooth movement
  float lastForceX;        // Last frame's force for momentum
  unsigned long lastUpdate; // Last physics update time
};

PlasmaNode plasmaNodes[MAX_PLASMA_NODES];
PlasmaColumn plasmaColumns[MATRIX_COLS];

// Global plasma variables
unsigned long plasmaSystemTimer = 0;
float plasmaGlobalEnergy = 128.0;    // Global energy level affects all nodes
bool plasmaInitialized = false;

// === CASCADE CHAIN REACTION SYSTEM ===
#define MAX_CASCADE_SHOCKWAVES 8  // Maximum active shockwaves
#define CASCADE_SHOCK_RADIUS 40   // Shockwave influence radius
#define CASCADE_STRESS_THRESHOLD 100  // Stress level to trigger fracture

struct CascadeShockwave {
  bool active;
  float x, y;               // Shockwave center position
  float radius;             // Current shockwave radius  
  float intensity;          // Shock intensity (decays over time)
  unsigned long birthTime;  // When this shockwave was created
  unsigned long lifespan;   // How long this shockwave lasts
};

struct CascadeColumn {
  float stressLevel;        // Accumulated stress from nearby fractures
  bool hasBeenFractured;    // Prevent multiple fractures
  unsigned long lastStressTime;  // Last time stress was applied
  float stressDecayRate;    // How fast stress decays
};

// Global cascade arrays
CascadeShockwave cascadeShockwaves[MAX_CASCADE_SHOCKWAVES];
CascadeColumn cascadeColumns[MATRIX_COLS];

// Global cascade variables  
unsigned long cascadeSystemTimer = 0;
bool cascadeInitialized = false;
int activeCascadeShockwaves = 0;

// === ENTROPY LIFECYCLE SYSTEM ===
// Order → Chaos → Collapse → Rebirth cycle
enum EntropyPhase {
  ENTROPY_ORDER,      // Perfect structure phase
  ENTROPY_DECAY,      // Gradual degradation phase  
  ENTROPY_CHAOS,      // Full chaos phase
  ENTROPY_COLLAPSE,   // System collapse phase
  ENTROPY_REBIRTH     // Clean restart phase
};

struct EntropySystem {
  EntropyPhase currentPhase;
  unsigned long phaseStartTime;
  float chaosLevel;           // 0.0 = perfect order, 1.0 = pure chaos
  float decayRate;            // How fast order degrades
  unsigned long phaseDuration[5]; // Duration of each phase (ms)
  
  // Order tracking
  float baseSpeed;            // Original uniform speed
  int baseSpacing;            // Original uniform spacing
  uint16_t baseColor;         // Original uniform color
  
  // Chaos injection
  float speedVariation;       // Speed randomness amount
  float positionJitter;       // Position randomness
  float colorDrift;           // Color randomness
  int glitchFrequency;        // How often glitches occur
};

struct EntropyColumn {
  float targetSpeed;          // What speed this column wants to be
  float actualSpeed;          // Current speed (slowly approaches target)
  float positionOffset;       // Chaotic position offset
  float colorShift;           // Individual color drift
  unsigned long lastGlitch;   // Last glitch time
  bool isGlitching;          // Currently glitching?
};

// Global entropy system
EntropySystem entropySystem;
EntropyColumn entropyColumns[MATRIX_COLS];
bool entropyInitialized = false;

// === HISTORY WEIGHT SYSTEM ===
// Spacetime memory - past paths physically influence new rain
#define HEATMAP_WIDTH 20   // Low-res for performance
#define HEATMAP_HEIGHT 15  // 40x30 screen mapped to 20x15 heatmap
#define HISTORY_DECAY_RATE 0.985f
#define HISTORY_INFLUENCE_RADIUS 3.0f
#define HISTORY_FORCE_STRENGTH 2.0f

struct HistorySystem {
  float heatmap[HEATMAP_HEIGHT][HEATMAP_WIDTH];  // The "spacetime memory"
  float decayRate;                               // How fast history fades
  float influenceRadius;                         // How far history affects new rain
  float forceStrength;                          // How strong the deflection is
  unsigned long lastUpdate;                     // Physics timing
  float maxHeat;                                // Current maximum heat value
};

struct HistoryColumn {
  float deflectionX;        // Current sideways deflection
  float deflectionVelocity; // Deflection momentum  
  float lastForceX;         // Previous frame's force
  float mass;               // This column's contribution to spacetime
};

// Global history system
HistorySystem historySystem;
HistoryColumn historyColumns[MATRIX_COLS];
bool historyInitialized = false;

// === TIDE BREATHING SYSTEM ===
// Global synchronized pulsing system
struct TideSystem {
  float globalPhase;           // Master tide phase (0 to 2π)
  float frequency;             // Tide frequency (cycles per second)
  float amplitude;             // Tide amplitude (0.0 to 1.0)
  
  // Speed modulation
  float baseSpeed;             // Normal fall speed
  float speedMin, speedMax;    // Speed range during tide
  
  // Brightness modulation  
  uint8_t brightnessBase;      // Normal brightness
  uint8_t brightnessMin, brightnessMax; // Brightness range during tide
  
  // Visual effects
  float breathingIntensity;    // How strong the breathing effect is
  bool risingTide;             // Whether tide is currently rising
};

TideSystem tideSystem;

// === DRIFT DECAY SYSTEM ===
// Natural order decay - columns slowly lose alignment
struct DriftSystem {
  float driftAmount[MATRIX_COLS];      // Current drift offset for each column
  float driftVelocity[MATRIX_COLS];    // Drift speed for each column
  float maxDrift;                      // Maximum drift before reset
  float driftAcceleration;             // How fast drift accumulates
  unsigned long lastReset;             // Last time system reset to order
  unsigned long resetInterval;         // Time between order resets
  bool isResetting;                    // Currently resetting to order
  float resetProgress;                 // Reset animation progress (0.0 to 1.0)
  
  // Visual state
  float originalSpacing;               // Perfect column spacing
  float currentChaos;                  // Current chaos level (0.0 to 1.0)
  bool orderPhase;                     // Currently in ordered phase
};

DriftSystem driftSystem;

// === RELIC GHOST MEMORY SYSTEM ===
// Old columns refuse to disappear - haunted by leftover processes
struct RelicColumn {
  bool isActive;                       // Is this relic currently visible
  float x, y;                          // Relic position
  char character;                      // Ancient character
  uint8_t brightness;                  // Dim brightness (always low)
  float speed;                         // Very slow movement
  unsigned long birthTime;             // When this relic was born
  unsigned long deathTime;             // When it should finally fade
  uint8_t originalMode;                // What mode created this relic
  float opacity;                       // Fading opacity (0.0 to 1.0)
  
  // Ghost behavior
  bool isDrifting;                     // Slowly drifts sideways
  float driftDirection;                // Random drift direction
  float phase;                         // Individual animation phase
  bool isPulsing;                      // Ghostly pulsing effect
};

struct RelicSystem {
  RelicColumn relics[15];              // Maximum 15 ghost relics
  unsigned long lastSpawnTime;         // Last time a relic spawned
  unsigned long spawnInterval;         // Time between relic spawns
  float spawnChance;                   // Chance to spawn relic (0.0 to 1.0)
  
  // Memory bank - store old column data
  struct ColumnMemory {
    float x;
    char character;
    uint8_t mode;
    unsigned long timestamp;
  } memory[50];                        // Remember last 50 columns
  
  int memoryIndex;                     // Current memory write position
  int activeRelics;                    // Count of currently active relics
  
  // Visual parameters
  uint8_t maxBrightness;               // Maximum relic brightness (always dim)
  unsigned long minLifespan;           // Minimum time relics stay alive
  unsigned long maxLifespan;           // Maximum time relics stay alive
  
  // Haunting behavior
  bool isHaunted;                      // System is currently being haunted
  float hauntingIntensity;             // How strong the haunting is (0.0 to 1.0)
};

RelicSystem relicSystem;

// Signal Pattern Detection System - Creates brief moments of hidden order
struct SignalPattern {
  bool active;                         // Is pattern currently manifesting
  unsigned long startTime;             // When pattern began
  unsigned long duration;              // How long pattern lasts (brief!)
  uint8_t patternType;                // Which pattern to show
  float intensity;                     // How strong the pattern is (0.0 to 1.0)
  
  // Pattern parameters
  float centerX, centerY;             // Pattern center point
  float scale;                        // Pattern size scaling
  float rotation;                     // Pattern rotation angle
  bool dissolving;                    // Pattern is dissolving back to chaos
};

struct SignalSystem {
  SignalPattern currentPattern;
  unsigned long lastTriggerTime;      // Last time a pattern appeared
  unsigned long quietPeriod;          // Time between pattern manifestations
  float triggerThreshold;             // Randomness threshold to trigger pattern
  
  // Pattern library - simple shapes that briefly appear
  uint8_t availablePatterns;          // Number of different patterns
  bool patternEnabled[8];             // Which patterns are enabled
  
  // Visual parameters
  uint8_t signalBrightness;           // Brightness during pattern moments
  float fadeSpeed;                    // How quickly patterns dissolve
} signalSystem;

// === FIREWORKS PRESSURE RELEASE SYSTEM ===
// Over-energized column heads rupture and spray symbols sideways
#define MAX_FIREWORK_PARTICLES 30        // Maximum particles per burst
#define FIREWORK_GRAVITY 0.15f           // Constant downward acceleration
#define FIREWORK_DRAG 0.90f              // Air resistance factor

struct FireworkParticle {
  bool active;                           // Is this particle alive
  float x, y;                           // Current position (float for smooth motion)
  float vx, vy;                         // Velocity (vx=sideways, vy=vertical)
  uint8_t life;                         // Remaining lifespan (frames)
  uint8_t energy;                       // Brightness/intensity
  char character;                       // What glyph to display
  uint16_t color;                       // Particle color
  float scale;                          // Size multiplier for visual effect
};

struct FireworkSystem {
  FireworkParticle particles[MAX_FIREWORK_PARTICLES];
  int activeParticles;                  // Count of currently active particles
  bool burstActive;                     // Is there currently a burst happening
  unsigned long lastBurstTime;          // Last time a burst occurred
  unsigned long burstCooldown;          // Time between allowed bursts
  
  // Burst parameters
  float burstX, burstY;                 // Last burst center point
  uint8_t particlesPerBurst;            // How many particles per explosion
  float initialVelocityRange;           // Speed range for initial burst
  uint8_t particleLifespan;             // How long particles live (frames)
  
  // Trigger conditions
  float energyThreshold;                // Column energy needed to trigger burst
  float randomBurstChance;              // Random chance per frame to burst
  bool cascadeTriggerEnabled;           // Can other events trigger bursts
  
  // Visual settings
  uint16_t burstColor;                  // Primary burst color
  float gravityStrength;                // How strong gravity effect is
  float dragStrength;                   // How much drag/air resistance
};

FireworkSystem fireworkSystem;

// Global advanced mode variables
float globalResonanceFreq = 1.0;  // Master resonance frequency
unsigned long gravityWellTimer = 0;
float gravityWellX = 160;         // Center of screen initially 
float gravityWellY = 120;

// Broken streams state tracking
bool columnPaused[MATRIX_COLS];
unsigned long pauseStartTime[MATRIX_COLS];
int skipDistance[MATRIX_COLS];

bool touchPressed = false;
unsigned long lastPress = 0;
const unsigned long BUTTON_DEBOUNCE = 300;

// Character sets for different modes
const char matrixChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()+-=[]{}|;:,.<>?";
const char xscreenChars[] = "ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝ0123456789";
const char binaryChars[] = "01";

// GLMatrix 3D-style variables
struct GLMatrixStrip {
  float x, z;
  int stripLength;
  float fallSpeed;
  int glyphs[MATRIX_ROWS];
  float spinnerY;
  unsigned long lastUpdate;
  float brightness[MATRIX_ROWS];
  bool highlight[MATRIX_ROWS];
};

GLMatrixStrip glStrips[MATRIX_COLS];
float cameraAngle = 0.0;
float wavePhase = 0.0;

// New effect variables
float globalTime = 0.0;          // Global animation time counter
float windDirection = 0.0;       // Wind direction for MATRIX_WIND
float windSpeed = 1.0;           // Wind speed multiplier

// Matrix Storm - Multi-layer hybrid effect
#define STORM_DROPS 35
struct StormDrop {
  float x, y;
  float speed;
  int length;
  int charType;  // 0=katakana, 1=binary, 2=symbols, 3=glitch
  int colorType; // 0=green, 1=blue, 2=red, 3=rainbow
  unsigned long lastUpdate;
  char chars[20]; // Trail characters
  bool glitchMode;
  unsigned long glitchEnd;
};

StormDrop stormDrops[STORM_DROPS];
float stormTime = 0;
bool stormInitialized = false;

// 3D Sphere variables - inspired by gltrackball effects
struct Sphere3D {
  float rotX, rotY, rotZ;     // 3D rotation angles
  float dRotX, dRotY, dRotZ;  // Rotation speeds
  float radius;               // Sphere radius
  int centerX, centerY;       // Screen center
  float time;                 // Animation time
  unsigned long lastUpdate;
  int surfaceType;            // 0=wireframe, 1=solid, 2=textured, 3=disco
  uint16_t baseColor;         // Base sphere color
};

Sphere3D sphere;
bool sphereInitialized = false;

// Enhanced Wireframe Sphere variables
struct WireSphere {
  float rotX, rotY, rotZ;     // 3D rotation angles
  float dRotX, dRotY, dRotZ;  // Rotation speeds
  float radius;               // Sphere radius
  int centerX, centerY;       // Screen center
  float colorShift;           // HSV color shifting
  float pulsePhase;           // Pulsing animation
  unsigned long lastUpdate;
  int wireMode;               // Different wireframe styles
  float lineThickness;        // Animated line thickness
};

WireSphere wireSphere;
bool wireSphereInitialized = false;

// Spheremonics variables - morphing spherical harmonics
struct Spheremonics {
  float rotX, rotY, rotZ;     // 3D rotation angles
  float dRotX, dRotY, dRotZ;  // Rotation speeds
  int centerX, centerY;       // Screen center
  float colorShift;           // Color cycling
  unsigned long lastUpdate;
  unsigned long lastMorph;    // Last parameter change
  int m[8];                   // The 8 spherical harmonic parameters
  int targetM[8];             // Target values for smooth morphing
  float morphProgress;        // 0.0 to 1.0 morph interpolation
  int renderMode;             // 0=dots, 1=wireframe, 2=filled
  float scale;                // Size scaling
};

Spheremonics spheremonics;
bool spheremonicsInitialized = false;

// Dot Morph variables - dense point cloud spheremonics  
struct DotMorph {
  float rotX, rotY, rotZ;     // 3D rotation angles
  float dRotX, dRotY, dRotZ;  // Rotation speeds
  int centerX, centerY;       // Screen center
  float colorShift;           // Color cycling
  unsigned long lastUpdate;
  unsigned long lastMorph;    // Last parameter change
  int m[8];                   // The 8 spherical harmonic parameters
  int targetM[8];             // Target values for smooth morphing
  float morphProgress;        // 0.0 to 1.0 morph interpolation
  float scale;                // Size scaling
  int dotDensity;             // Point cloud density
  float pulseBrightness;      // Pulsing brightness effect
};

DotMorph dotMorph;
bool dotMorphInitialized = false;

// Wire Cube removed to save RAM

// Hypercube variables - 4D tesseract projection
struct Hypercube {
  float angleXY, angleXZ, angleYZ;  // 3D rotation angles
  float angleXW, angleYW, angleZW;  // 4D rotation angles
  float dAngleXY, dAngleXZ, dAngleYZ; // 3D rotation speeds
  float dAngleXW, dAngleYW, dAngleZW; // 4D rotation speeds
  int centerX, centerY;             // Screen center
  float observerZ;                  // 3D projection distance
  float scale;                      // Size scaling
  float colorShift;                 // Color cycling
  unsigned long lastUpdate;
  int renderStyle;                  // Different rendering styles
  float vertices4D[16][4];          // 16 4D vertices
  int projected[16][2];             // 2D projected points
  float depths[16];                 // Z-depths for visibility
};

Hypercube hypercube;
bool hypercubeInitialized = false;

// XJack variables - Simplified version like CYD project
float jackTime = 0;

// Abstractile variables - Simple tiled pattern effect
float abstractileTime = 0;

// Truchet variables - Curved and angular tile patterns
float truchetTime = 0;

// Binary Horizon variables - Binary data streams from horizon
float horizonTime = 0;

// Font Glide variables - Text characters gliding and morphing  
float glideTime = 0;

// Epicycle variables - Simple version (no complex structs needed)
float epicycleTime = 0;

// Utility functions
float distance(float x1, float y1, float x2, float y2) {
  return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

// HSV to RGB conversion for color shifting
uint16_t hsvToRgb565(float h, float s, float v) {
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
  } else if (h >= 180 && h < 135) {
    r = 0; g = x; b = c;
  } else if (h >= 135 && h < 300) {
    r = x; g = 0; b = c;
  } else {
    r = c; g = 0; b = x;
  }
  
  int ri = (int)((r + m) * 255);
  int gi = (int)((g + m) * 255);
  int bi = (int)((b + m) * 255);
  
  return gfx->color565(ri, gi, bi);
}

// Forward declarations
void initGLMatrix();
void initMatrixStorm();
void drawMatrixStorm();
void updateDripColumn(int col, unsigned long currentTime);
void drawDripColumn(int col);
uint16_t getDripColor(int intensity, bool isHead);
void initFractureColumn(int col);
void updateFractureColumn(int col, unsigned long currentTime);
void splitColumn(int col, int splitType);
void drawFractureColumn(int col);
uint16_t getFractureColor(int intensity, bool isHead);
void initAdvancedColumn(int col);
void updateAdvancedColumn(int col, unsigned long currentTime);
void drawAdvancedColumn(int col);
uint16_t getAdvancedColor(int intensity, bool isHead, int col);
void initPlasmaSystem();
void initPlasmaColumn(int col);
void updatePlasmaSystem(unsigned long currentTime);
void updatePlasmaColumn(int col, unsigned long currentTime);
void drawPlasmaColumn(int col);
float calculatePlasmaForce(int col, float colX, float colY);
uint16_t getPlasmaColor(int intensity, bool isHead, EffectMode plasmaType);
void getPlasmaParameters(EffectMode plasmaType, float* energyMult, float* forceMult, float* sparkChance);

// === CASCADE CHAIN REACTION FUNCTIONS ===
void initCascadeSystem();
void initCascadeColumn(int col);
void updateCascadeSystem(unsigned long currentTime);
void updateCascadeColumn(int col, unsigned long currentTime);
void drawCascadeColumn(int col);
void triggerCascadeShockwave(float x, float y, float intensity);
void applyCascadeStress(int col, float stress);
bool shouldColumnFracture(int col);
void createCascadeFracture(int col);
void getCascadeParameters(EffectMode cascadeType, float* stressThreshold, float* shockRadius, int* shockLifespan, float* stressDecay);
uint16_t getCascadeColor(int col, int intensity, bool isHead, EffectMode cascadeType);

// === ENTROPY LIFECYCLE FUNCTIONS ===
void initEntropySystem();
void initEntropyColumn(int col);
void updateEntropySystem(unsigned long currentTime);
void updateEntropyColumn(int col, unsigned long currentTime);
uint16_t getEntropyColor(int intensity, bool isHead);

// === TIDE BREATHING FUNCTIONS ===
void initTideSystem();
void updateTideSystem(unsigned long currentTime);
float getTideSpeedMultiplier();
uint8_t getTideBrightness();

// === DRIFT DECAY FUNCTIONS ===
void initDriftSystem();
void updateDriftSystem(unsigned long currentTime);
float getDriftOffset(int col);

// === RELIC GHOST MEMORY FUNCTIONS ===
void initRelicSystem();
void updateRelicSystem(unsigned long currentTime);
void spawnRelic(float x, char character, uint8_t mode);
void drawRelics();
void rememberColumn(float x, char character, uint8_t mode);

// === SIGNAL PATTERN DETECTION FUNCTIONS ===
void initSignalSystem();
void updateSignalSystem(unsigned long currentTime);
void triggerSignalPattern();
void drawSignalPattern();

// === HISTORY WEIGHT SPACETIME FUNCTIONS ===
void initHistorySystem();
void initHistoryColumn(int col);
void updateHistorySystem(unsigned long currentTime);
void updateHistoryColumn(int col, unsigned long currentTime);
void recordColumnPath(int col, float x, float y);
void applyHistoryForces(int col, unsigned long currentTime);
uint16_t getHistoryColor(int intensity, bool isHead);

void initFireworkSystem();
void updateFireworkSystem(unsigned long currentTime);
void triggerFireworkBurst(float x, float y, uint16_t color);
void drawFireworkParticles();

void drawLyapunov();
void drawPlasmaEffect();
void initSphere3D();
void drawSphere3D();
void initWireSphere();
void drawWireSphere();
void initSpheremonics();
void drawSpheremonics();
void initDotMorph();
void drawDotMorph();
// Wire Cube functions removed
void initHypercube();
void drawHypercube();
void initXJack();
void drawXJack();
void initAbstractile();
void drawAbstractile();
void initTruchet();
void drawTruchet();
void initBinaryHorizon();
void drawBinaryHorizon();
void initFontglide();
void drawFontglide();
void initEpicycle();
void drawEpicycle();

void drawWireCircle3D(float centerY, float offsetY, float radius, float rx, float ry, float rz, uint16_t color);
void drawWireMeridian3D(float theta, float radius, float rx, float ry, float rz, uint16_t color);
void drawWireLine3D(float phi1, float theta1, float phi2, float theta2, float radius, float rx, float ry, float rz, uint16_t color);
void drawWireOrbit3D(float orbitAngle, float radius, float rx, float ry, float rz, uint16_t color);
void drawRotatedLine(float x1, float y1, float z1, float x2, float y2, float z2, float rx, float ry, float rz, uint16_t color);
void drawLine(int x1, int y1, int x2, int y2, uint16_t color);
void drawSpheremonicLine(float x1, float y1, float z1, float x2, float y2, float z2, 
                        float rx, float ry, float rz, uint16_t color);
void drawDottedLine(int x1, int y1, int x2, int y2, uint16_t color);

// Initialize a single matrix column based on current mode
void initColumn(int col) {
  columns[col].y = -random(10, 30);
  columns[col].speed = random(1, 4);
  columns[col].length = random(5, 15);
  columns[col].lastUpdate = millis();
  
  // Fill with characters based on current mode
  const char* charSet;
  int charSetSize;
  
  switch(currentMode) {
    case MATRIX_CUSTOM:
      charSet = matrixChars;
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_XSCREEN:
      charSet = xscreenChars;
      charSetSize = sizeof(xscreenChars) - 1;
      break;
    case BINARY_RAIN:
      charSet = binaryChars;
      charSetSize = sizeof(binaryChars) - 1;
      break;
    case GLMATRIX_3D:
      charSet = matrixChars;
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_MULTICOLOR:
      charSet = matrixChars; // Same characters as custom
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_SPEEDBURST:
      charSet = matrixChars; // Same characters as custom
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_NEONGREEN:
      charSet = matrixChars; // Same characters as custom
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_PULSE:
      charSet = matrixChars; // Same characters as custom
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_GLITCH:
      charSet = matrixChars; // Same characters as custom
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_BROKEN:
      charSet = matrixChars; // Same characters as custom
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_RETRO:
    case MATRIX_FIRE:
    case MATRIX_ICE:
    case MATRIX_TOXIC:
    case MATRIX_CYBER:
    case MATRIX_STORM:
    case MATRIX_BLOOD:
    case MATRIX_GOLD:
    case MATRIX_VOID:
      charSet = matrixChars; // Same characters for all new modes
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_PHANTOM:
    case MATRIX_ECHO:
    case MATRIX_WIND:
    case MATRIX_RIPPLE:
    case MATRIX_DEPTHFOG:
      charSet = matrixChars; // Same characters for new easy modes
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_WINDSTORM:
    case MATRIX_WINDFIRE:
    case MATRIX_HEAVYWIND:
    case MATRIX_WINDICE:
    case MATRIX_MICROWIND:
    case MATRIX_TIDAL:
    case MATRIX_RIPPLEFIRE:
    case MATRIX_VIBRATE:
    case MATRIX_RIPPLESTORM:
    case MATRIX_SLOWRIPPLE:
      charSet = matrixChars; // Same characters for wind/ripple variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_DRIP_BLOOD:
    case MATRIX_DRIP_HONEY:
    case MATRIX_DRIP_ACID:
    case MATRIX_DRIP_MERCURY:
    case MATRIX_DRIP_WATER:
      charSet = matrixChars; // Same characters for drip variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_FRACTURE_BINARY:
    case MATRIX_FRACTURE_TRIPLE:
    case MATRIX_FRACTURE_CASCADE:
    case MATRIX_FRACTURE_ORGANIC:
    case MATRIX_FRACTURE_EXPLOSIVE:
      charSet = matrixChars; // Same characters for fracture variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_MOLTEN:
    case MATRIX_GLASS:
    case MATRIX_GRAVITYWELL:
    case MATRIX_RESONANCE:
    case MATRIX_BIOME:
      charSet = matrixChars; // Same characters for advanced modes
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_PLASMA:
      charSet = matrixChars; // Same characters for plasma mode
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_PLASMA_FIRE:
    case MATRIX_PLASMA_ICE:
    case MATRIX_PLASMA_TOXIC:
    case MATRIX_PLASMA_STORM:
      charSet = matrixChars; // Same characters for plasma variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_CASCADE:
      charSet = matrixChars; // Same characters for cascade mode
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_CASCADE_ICE:
    case MATRIX_CASCADE_FIRE:
    case MATRIX_CASCADE_TOXIC:
    case MATRIX_CASCADE_STORM:
      charSet = matrixChars; // Same characters for cascade variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_FIREWORKS:
    case MATRIX_FIREWORKS_FAN:
    case MATRIX_FIREWORKS_CASCADE:
    case MATRIX_FIREWORKS_SLOWMO:
    case MATRIX_FIREWORKS_ICE:
      charSet = matrixChars; // Same characters for fireworks modes
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_HISTORY_WEIGHT:
      charSet = matrixChars; // Same characters for history weight mode
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_HISTORY_FIRE:
    case MATRIX_HISTORY_ICE:
    case MATRIX_HISTORY_STORM:
    case MATRIX_HISTORY_DEEP:
      charSet = matrixChars; // Same characters for all history variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_RIPPLEWIND:
      charSet = matrixChars; // Same characters for ripple-wind combo
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_RIPPLEWIND_FIRE:
    case MATRIX_RIPPLEWIND_ICE:
    case MATRIX_RIPPLEWIND_STORM:
    case MATRIX_RIPPLEWIND_DEEP:
      charSet = matrixChars; // Same characters for all ripplewind variations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_RIPPLEFRACTURE:
      charSet = matrixChars; // Same characters for ripple-fracture combo
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_FIREWORKSWIND:
    case MATRIX_FIREWORKSRIPPLE:
    case MATRIX_FIREWORKSFRACTURE:
    case MATRIX_FIREWORKSENTROPY:
      charSet = matrixChars; // Same characters for all fireworks combinations
      charSetSize = sizeof(matrixChars) - 1;
      break;
    case MATRIX_TIDEWIND:
      charSet = matrixChars; // Same characters for tide-wind combination
      charSetSize = sizeof(matrixChars) - 1;
      break;
    default:
      charSet = matrixChars;
      charSetSize = sizeof(matrixChars) - 1;
  }
  
  for(int i = 0; i < MATRIX_ROWS; i++) {
    columns[col].chars[i] = charSet[random(charSetSize)];
  }
}

// Initialize drip parameters for viscous liquid effects
void initDripColumn(int col) {
  dripColumns[col].state = DRIP_NORMAL;
  dripColumns[col].stretchLength = 0;
  dripColumns[col].stateStartTime = millis();
  dripColumns[col].hasDroplet = false;
  dripColumns[col].dropletY = 0;
  
  // Set liquid-specific parameters
  switch(currentMode) {
    case MATRIX_DRIP_BLOOD:
      dripColumns[col].normalSpeed = 120;     // Slow, viscous
      dripColumns[col].stretchThreshold = 25; // Stretches far
      dripColumns[col].dropletSpeed = 3;      // Heavy drops
      break;
    case MATRIX_DRIP_HONEY:
      dripColumns[col].normalSpeed = 200;     // Very slow
      dripColumns[col].stretchThreshold = 35; // Stretches very far
      dripColumns[col].dropletSpeed = 2;      // Very slow drops
      break;
    case MATRIX_DRIP_ACID:
      dripColumns[col].normalSpeed = 80;      // Medium speed
      dripColumns[col].stretchThreshold = 15; // Moderate stretch
      dripColumns[col].dropletSpeed = 5;      // Active drops
      break;
    case MATRIX_DRIP_MERCURY:
      dripColumns[col].normalSpeed = 60;      // Fast but heavy
      dripColumns[col].stretchThreshold = 10; // Snaps quickly
      dripColumns[col].dropletSpeed = 8;      // Heavy, fast drops
      break;
    case MATRIX_DRIP_WATER:
      dripColumns[col].normalSpeed = 40;      // Fast, light
      dripColumns[col].stretchThreshold = 8;  // Quick drops
      dripColumns[col].dropletSpeed = 10;     // Light, fast drops
      break;
    default:
      dripColumns[col].normalSpeed = 100;
      dripColumns[col].stretchThreshold = 20;
      dripColumns[col].dropletSpeed = 5;
  }
}

// Initialize fracture parameters for column splitting effects
void initFractureColumn(int col) {
  fractureColumns[col].canSplit = true;
  fractureColumns[col].splitCount = 0;
  fractureColumns[col].lastSplit = 0;
  fractureColumns[col].activeChildren = 0;
  
  // Set fracture-specific parameters
  switch(currentMode) {
    case MATRIX_FRACTURE_BINARY:
      fractureColumns[col].maxSplits = 1;  // Only split once
      break;
    case MATRIX_FRACTURE_TRIPLE:
      fractureColumns[col].maxSplits = 1;  // Only split once, but into 3
      break;
    case MATRIX_FRACTURE_CASCADE:
      fractureColumns[col].maxSplits = 3;  // Can split multiple times
      break;
    case MATRIX_FRACTURE_ORGANIC:
      fractureColumns[col].maxSplits = 2;  // Moderate splitting
      break;
    case MATRIX_FRACTURE_EXPLOSIVE:
      fractureColumns[col].maxSplits = 1;  // One big explosion
      break;
    default:
      fractureColumns[col].maxSplits = 1;
  }
  
  // Initialize all child columns as inactive
  for (int i = 0; i < MAX_CHILD_COLUMNS; i++) {
    fractureColumns[col].children[i].active = false;
  }
}

// Initialize advanced mode states
void initAdvancedColumn(int col) {
  AdvancedColumn* adv = &advancedColumns[col];
  
  // Reset all states
  adv->isMolten = false;
  adv->moltenStartTime = 0;
  adv->moltenPhase = 0;
  adv->hasGlint = false;
  adv->glintTime = 0;
  adv->glintIntensity = 0;
  adv->bendAmount = 0;
  adv->resonancePhase = random(628) / 100.0; // Random phase 0-2π
  adv->isResonant = false;
  adv->lastSync = 0;
  
  // Set gravity well attraction based on distance
  float dx = (col * CHAR_WIDTH) - gravityWellX;
  float dy = 120 - gravityWellY; // Assume middle Y
  float distance = sqrt(dx*dx + dy*dy);
  adv->gravityX = gravityWellX;
  adv->gravityY = gravityWellY;
  adv->bendAmount = max(0.1, 50.0 / (distance + 10)); // Inverse distance
  
  // Set biome zone based on column position
  if (col < MATRIX_COLS / 3) {
    adv->biomeZone = 0; // Left third: liquid
  } else if (col > 2 * MATRIX_COLS / 3) {
    adv->biomeZone = 2; // Right third: fracture  
  } else {
    adv->biomeZone = 1; // Center third: storm
  }
}

// Switch between effect modes
void switchMode() {
  currentMode = (EffectMode)((currentMode + 1) % TOTAL_MODES);
  
  // Clear screen
  gfx->fillScreen(BLACK);
  
  // Show mode name
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(10, 10);
  
  switch(currentMode) {
    case MATRIX_CUSTOM:
      gfx->print("Matrix Custom");
      break;
    case MATRIX_XSCREEN:
      gfx->print("Matrix XScreen");
      break;
    case BINARY_RAIN:
      gfx->print("Binary Rain");
      break;
    case GLMATRIX_3D:
      gfx->print("GLMatrix 3D");
      break;
    case MATRIX_MULTICOLOR:
      gfx->print("Matrix MultiColor");
      break;
    case MATRIX_SPEEDBURST:
      gfx->print("Matrix SpeedBurst");
      break;
    case MATRIX_NEONGREEN:
      gfx->print("Matrix NeonGreen");
      break;
    case MATRIX_PULSE:
      gfx->print("Matrix Pulse");
      break;
    case MATRIX_GLITCH:
      gfx->print("Matrix Glitch");
      break;
    case MATRIX_BROKEN:
      gfx->print("Matrix Broken");
      break;
    case MATRIX_RETRO:
      gfx->print("Matrix Retro");
      break;
    case MATRIX_FIRE:
      gfx->print("Matrix Fire");
      break;
    case MATRIX_ICE:
      gfx->print("Matrix Ice");
      break;
    case MATRIX_TOXIC:
      gfx->print("Matrix Toxic");
      break;
    case MATRIX_CYBER:
      gfx->print("Matrix Cyber");
      break;
    case MATRIX_STORM:
      gfx->print("Matrix Storm");
      break;
    case MATRIX_BLOOD:
      gfx->print("Matrix Blood");
      break;
    case MATRIX_GOLD:
      gfx->print("Matrix Gold");
      break;
    case MATRIX_VOID:
      gfx->print("Matrix Void");
      break;
    case MATRIX_PHANTOM:
      gfx->print("Matrix Phantom");
      break;
    case MATRIX_ECHO:
      gfx->print("Matrix Echo");
      break;
    case MATRIX_WIND:
      gfx->print("Matrix Wind");
      break;
    case MATRIX_RIPPLE:
      gfx->print("Matrix Ripple");
      break;
    case MATRIX_DEPTHFOG:
      gfx->print("Matrix DepthFog");
      break;
    case MATRIX_WINDSTORM:
      gfx->print("Matrix WindStorm");
      break;
    case MATRIX_WINDFIRE:
      gfx->print("Matrix WindFire");
      break;
    case MATRIX_HEAVYWIND:
      gfx->print("Matrix HeavyWind");
      break;
    case MATRIX_WINDICE:
      gfx->print("Matrix WindIce");
      break;
    case MATRIX_MICROWIND:
      gfx->print("Matrix MicroWind");
      break;
    case MATRIX_TIDAL:
      gfx->print("Matrix Tidal");
      break;
    case MATRIX_RIPPLEFIRE:
      gfx->print("Matrix RippleFire");
      break;
    case MATRIX_VIBRATE:
      gfx->print("Matrix Vibrate");
      break;
    case MATRIX_RIPPLESTORM:
      gfx->print("Matrix RippleStorm");
      break;
    case MATRIX_SLOWRIPPLE:
      gfx->print("Matrix SlowRipple");
      break;
    case MATRIX_DRIP_BLOOD:
      gfx->print("Matrix Drip Blood");
      break;
    case MATRIX_DRIP_HONEY:
      gfx->print("Matrix Drip Honey");
      break;
    case MATRIX_DRIP_ACID:
      gfx->print("Matrix Drip Acid");
      break;
    case MATRIX_DRIP_MERCURY:
      gfx->print("Matrix Drip Mercury");
      break;
    case MATRIX_DRIP_WATER:
      gfx->print("Matrix Drip Water");
      break;
    case MATRIX_FRACTURE_BINARY:
      gfx->print("Matrix Fracture Binary");
      break;
    case MATRIX_FRACTURE_TRIPLE:
      gfx->print("Matrix Fracture Triple");
      break;
    case MATRIX_FRACTURE_CASCADE:
      gfx->print("Matrix Fracture Cascade");
      break;
    case MATRIX_FRACTURE_ORGANIC:
      gfx->print("Matrix Fracture Organic");
      break;
    case MATRIX_FRACTURE_EXPLOSIVE:
      gfx->print("Matrix Fracture Explosive");
      break;
    case MATRIX_MOLTEN:
      gfx->print("Matrix Molten");
      break;
    case MATRIX_GLASS:
      gfx->print("Matrix Glass");
      break;
    case MATRIX_GRAVITYWELL:
      gfx->print("Matrix GravityWell");
      break;
    case MATRIX_RESONANCE:
      gfx->print("Matrix Resonance");
      break;
    case MATRIX_BIOME:
      gfx->print("Matrix Biome");
      break;
    case MATRIX_PLASMA:
      gfx->print("Matrix Plasma");
      break;
    case MATRIX_PLASMA_FIRE:
      gfx->print("Matrix Plasma Fire");
      break;
    case MATRIX_PLASMA_ICE:
      gfx->print("Matrix Plasma Ice");
      break;
    case MATRIX_PLASMA_TOXIC:
      gfx->print("Matrix Plasma Toxic");
      break;
    case MATRIX_PLASMA_STORM:
      gfx->print("Matrix Plasma Storm");
      break;
    case MATRIX_CASCADE:
      gfx->print("Matrix Cascade");
      break;
    case MATRIX_CASCADE_ICE:
      gfx->print("Matrix Cascade Ice");
      break;
    case MATRIX_CASCADE_FIRE:
      gfx->print("Matrix Cascade Fire");
      break;
    case MATRIX_CASCADE_TOXIC:
      gfx->print("Matrix Cascade Toxic");
      break;
    case MATRIX_CASCADE_STORM:
      gfx->print("Matrix Cascade Storm");
      break;
    case MATRIX_ENTROPY:
      gfx->print("Matrix Entropy");
      break;
    case MATRIX_TIDE:
      gfx->print("Matrix Tide");
      break;
    case MATRIX_TIDE_FIRE:
      gfx->print("Matrix Tide Fire");
      break;
    case MATRIX_TIDE_ICE:
      gfx->print("Matrix Tide Ice");
      break;
    case MATRIX_TIDE_STORM:
      gfx->print("Matrix Tide Storm");
      break;
    case MATRIX_TIDE_DEEP:
      gfx->print("Matrix Tide Deep");
      break;
    case MATRIX_DRIFT:
      gfx->print("Matrix Drift");
      break;
    case MATRIX_DRIFT_CHAOS:
      gfx->print("Matrix Drift Chaos");
      break;
    case MATRIX_DRIFT_ORBIT:
      gfx->print("Matrix Drift Orbit");
      break;
    case MATRIX_DRIFT_GRAVITY:
      gfx->print("Matrix Drift Gravity");
      break;
    case MATRIX_DRIFT_TIDE:
      gfx->print("Matrix Drift Tide");
      break;
    case MATRIX_RELIC:
      gfx->print("Matrix Relic");
      break;
    case MATRIX_RELIC_ANCIENT:
      gfx->print("Matrix Relic Ancient");
      break;
    case MATRIX_RELIC_RECENT:
      gfx->print("Matrix Relic Recent");
      break;
    case MATRIX_RELIC_SWARM:
      gfx->print("Matrix Relic Swarm");
      break;
    case MATRIX_RELIC_ECHO:
      gfx->print("Matrix Relic Echo");
      break;
    case MATRIX_SIGNAL:
      gfx->print("Matrix Signal");
      break;
    case MATRIX_SIGNAL_BINARY:
      gfx->print("Matrix Signal Binary");
      break;
    case MATRIX_SIGNAL_ORGANIC:
      gfx->print("Matrix Signal Organic");
      break;
    case MATRIX_SIGNAL_CRYPTIC:
      gfx->print("Matrix Signal Cryptic");
      break;
    case MATRIX_SIGNAL_GLITCH:
      gfx->print("Matrix Signal Glitch");
      break;
    case MATRIX_FIREWORKS:
      gfx->print("Matrix Fireworks");
      break;
    case MATRIX_FIREWORKS_FAN:
      gfx->print("Matrix Fireworks Fan");
      break;
    case MATRIX_FIREWORKS_CASCADE:
      gfx->print("Matrix Fireworks Cascade");
      break;
    case MATRIX_FIREWORKS_SLOWMO:
      gfx->print("Matrix Fireworks SlowMo");
      break;
    case MATRIX_FIREWORKS_ICE:
      gfx->print("Matrix Fireworks Ice");
      break;
    case MATRIX_HISTORY_WEIGHT:
      gfx->print("Matrix History Weight");
      break;
    case MATRIX_HISTORY_FIRE:
      gfx->print("Matrix History Fire");
      break;
    case MATRIX_HISTORY_ICE:
      gfx->print("Matrix History Ice");
      break;
    case MATRIX_HISTORY_STORM:
      gfx->print("Matrix History Storm");
      break;
    case MATRIX_HISTORY_DEEP:
      gfx->print("Matrix History Deep");
      break;
    case MATRIX_RIPPLEWIND:
      gfx->print("Matrix RippleWind");
      break;
    case MATRIX_RIPPLEWIND_FIRE:
      gfx->print("Matrix RippleWind Fire");
      break;
    case MATRIX_RIPPLEWIND_ICE:
      gfx->print("Matrix RippleWind Ice");
      break;
    case MATRIX_RIPPLEWIND_STORM:
      gfx->print("Matrix RippleWind Storm");
      break;
    case MATRIX_RIPPLEWIND_DEEP:
      gfx->print("Matrix RippleWind Deep");
      break;
    case MATRIX_RIPPLEFRACTURE:
      gfx->print("Matrix RippleFracture");
      break;
    case MATRIX_FIREWORKSWIND:
      gfx->print("Matrix Fireworks Wind");
      break;
    case MATRIX_FIREWORKSRIPPLE:
      gfx->print("Matrix Fireworks Ripple");
      break;
    case MATRIX_FIREWORKSFRACTURE:
      gfx->print("Matrix Fireworks Fracture");
      break;
    case MATRIX_FIREWORKSENTROPY:
      gfx->print("Matrix Fireworks Entropy");
      break;
    case MATRIX_TIDEWIND:
      gfx->print("Matrix Tide Wind");
      break;
  }
  
  delay(200);  // Quick flash instead of 1500ms
  gfx->fillScreen(BLACK);
  
  // Mode-specific initialization
  if (currentMode == GLMATRIX_3D) {
    initGLMatrix();
  } else {
    // Reinitialize all columns for regular matrix modes
    for(int i = 0; i < MATRIX_COLS; i++) {
      initColumn(i);
      columns[i].y = random(-100, gfx->height());
      // Reset broken streams state
      columnPaused[i] = false;
      pauseStartTime[i] = 0;
      skipDistance[i] = 0;
      
      // Initialize drip state for drip modes
      if (currentMode >= MATRIX_DRIP_BLOOD && currentMode <= MATRIX_DRIP_WATER) {
        initDripColumn(i);
      }
      
      // Initialize fracture state for fracture modes
      if (currentMode >= MATRIX_FRACTURE_BINARY && currentMode <= MATRIX_FRACTURE_EXPLOSIVE) {
        initFractureColumn(i);
      }
      
      // Initialize advanced state for advanced modes
      if (currentMode >= MATRIX_MOLTEN && currentMode <= MATRIX_BIOME) {
        initAdvancedColumn(i);
      }
      
      // Initialize plasma state for plasma mode
      if (currentMode == MATRIX_PLASMA) {
        initPlasmaColumn(i);
      }
      
      // Initialize plasma state for plasma variations  
      if (currentMode >= MATRIX_PLASMA_FIRE && currentMode <= MATRIX_PLASMA_STORM) {
        initPlasmaColumn(i);
      }
      
      // Initialize cascade state for cascade mode
      if (currentMode == MATRIX_CASCADE) {
        initCascadeColumn(i);
      }
      
      // Initialize cascade state for cascade variations
      if (currentMode >= MATRIX_CASCADE_ICE && currentMode <= MATRIX_CASCADE_STORM) {
        initCascadeColumn(i);
      }
      
      // Initialize entropy state for entropy mode
      if (currentMode == MATRIX_ENTROPY) {
        initEntropyColumn(i);
      }
    }
    
    // Initialize global plasma system
    if (currentMode == MATRIX_PLASMA) {
      initPlasmaSystem();
    }
    
    // Initialize plasma system for variations
    if (currentMode >= MATRIX_PLASMA_FIRE && currentMode <= MATRIX_PLASMA_STORM) {
      initPlasmaSystem();
    }
    
    // Initialize cascade system for cascade mode
    if (currentMode == MATRIX_CASCADE) {
      initCascadeSystem();
    }
    
    // Initialize cascade system for cascade variations
    if (currentMode >= MATRIX_CASCADE_ICE && currentMode <= MATRIX_CASCADE_STORM) {
      initCascadeSystem();
    }
    
    // Initialize entropy system for entropy mode
    if (currentMode == MATRIX_ENTROPY) {
      initEntropySystem();
    }
    
    // Initialize tide system for tide mode and variations
    if (currentMode >= MATRIX_TIDE && currentMode <= MATRIX_TIDE_DEEP) {
      initTideSystem();
    }
    
    // Initialize drift system for drift mode and variations
    if (currentMode >= MATRIX_DRIFT && currentMode <= MATRIX_DRIFT_TIDE) {
      initDriftSystem();
    }
    
    // Initialize relic system for relic mode and variations
    if (currentMode >= MATRIX_RELIC && currentMode <= MATRIX_RELIC_ECHO) {
      initRelicSystem();
    }
    
    // Initialize signal system for MATRIX_SIGNAL mode and variations
    if (currentMode >= MATRIX_SIGNAL && currentMode <= MATRIX_SIGNAL_GLITCH) {
      initSignalSystem();
    }
    
    // Initialize firework system for MATRIX_FIREWORKS mode and all variations including combos
    if ((currentMode >= MATRIX_FIREWORKS && currentMode <= MATRIX_FIREWORKS_ICE) ||
        (currentMode >= MATRIX_FIREWORKSWIND && currentMode <= MATRIX_FIREWORKSENTROPY)) {
      initFireworkSystem();
    }
    
    // Initialize history weight system for MATRIX_HISTORY_WEIGHT and variations
    if (currentMode >= MATRIX_HISTORY_WEIGHT && currentMode <= MATRIX_HISTORY_DEEP) {
      initHistorySystem();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Matrix Effect 101 - Universal M5StickC Plus1/Plus2 Port");
  
  // Initialize M5Unified - auto-detects Plus1 vs Plus2
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Auto-detect device type and report
  const char* deviceName;
  switch (M5.getBoard()) {
    case m5::board_t::board_M5StickCPlus:
      deviceName = "M5StickC Plus1.1";
      break;
    case m5::board_t::board_M5StickCPlus2:
      deviceName = "M5StickC Plus2";
      break;
    default:
      deviceName = "Unknown M5 Device";
      break;
  }
  
  Serial.printf("Detected: %s\n", deviceName);
  
  gfx->setRotation(3); // Landscape mode for both Plus1 and Plus2
  gfx->fillScreen(BLACK);
  
  Serial.printf("Display size: %dx%d\n", gfx->width(), gfx->height());
  
  // No touch setup needed for M5StickC - uses buttons
  // Matrix grid adapted for 135x135 display
  
  // Initialize matrix effects
  randomSeed(esp_random());
  for(int i = 0; i < MATRIX_COLS; i++) {
    initColumn(i);
    columns[i].y = random(-100, gfx->height()); // Spread initial positions
    // Initialize broken streams state
    columnPaused[i] = false;
    pauseStartTime[i] = 0;
    skipDistance[i] = 0;
  }
  initGLMatrix(); // Initialize GLMatrix data for 3D mode
  
  Serial.println("✅ Matrix Effect 101 - All 101 modes loaded!");
  Serial.println("PWR: Previous | B: Next | A: Auto-scroll");
  
  lastModeSwitch = millis();
}

// Draw drip column with stretching and droplet effects
void drawDripColumn(int col) {
  int x = col * CHAR_WIDTH;
  DripColumn* drip = &dripColumns[col];
  
  gfx->setTextSize(1);
  
  // Draw main column trail
  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    
    // Add stretch effect
    if (drip->state == DRIP_STRETCHING && i == columns[col].length - 1) {
      charY += drip->stretchLength; // Stretch the tail
    }
    
    if (charY < -CHAR_HEIGHT || charY > gfx->height()) continue;
    
    // Calculate color intensity with stretch effects
    int intensity = 255 - (i * 255 / columns[col].length);
    intensity = max(intensity, 15);
    
    // Modify intensity for stretching effect
    if (drip->state == DRIP_STRETCHING && i >= columns[col].length - 3) {
      intensity = intensity * (drip->stretchLength + 10) / 20; // Fade during stretch
    }
    
    uint16_t color = getDripColor(intensity, i == 0);
    
    // Clear previous position
    gfx->fillRect(x, charY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, BLACK);
    
    // Draw character
    gfx->setTextColor(color);
    gfx->setCursor(x, charY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
  }
  
  // Draw stretched connection if stretching
  if (drip->state == DRIP_STRETCHING && drip->stretchLength > 5) {
    int tailY = columns[col].y - ((columns[col].length - 1) * CHAR_HEIGHT);
    uint16_t stretchColor = getDripColor(100, false); // Dim connecting line
    
    // Draw vertical line for stretch
    for (int sy = tailY; sy < tailY + drip->stretchLength; sy += 2) {
      if (sy >= 0 && sy < gfx->height()) {
        gfx->drawPixel(x + CHAR_WIDTH/2, sy, stretchColor);
      }
    }
  }
  
  // Draw falling droplet
  if (drip->hasDroplet) {
    if (drip->dropletY >= 0 && drip->dropletY < gfx->height()) {
      uint16_t dropletColor = getDripColor(200, true); // Bright droplet
      
      // Clear previous droplet position
      gfx->fillRect(x, drip->dropletY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, BLACK);
      
      // Draw droplet (could be a character or just a dot)
      gfx->setTextColor(dropletColor);
      gfx->setCursor(x, drip->dropletY);
      gfx->print('.');  // Simple droplet representation
    }
  }
}

// Get color for drip effects based on current drip mode
uint16_t getDripColor(int intensity, bool isHead) {
  switch(currentMode) {
    case MATRIX_DRIP_BLOOD:
      // Blood: dark red to bright red
      if (isHead) return gfx->color565(intensity, 0, 0);
      return gfx->color565(intensity * 0.8, intensity * 0.1, 0);
      
    case MATRIX_DRIP_HONEY:
      // Honey: golden amber gradient
      if (isHead) return gfx->color565(intensity, intensity * 0.8, 0);
      return gfx->color565(intensity * 0.9, intensity * 0.6, intensity * 0.1);
      
    case MATRIX_DRIP_ACID:
      // Acid: toxic green with occasional sparks
      if (random(100) < 3) return gfx->color565(255, 255, 255); // Rare spark
      if (isHead) return gfx->color565(intensity * 0.3, intensity, intensity * 0.2);
      return gfx->color565(intensity * 0.2, intensity * 0.8, 0);
      
    case MATRIX_DRIP_MERCURY:
      // Mercury: metallic silver
      if (isHead) return gfx->color565(intensity, intensity, intensity);
      return gfx->color565(intensity * 0.8, intensity * 0.8, intensity * 0.9);
      
    case MATRIX_DRIP_WATER:
      // Water: clear blue
      if (isHead) return gfx->color565(intensity * 0.3, intensity * 0.8, intensity);
      return gfx->color565(0, intensity * 0.6, intensity * 0.9);
      
    default:
      return gfx->color565(0, intensity, 0); // Fallback green
  }
}

// Update drip physics for viscous liquid effects
void updateDripColumn(int col, unsigned long currentTime) {
  DripColumn* drip = &dripColumns[col];
  
  switch(drip->state) {
    case DRIP_NORMAL:
      // Normal falling with chance to start stretching
      columns[col].y += CHAR_HEIGHT;
      
      // Random chance to start stretching (different per liquid)
      if (random(200) == 0) { // 1 in 200 chance
        drip->state = DRIP_STRETCHING;
        drip->stretchLength = 0;
        drip->stateStartTime = currentTime;
      }
      break;
      
    case DRIP_STRETCHING:
      // Column stretches before snapping
      drip->stretchLength += 2;
      
      // Move very slowly while stretching
      if (currentTime - drip->stateStartTime > drip->normalSpeed * 3) {
        columns[col].y += CHAR_HEIGHT / 3; // Slower movement
        drip->stateStartTime = currentTime;
      }
      
      // Check if stretch threshold reached
      if (drip->stretchLength >= drip->stretchThreshold) {
        drip->state = DRIP_SNAPPING;
        drip->stateStartTime = currentTime;
        
        // Create droplet
        drip->hasDroplet = true;
        drip->dropletY = columns[col].y + (columns[col].length * CHAR_HEIGHT);
      }
      break;
      
    case DRIP_SNAPPING:
      // Quick snap back to normal
      columns[col].y += CHAR_HEIGHT * 2; // Fast snap movement
      drip->stretchLength = max(0, drip->stretchLength - 5);
      
      // Return to normal after brief snap
      if (currentTime - drip->stateStartTime > 200) { // 200ms snap
        drip->state = DRIP_NORMAL;
        drip->stretchLength = 0;
      }
      break;
  }
  
  // Update droplet if it exists
  if (drip->hasDroplet) {
    drip->dropletY += drip->dropletSpeed;
    
    // Remove droplet if off screen
    if (drip->dropletY > gfx->height() + 20) {
      drip->hasDroplet = false;
    }
  }
  
  // Reset column if completely off screen
  if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT) + 50) {
    initColumn(col);
    initDripColumn(col);
    columns[col].y = random(-100, -10);
    return;
  }
  
  // Draw the drip column
  drawDripColumn(col);
}

// Split a column based on fracture type
void splitColumn(int col, int splitType) {
  FractureColumn* fracture = &fractureColumns[col];
  
  if (!fracture->canSplit || fracture->splitCount >= fracture->maxSplits) {
    return; // Cannot split
  }
  
  unsigned long currentTime = millis();
  if (currentTime - fracture->lastSplit < 500) {
    return; // Too soon since last split
  }
  
  fracture->lastSplit = currentTime;
  fracture->splitCount++;
  
  int baseX = col * CHAR_WIDTH;
  int baseY = columns[col].y;
  
  switch(splitType) {
    case 1: // BINARY - Simple 1→2 split
      if (fracture->activeChildren < MAX_CHILD_COLUMNS - 1) {
        // Create left child
        ChildColumn* left = &fracture->children[fracture->activeChildren++];
        left->active = true;
        left->x = baseX - 15;  // Drift left
        left->y = baseY;
        left->xSpeed = -0.5;   // Move left slowly
        left->ySpeed = CHAR_HEIGHT;
        left->length = columns[col].length;
        left->generation = 1;
        left->birthTime = currentTime;
        left->lifespan = 5000; // 5 seconds
        // Copy characters
        for (int i = 0; i < MATRIX_ROWS; i++) {
          left->chars[i] = columns[col].chars[i];
        }
        
        // Create right child
        ChildColumn* right = &fracture->children[fracture->activeChildren++];
        right->active = true;
        right->x = baseX + 15; // Drift right
        right->y = baseY;
        right->xSpeed = 0.5;   // Move right slowly
        right->ySpeed = CHAR_HEIGHT;
        right->length = columns[col].length;
        right->generation = 1;
        right->birthTime = currentTime;
        right->lifespan = 5000;
        // Copy characters
        for (int i = 0; i < MATRIX_ROWS; i++) {
          right->chars[i] = columns[col].chars[i];
        }
      }
      break;
      
    case 2: // TRIPLE - 1→3 split
      if (fracture->activeChildren < MAX_CHILD_COLUMNS - 2) {
        // Create three children: left, center, right
        for (int child = 0; child < 3; child++) {
          ChildColumn* newChild = &fracture->children[fracture->activeChildren++];
          newChild->active = true;
          newChild->x = baseX + (child - 1) * 20; // -20, 0, +20
          newChild->y = baseY;
          newChild->xSpeed = (child - 1) * 0.3;   // -0.3, 0, +0.3
          newChild->ySpeed = CHAR_HEIGHT;
          newChild->length = columns[col].length;
          newChild->generation = 1;
          newChild->birthTime = currentTime;
          newChild->lifespan = 4000;
          // Copy characters
          for (int i = 0; i < MATRIX_ROWS; i++) {
            newChild->chars[i] = columns[col].chars[i];
          }
        }
      }
      break;
      
    case 3: // EXPLOSIVE - Multiple random splits
      {
        int numSplits = random(4, 8); // 4-7 fragments
        for (int s = 0; s < numSplits && fracture->activeChildren < MAX_CHILD_COLUMNS; s++) {
          ChildColumn* newChild = &fracture->children[fracture->activeChildren++];
          newChild->active = true;
          newChild->x = baseX + random(-40, 41);  // Wide scatter
          newChild->y = baseY;
          newChild->xSpeed = random(-15, 16) / 10.0; // -1.5 to +1.5
          newChild->ySpeed = CHAR_HEIGHT + random(-2, 3);
          newChild->length = columns[col].length / 2; // Shorter fragments
          newChild->generation = 1;
          newChild->birthTime = currentTime;
          newChild->lifespan = 3000; // Shorter life
          // Copy characters
          for (int i = 0; i < MATRIX_ROWS; i++) {
            newChild->chars[i] = columns[col].chars[i];
          }
        }
      }
      break;
      
    default: // ORGANIC/CASCADE - Natural split
      if (fracture->activeChildren < MAX_CHILD_COLUMNS - 1) {
        // Create two children with slight curve
        for (int child = 0; child < 2; child++) {
          ChildColumn* newChild = &fracture->children[fracture->activeChildren++];
          newChild->active = true;
          newChild->x = baseX + (child ? 12 : -12);
          newChild->y = baseY;
          newChild->xSpeed = (child ? 0.3 : -0.3) + random(-10, 11) / 50.0; // Add randomness
          newChild->ySpeed = CHAR_HEIGHT;
          newChild->length = columns[col].length;
          newChild->generation = 1;
          newChild->birthTime = currentTime;
          newChild->lifespan = 4000;
          // Copy characters
          for (int i = 0; i < MATRIX_ROWS; i++) {
            newChild->chars[i] = columns[col].chars[i];
          }
        }
      }
      break;
  }
  
  // Disable further splitting for non-cascade modes
  if (currentMode != MATRIX_FRACTURE_CASCADE) {
    fracture->canSplit = false;
  }
}

// Update fracture physics for column splitting effects
void updateFractureColumn(int col, unsigned long currentTime) {
  FractureColumn* fracture = &fractureColumns[col];
  
  // Normal column movement
  columns[col].y += CHAR_HEIGHT;
  
  // Check for split triggers based on mode
  if (fracture->canSplit && fracture->splitCount < fracture->maxSplits) {
    bool shouldSplit = false;
    int splitType = 0;
    
    switch(currentMode) {
      case MATRIX_FRACTURE_BINARY:
        if (random(150) == 0) { // 1 in 150 chance
          shouldSplit = true;
          splitType = 1;
        }
        break;
      case MATRIX_FRACTURE_TRIPLE:
        if (random(200) == 0) { // 1 in 200 chance
          shouldSplit = true;
          splitType = 2;
        }
        break;
      case MATRIX_FRACTURE_CASCADE:
        if (random(100) == 0) { // 1 in 100 chance (more frequent)
          shouldSplit = true;
          splitType = 4; // Default/Organic
        }
        break;
      case MATRIX_FRACTURE_ORGANIC:
        if (random(120) == 0) { // 1 in 120 chance
          shouldSplit = true;
          splitType = 4; // Default/Organic
        }
        break;
      case MATRIX_FRACTURE_EXPLOSIVE:
        if (random(300) == 0) { // 1 in 300 chance (rare but dramatic)
          shouldSplit = true;
          splitType = 3;
        }
        break;
    }
    
    if (shouldSplit) {
      splitColumn(col, splitType);
    }
  }
  
  // Update all child columns
  for (int i = 0; i < MAX_CHILD_COLUMNS; i++) {
    ChildColumn* child = &fracture->children[i];
    
    if (!child->active) continue;
    
    // Check if child has expired
    if (currentTime - child->birthTime > child->lifespan) {
      child->active = false;
      fracture->activeChildren--;
      continue;
    }
    
    // Update child position
    child->x += child->xSpeed;
    child->y += child->ySpeed;
    
    // Check if child is off screen
    if (child->y > gfx->height() + (child->length * CHAR_HEIGHT) || 
        child->x < -CHAR_WIDTH || child->x > gfx->width()) {
      child->active = false;
      fracture->activeChildren--;
      continue;
    }
    
    // CASCADE mode: children can split too!
    if (currentMode == MATRIX_FRACTURE_CASCADE && child->generation < 2) {
      if (random(400) == 0) { // Very rare secondary splits
        // Create a mini-split from this child
        if (fracture->activeChildren < MAX_CHILD_COLUMNS - 1) {
          // Find next free slot
          for (int j = 0; j < MAX_CHILD_COLUMNS; j++) {
            if (!fracture->children[j].active) {
              ChildColumn* newChild = &fracture->children[j];
              newChild->active = true;
              newChild->x = child->x + random(-10, 11);
              newChild->y = child->y;
              newChild->xSpeed = child->xSpeed + random(-3, 4) / 10.0;
              newChild->ySpeed = child->ySpeed;
              newChild->length = child->length / 2; // Smaller fragments
              newChild->generation = child->generation + 1;
              newChild->birthTime = currentTime;
              newChild->lifespan = 2000; // Short life
              // Copy characters
              for (int k = 0; k < MATRIX_ROWS; k++) {
                newChild->chars[k] = child->chars[k];
              }
              fracture->activeChildren++;
              break;
            }
          }
        }
      }
    }
  }
  
  // Reset main column if completely off screen
  if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT) + 50) {
    initColumn(col);
    initFractureColumn(col);
    columns[col].y = random(-100, -10);
    return;
  }
  
  // Draw the fracture column
  drawFractureColumn(col);
}

// Draw fracture column with splits and child columns
void drawFractureColumn(int col) {
  int x = col * CHAR_WIDTH;
  FractureColumn* fracture = &fractureColumns[col];
  
  gfx->setTextSize(1);
  
  // Draw main column trail (same as normal matrix)
  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    
    if (charY < -CHAR_HEIGHT || charY > gfx->height()) continue;
    
    // Calculate color intensity
    int intensity = 255 - (i * 255 / columns[col].length);
    intensity = max(intensity, 15);
    
    uint16_t color = getFractureColor(intensity, i == 0);
    
    // Clear previous position
    gfx->fillRect(x, charY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, BLACK);
    
    // Draw character
    gfx->setTextColor(color);
    gfx->setCursor(x, charY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
  }
  
  // Draw all child columns
  for (int i = 0; i < MAX_CHILD_COLUMNS; i++) {
    ChildColumn* child = &fracture->children[i];
    
    if (!child->active) continue;
    
    // Draw child column trail
    for (int j = 0; j < child->length; j++) {
      int childCharY = child->y - (j * CHAR_HEIGHT);
      
      if (childCharY < -CHAR_HEIGHT || childCharY > gfx->height()) continue;
      if (child->x < -CHAR_WIDTH || child->x > gfx->width()) continue;
      
      // Calculate color intensity for child (slightly dimmer)
      int childIntensity = (255 - (j * 255 / child->length)) * 0.8;
      childIntensity = max(childIntensity, 10);
      
      // Age-based fading
      unsigned long age = millis() - child->birthTime;
      float ageFactor = 1.0 - (age / (float)child->lifespan);
      childIntensity = (int)(childIntensity * ageFactor);
      
      uint16_t childColor = getFractureColor(childIntensity, j == 0);
      
      // Clear previous position
      gfx->fillRect((int)child->x, childCharY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, BLACK);
      
      // Draw child character
      if (childIntensity > 5) {
        gfx->setTextColor(childColor);
        gfx->setCursor((int)child->x, childCharY);
        gfx->print(child->chars[j % MATRIX_ROWS]);
      }
    }
  }
}

// Get color for fracture effects based on current fracture mode
uint16_t getFractureColor(int intensity, bool isHead) {
  switch(currentMode) {
    case MATRIX_FRACTURE_BINARY:
      // Binary fracture: electric white/blue
      if (isHead) return gfx->color565(intensity, intensity, intensity);
      return gfx->color565(intensity * 0.8, intensity * 0.9, intensity);
      
    case MATRIX_FRACTURE_TRIPLE:
      // Triple fracture: lightning blue/cyan
      if (isHead) return gfx->color565(0, intensity, intensity);
      return gfx->color565(0, intensity * 0.8, intensity);
      
    case MATRIX_FRACTURE_CASCADE:
      // Cascade fracture: orange energy
      if (isHead) return gfx->color565(intensity, intensity * 0.6, 0);
      return gfx->color565(intensity * 0.9, intensity * 0.5, 0);
      
    case MATRIX_FRACTURE_ORGANIC:
      // Organic fracture: natural green
      if (isHead) return gfx->color565(0, intensity, 0);
      return gfx->color565(0, intensity * 0.8, intensity * 0.2);
      
    case MATRIX_FRACTURE_EXPLOSIVE:
      // Explosive fracture: red energy
      if (isHead) return gfx->color565(intensity, 0, 0);
      return gfx->color565(intensity, intensity * 0.2, 0);
      
    default:
      return gfx->color565(0, intensity, 0); // Fallback green
  }
}

// === ADVANCED MODE IMPLEMENTATIONS ===

// Update advanced mode physics
void updateAdvancedColumn(int col, unsigned long currentTime) {
  AdvancedColumn* adv = &advancedColumns[col];
  
  // Update global gravity well position (slowly moving)
  if (currentMode == MATRIX_GRAVITYWELL && currentTime - gravityWellTimer > 100) {
    gravityWellTimer = currentTime;
    gravityWellX += sin(currentTime * 0.001) * 0.5;
    gravityWellY += cos(currentTime * 0.00073) * 0.3;
    gravityWellX = constrain(gravityWellX, 50, gfx->width() - 50);
    gravityWellY = constrain(gravityWellY, 50, gfx->height() - 50);
    
    // Recalculate this column's bend amount
    float dx = (col * CHAR_WIDTH) - gravityWellX;
    float dy = columns[col].y - gravityWellY;
    float distance = sqrt(dx*dx + dy*dy);
    adv->bendAmount = max(0.1, 30.0 / (distance + 5));
  }
  
  // Update resonance sync state  
  if (currentMode == MATRIX_RESONANCE) {
    adv->resonancePhase += 0.1 * globalResonanceFreq;
    if (adv->resonancePhase > 6.28) adv->resonancePhase -= 6.28;
    
    // Sync/desync cycles
    if (currentTime % 8000 < 4000) {
      // Sync period - all columns try to match frequency
      adv->isResonant = true;
      globalResonanceFreq = 1.0;
    } else {
      // Desync period - columns drift apart
      adv->isResonant = false;
      globalResonanceFreq = 0.5 + (col % 5) * 0.2; // Different frequencies
    }
  }
  
  // MOLTEN mode: cycle through phases
  if (currentMode == MATRIX_MOLTEN) {
    if (!adv->isMolten && random(300) == 0) {
      adv->isMolten = true;
      adv->moltenStartTime = currentTime;
      adv->moltenPhase = 0; // Start with liquid
    }
    
    if (adv->isMolten) {
      unsigned long elapsed = currentTime - adv->moltenStartTime;
      if (elapsed < 2000) {
        adv->moltenPhase = 0; // Liquid phase (drip-like)
      } else if (elapsed < 4000) {
        adv->moltenPhase = 1; // Fracture phase  
      } else if (elapsed < 6000) {
        adv->moltenPhase = 2; // Remerge phase
      } else {
        adv->isMolten = false; // Reset
      }
    }
  }
  
  // GLASS mode: random glints
  if (currentMode == MATRIX_GLASS) {
    if (!adv->hasGlint && random(500) == 0) {
      adv->hasGlint = true;
      adv->glintTime = currentTime;
      adv->glintIntensity = 255;
    }
    
    if (adv->hasGlint) {
      unsigned long elapsed = currentTime - adv->glintTime;
      if (elapsed > 200) {
        adv->hasGlint = false;
      } else {
        adv->glintIntensity = 255 - (elapsed * 255 / 200);
      }
    }
  }
  
  // Normal column movement (modified by advanced effects)
  columns[col].y += CHAR_HEIGHT;
  
  // Reset if column is off screen
  if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT)) {
    initColumn(col);
    initAdvancedColumn(col);
    return;
  }
  
  // Draw with advanced effects
  drawAdvancedColumn(col);
}

// Draw advanced mode columns with special effects
void drawAdvancedColumn(int col) {
  AdvancedColumn* adv = &advancedColumns[col];
  int baseX = col * CHAR_WIDTH;
  
  // Calculate final X position with advanced effects
  int drawX = baseX;
  
  // GRAVITYWELL: bend columns toward gravity point
  if (currentMode == MATRIX_GRAVITYWELL) {
    float dx = gravityWellX - baseX;
    drawX += (int)(dx * adv->bendAmount * 0.1); // Subtle bend
    drawX = constrain(drawX, 0, gfx->width() - CHAR_WIDTH);
  }
  
  // RESONANCE: rhythmic sway
  if (currentMode == MATRIX_RESONANCE) {
    float sway = sin(adv->resonancePhase) * 8;
    if (!adv->isResonant) sway *= 0.3; // Smaller sway when desynced
    drawX += (int)sway;
    drawX = constrain(drawX, 0, gfx->width() - CHAR_WIDTH);
  }
  
  gfx->setTextSize(1);
  
  // Draw column trail
  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    
    if (charY < -CHAR_HEIGHT || charY > gfx->height()) continue;
    
    // Calculate color intensity
    int intensity = 255 - (i * 255 / columns[col].length);
    intensity = max(intensity, 15);
    
    uint16_t color = getAdvancedColor(intensity, i == 0, col);
    
    // Clear previous position (wider area for gravity well)
    int clearWidth = (currentMode == MATRIX_GRAVITYWELL) ? CHAR_WIDTH + 4 : CHAR_WIDTH;
    gfx->fillRect(drawX - 2, charY - CHAR_HEIGHT, clearWidth, CHAR_HEIGHT, BLACK);
    
    // Draw character
    gfx->setTextColor(color);
    gfx->setCursor(drawX, charY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
    
    // GLASS mode: add glint effect
    if (currentMode == MATRIX_GLASS && adv->hasGlint && i < 3) {
      uint16_t glintColor = gfx->color565(adv->glintIntensity, adv->glintIntensity, adv->glintIntensity);
      gfx->drawPixel(drawX + random(CHAR_WIDTH), charY + random(CHAR_HEIGHT), glintColor);
    }
  }
  
  // GRAVITYWELL: draw subtle gravity indicator
  if (currentMode == MATRIX_GRAVITYWELL && col % 8 == 0) {
    uint16_t wellColor = gfx->color565(0, 0, 100);
    gfx->drawPixel((int)gravityWellX, (int)gravityWellY, wellColor);
    gfx->drawPixel((int)gravityWellX + 1, (int)gravityWellY, wellColor);
    gfx->drawPixel((int)gravityWellX, (int)gravityWellY + 1, wellColor);
    gfx->drawPixel((int)gravityWellX - 1, (int)gravityWellY, wellColor);
    gfx->drawPixel((int)gravityWellX, (int)gravityWellY - 1, wellColor);
  }
}

// Get colors for advanced modes
uint16_t getAdvancedColor(int intensity, bool isHead, int col) {
  AdvancedColumn* adv = &advancedColumns[col];
  
  switch(currentMode) {
    case MATRIX_MOLTEN:
      if (adv->isMolten) {
        if (adv->moltenPhase == 0) {
          // Liquid phase: lava red-orange
          if (isHead) return gfx->color565(intensity, intensity * 0.6, 0);
          return gfx->color565(intensity * 0.8, intensity * 0.4, 0);
        } else if (adv->moltenPhase == 1) {
          // Fracture phase: bright orange cracks
          if (isHead) return gfx->color565(intensity, intensity * 0.8, intensity * 0.2);
          return gfx->color565(intensity * 0.9, intensity * 0.6, 0);
        } else {
          // Remerge phase: cooling to red
          if (isHead) return gfx->color565(intensity, intensity * 0.2, 0);
          return gfx->color565(intensity * 0.8, 0, 0);
        }
      } else {
        // Normal state: dark red
        return gfx->color565(intensity * 0.6, 0, 0);
      }
      
    case MATRIX_GLASS:
      // Crystal clear with white highlights
      if (isHead) return gfx->color565(intensity * 0.8, intensity, intensity);
      if (adv->hasGlint) {
        return gfx->color565(intensity * 0.9, intensity * 0.9, intensity);
      } else {
        return gfx->color565(intensity * 0.6, intensity * 0.8, intensity);
      }
      
    case MATRIX_GRAVITYWELL:
      // Deep space blue with gravity distortion
      {
        float distortion = adv->bendAmount;
        uint8_t blueIntensity = min(255, intensity + (int)(distortion * 50));
        if (isHead) return gfx->color565(0, intensity * 0.3, blueIntensity);
        return gfx->color565(0, intensity * 0.2, blueIntensity * 0.8);
      }
      
    case MATRIX_RESONANCE:
      // Pulsing purple based on resonance phase
      {
        float pulse = (sin(adv->resonancePhase) + 1.0) * 0.5; // 0-1
        uint8_t pulsedIntensity = intensity * (0.5 + pulse * 0.5);
        if (adv->isResonant) {
          return gfx->color565(pulsedIntensity, 0, pulsedIntensity);
        } else {
          return gfx->color565(pulsedIntensity * 0.7, pulsedIntensity * 0.3, pulsedIntensity * 0.7);
        }
      }
      
    case MATRIX_BIOME:
      // Different colors per zone
      switch(adv->biomeZone) {
        case 0: // Liquid zone: aqua
          if (isHead) return gfx->color565(0, intensity, intensity);
          return gfx->color565(0, intensity * 0.8, intensity * 0.9);
        case 1: // Storm zone: electric blue
          if (random(100) < 3) return gfx->color565(255, 255, 255); // Lightning
          if (isHead) return gfx->color565(intensity * 0.3, intensity * 0.7, intensity);
          return gfx->color565(0, intensity * 0.5, intensity * 0.8);
        case 2: // Fracture zone: orange cracks
          if (isHead) return gfx->color565(intensity, intensity * 0.6, 0);
          return gfx->color565(intensity * 0.8, intensity * 0.4, 0);
        default:
          return gfx->color565(0, intensity, 0);
      }
      
    default:
      return gfx->color565(0, intensity, 0);
  }
}

// === MATRIX_PLASMA EMERGENT SIMULATION ===

// Initialize the global plasma field system
void initPlasmaSystem() {
  plasmaInitialized = true;
  plasmaSystemTimer = millis();
  plasmaGlobalEnergy = 128.0;
  
  // Create 4 initial plasma nodes at random positions
  for (int i = 0; i < MAX_PLASMA_NODES; i++) {
    PlasmaNode* node = &plasmaNodes[i];
    
    node->active = true;
    node->x = random(50, gfx->width() - 50);
    node->y = random(50, gfx->height() - 50);
    
    // Mix of attractive and repulsive nodes
    node->strength = (i % 2 == 0) ? random(20, 60) : -random(10, 30);
    
    node->pulsePhase = random(628) / 100.0; // Random phase 0-2π
    node->driftVx = (random(200) - 100) / 200.0; // -0.5 to +0.5
    node->driftVy = (random(200) - 100) / 200.0;
    node->birthTime = millis() + i * 1000; // Stagger activation
  }
}

// Initialize plasma physics for a column
void initPlasmaColumn(int col) {
  PlasmaColumn* plasma = &plasmaColumns[col];
  
  plasma->bendX = 0.0;
  plasma->velocity = 0.0;
  plasma->lastForceX = 0.0;
  plasma->lastUpdate = 0;
}

// Update the global plasma field system
void updatePlasmaSystem(unsigned long currentTime) {
  if (!plasmaInitialized) return;
  
  // Update global energy with slow oscillation
  plasmaGlobalEnergy = 100 + 55 * sin(currentTime * 0.0007);
  
  // Update each plasma node
  for (int i = 0; i < MAX_PLASMA_NODES; i++) {
    PlasmaNode* node = &plasmaNodes[i];
    
    if (!node->active || currentTime < node->birthTime) continue;
    
    // Update pulse phase
    node->pulsePhase += 0.08;
    if (node->pulsePhase > 6.28) node->pulsePhase -= 6.28;
    
    // Pulse the strength
    float pulseFactor = 1.0 + 0.3 * sin(node->pulsePhase);
    node->strength = node->strength * pulseFactor;
    
    // Drift the position slowly
    node->x += node->driftVx;
    node->y += node->driftVy;
    
    // Bounce off edges
    if (node->x <= 30 || node->x >= gfx->width() - 30) {
      node->driftVx = -node->driftVx;
      node->x = constrain(node->x, 30, gfx->width() - 30);
    }
    if (node->y <= 30 || node->y >= gfx->height() - 30) {
      node->driftVy = -node->driftVy;
      node->y = constrain(node->y, 30, gfx->height() - 30);
    }
    
    // Occasionally change drift direction
    if (random(800) == 0) {
      node->driftVx = (random(200) - 100) / 200.0;
      node->driftVy = (random(200) - 100) / 200.0;
    }
  }
}

// Calculate plasma force on a column at position (colX, colY)
float calculatePlasmaForce(int col, float colX, float colY) {
  float totalForceX = 0.0;
  unsigned long currentTime = millis();
  
  // Get plasma-specific parameters
  float energyMult, forceMult, sparkChance;
  getPlasmaParameters(currentMode, &energyMult, &forceMult, &sparkChance);
  
  for (int i = 0; i < MAX_PLASMA_NODES; i++) {
    PlasmaNode* node = &plasmaNodes[i];
    
    if (!node->active || currentTime < node->birthTime) continue;
    
    // Calculate distance to this node
    float dx = node->x - colX;
    float dy = node->y - colY;
    float distance = sqrt(dx*dx + dy*dy);
    
    if (distance < 5.0) distance = 5.0; // Prevent division by zero
    
    // Inverse distance force field
    float forceMagnitude = node->strength / (distance * 0.1);
    
    // Apply global energy scaling with plasma type multiplier
    forceMagnitude *= (plasmaGlobalEnergy / 128.0) * energyMult;
    
    // Apply plasma-specific force multiplier
    forceMagnitude *= forceMult;
    
    // Force direction (positive strength = attract, negative = repel)
    float forceX = (dx / distance) * forceMagnitude * 0.1;
    
    totalForceX += forceX;
  }
  
  return constrain(totalForceX, -15.0 * forceMult, 15.0 * forceMult); // Scale limits too
}

// Update plasma physics for a column
void updatePlasmaColumn(int col, unsigned long currentTime) {
  PlasmaColumn* plasma = &plasmaColumns[col];
  
  // Update global plasma system
  updatePlasmaSystem(currentTime);
  
  // Normal column movement
  columns[col].y += CHAR_HEIGHT;
  
  // Calculate current column position
  float baseX = col * CHAR_WIDTH;
  float colY = columns[col].y;
  
  // Calculate plasma forces
  float forceX = calculatePlasmaForce(col, baseX + plasma->bendX, colY);
  
  // Apply physics: Force -> Acceleration -> Velocity -> Position
  float deltaTime = 0.1; // Fixed time step
  
  // Smooth velocity update with momentum
  plasma->velocity = plasma->velocity * 0.9 + forceX * deltaTime;
  plasma->velocity = constrain(plasma->velocity, -8.0, 8.0);
  
  // Update bend position
  plasma->bendX += plasma->velocity * deltaTime;
  plasma->bendX = constrain(plasma->bendX, -40.0, 40.0); // Limit bend range
  
  // Add slight damping to prevent oscillation
  plasma->bendX *= 0.98;
  
  plasma->lastForceX = forceX;
  plasma->lastUpdate = currentTime;
  
  // Reset if column is off screen
  if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT)) {
    initColumn(col);
    initPlasmaColumn(col);
    return;
  }
  
  // Draw with plasma effects
  drawPlasmaColumn(col);
}

// Draw column with plasma bending effects
void drawPlasmaColumn(int col) {
  PlasmaColumn* plasma = &plasmaColumns[col];
  int baseX = col * CHAR_WIDTH;
  
  // Get plasma-specific parameters
  float energyMult, forceMult, sparkChance;
  getPlasmaParameters(currentMode, &energyMult, &forceMult, &sparkChance);
  
  gfx->setTextSize(1);
  
  // Draw column trail with plasma bending
  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    
    if (charY < -CHAR_HEIGHT || charY > gfx->height()) continue;
    
    // Calculate bend for this character (less bend at the tail)
    float bendFactor = 1.0 - (i / (float)columns[col].length * 0.7); // Head bends more
    int drawX = baseX + (int)(plasma->bendX * bendFactor);
    drawX = constrain(drawX, 0, gfx->width() - CHAR_WIDTH);
    
    // Calculate color intensity
    int intensity = 255 - (i * 255 / columns[col].length);
    intensity = max(intensity, 15);
    
    // Use plasma-specific colors
    uint16_t color = getPlasmaColor(intensity, i == 0, currentMode);
    
    // Clear previous position (wider area for bending)
    gfx->fillRect(drawX - 3, charY - CHAR_HEIGHT, CHAR_WIDTH + 6, CHAR_HEIGHT, BLACK);
    
    // Draw character
    gfx->setTextColor(color);
    gfx->setCursor(drawX, charY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
    
    // Add energy sparks with plasma-specific rates
    if (i < 2 && abs(plasma->lastForceX) > 5.0 && random(100) < sparkChance) {
      uint16_t sparkColor = gfx->color565(255, 255, 255);
      gfx->drawPixel(drawX + random(CHAR_WIDTH), charY + random(CHAR_HEIGHT), sparkColor);
    }
  }
  
  // Debug: Draw plasma nodes (every 10th column) with type-specific colors
  if (col % 10 == 0) {
    for (int i = 0; i < MAX_PLASMA_NODES; i++) {
      PlasmaNode* node = &plasmaNodes[i];
      
      if (!node->active || millis() < node->birthTime) continue;
      
      // Node indicator color based on plasma type and polarity
      uint16_t nodeColor;
      if (node->strength > 0) {
        // Attractive nodes - use plasma theme color
        switch(currentMode) {
          case MATRIX_PLASMA:        nodeColor = gfx->color565(0, 0, 100); break;
          case MATRIX_PLASMA_FIRE:   nodeColor = gfx->color565(100, 40, 0); break;
          case MATRIX_PLASMA_ICE:    nodeColor = gfx->color565(80, 80, 100); break;
          case MATRIX_PLASMA_TOXIC:  nodeColor = gfx->color565(0, 100, 0); break;
          case MATRIX_PLASMA_STORM:  nodeColor = gfx->color565(80, 0, 100); break;
          default:                   nodeColor = gfx->color565(0, 0, 100); break;
        }
      } else {
        // Repulsive nodes - red for all types
        nodeColor = gfx->color565(100, 0, 0);
      }
      
      // Draw subtle 2x2 node indicator
      gfx->drawPixel((int)node->x, (int)node->y, nodeColor);
      gfx->drawPixel((int)node->x + 1, (int)node->y, nodeColor);
      gfx->drawPixel((int)node->x, (int)node->y + 1, nodeColor);
      gfx->drawPixel((int)node->x + 1, (int)node->y + 1, nodeColor);
    }
  }
}

// Get plasma-specific parameters for different energy types
void getPlasmaParameters(EffectMode plasmaType, float* energyMult, float* forceMult, float* sparkChance) {
  switch(plasmaType) {
    case MATRIX_PLASMA:
      *energyMult = 1.0;    // Normal energy
      *forceMult = 1.0;     // Normal force
      *sparkChance = 15.0;  // Normal spark rate
      break;
      
    case MATRIX_PLASMA_FIRE:
      *energyMult = 1.3;    // High energy - more intense
      *forceMult = 1.5;     // Stronger forces - more violent bends
      *sparkChance = 25.0;  // More sparks - fire energy
      break;
      
    case MATRIX_PLASMA_ICE:
      *energyMult = 0.7;    // Low energy - cooler
      *forceMult = 0.6;     // Weaker forces - gentle bends
      *sparkChance = 8.0;   // Fewer sparks - frozen energy
      break;
      
    case MATRIX_PLASMA_TOXIC:
      *energyMult = 1.1;    // Slightly high energy
      *forceMult = 0.9;     // Normal forces but chaotic
      *sparkChance = 20.0;  // More sparks - radioactive
      break;
      
    case MATRIX_PLASMA_STORM:
      *energyMult = 1.2;    // High energy 
      *forceMult = 2.0;     // Very strong forces - lightning-like
      *sparkChance = 30.0;  // Most sparks - electrical storm
      break;
      
    default:
      *energyMult = 1.0;
      *forceMult = 1.0;
      *sparkChance = 15.0;
      break;
  }
}

// Get colors for different plasma types
uint16_t getPlasmaColor(int intensity, bool isHead, EffectMode plasmaType) {
  switch(plasmaType) {
    case MATRIX_PLASMA:
      // Original electric cyan
      if (isHead) {
        return gfx->color565(intensity * 0.3, intensity, intensity);
      } else {
        float energyFactor = (plasmaGlobalEnergy / 128.0);
        uint8_t blue = intensity * energyFactor;
        uint8_t green = intensity * 0.7 * energyFactor;
        return gfx->color565(0, green, blue);
      }
      
    case MATRIX_PLASMA_FIRE:
      // Fire plasma - orange/red energy
      if (isHead) {
        return gfx->color565(intensity, intensity * 0.6, intensity * 0.1);
      } else {
        float energyFactor = (plasmaGlobalEnergy / 128.0);
        uint8_t red = intensity * energyFactor;
        uint8_t green = intensity * 0.4 * energyFactor;
        return gfx->color565(red, green, 0);
      }
      
    case MATRIX_PLASMA_ICE:
      // Ice plasma - white/blue energy  
      if (isHead) {
        return gfx->color565(intensity * 0.8, intensity * 0.9, intensity);
      } else {
        float energyFactor = (plasmaGlobalEnergy / 128.0);
        uint8_t white = intensity * energyFactor;
        uint8_t blue = intensity * energyFactor;
        return gfx->color565(white * 0.6, white * 0.8, blue);
      }
      
    case MATRIX_PLASMA_TOXIC:
      // Toxic plasma - green/yellow radioactive
      if (isHead) {
        return gfx->color565(intensity * 0.4, intensity, intensity * 0.2);
      } else {
        float energyFactor = (plasmaGlobalEnergy / 128.0);
        uint8_t green = intensity * energyFactor;
        uint8_t yellow = intensity * 0.6 * energyFactor;
        return gfx->color565(yellow, green, 0);
      }
      
    case MATRIX_PLASMA_STORM:
      // Storm plasma - purple/white lightning
      if (isHead) {
        return gfx->color565(intensity, intensity * 0.3, intensity);
      } else {
        float energyFactor = (plasmaGlobalEnergy / 128.0);
        uint8_t purple = intensity * energyFactor;
        uint8_t blue = intensity * 0.8 * energyFactor;
        return gfx->color565(purple, purple * 0.3, blue);
      }
      
    default:
      return gfx->color565(0, intensity, intensity);
  }
}

// === MATRIX_CASCADE CHAIN REACTION EMERGENT SIMULATION ===

// Initialize the global cascade system
void initCascadeSystem() {
  cascadeInitialized = true;
  cascadeSystemTimer = millis();
  activeCascadeShockwaves = 0;
  
  // Clear all shockwaves
  for (int i = 0; i < MAX_CASCADE_SHOCKWAVES; i++) {
    cascadeShockwaves[i].active = false;
  }
  
  // Initialize all columns with random baseline stress
  for (int i = 0; i < MATRIX_COLS; i++) {
    cascadeColumns[i].stressLevel = random(20, 50); // Baseline stress
    cascadeColumns[i].hasBeenFractured = false;
    cascadeColumns[i].lastStressTime = 0;
    cascadeColumns[i].stressDecayRate = 0.95; // 5% decay per frame
  }
}

// Initialize cascade physics for a column
void initCascadeColumn(int col) {
  CascadeColumn* cascade = &cascadeColumns[col];
  
  cascade->stressLevel = random(20, 50); // Random baseline stress
  cascade->hasBeenFractured = false;
  cascade->lastStressTime = 0;
  cascade->stressDecayRate = 0.95 + (random(10) / 1000.0); // Slightly random decay
}

// Update the global cascade system
void updateCascadeSystem(unsigned long currentTime) {
  if (!cascadeInitialized) return;
  
  // Get cascade parameters for current mode
  float stressThreshold, shockRadius;
  int shockLifespan;
  float stressDecay;
  getCascadeParameters(currentMode, &stressThreshold, &shockRadius, &shockLifespan, &stressDecay);
  
  activeCascadeShockwaves = 0;
  
  // Update all active shockwaves
  for (int i = 0; i < MAX_CASCADE_SHOCKWAVES; i++) {
    CascadeShockwave* shock = &cascadeShockwaves[i];
    
    if (!shock->active) continue;
    
    // Check if shockwave has expired
    if (currentTime > shock->birthTime + shock->lifespan) {
      shock->active = false;
      continue;
    }
    
    activeCascadeShockwaves++;
    
    // Expand shockwave radius over time (use dynamic radius)
    float age = (currentTime - shock->birthTime) / (float)shock->lifespan;
    shock->radius = shockRadius * age;
    
    // Decay intensity over time
    shock->intensity = shock->intensity * (1.0 - age * 0.5); // Decays to 50% over lifetime
    
    // Apply stress to nearby columns
    for (int col = 0; col < MATRIX_COLS; col++) {
      float colX = col * CHAR_WIDTH + (CHAR_WIDTH / 2);
      float distance = sqrt((shock->x - colX) * (shock->x - colX) + 
                           (shock->y - gfx->height()/2) * (shock->y - gfx->height()/2));
      
      // Apply stress if within shockwave radius
      if (distance <= shock->radius) {
        float falloff = 1.0 - (distance / shock->radius); // Linear falloff
        float stress = shock->intensity * falloff * 30.0; // Scale stress
        applyCascadeStress(col, stress);
      }
    }
  }
  
  // Randomly trigger initial fractures to seed cascades
  if (random(500) == 0) { // Low probability initial fracture
    int col = random(MATRIX_COLS);
    applyCascadeStress(col, stressThreshold + 20); // Guaranteed fracture
  }
}

// Apply stress to a specific column
void applyCascadeStress(int col, float stress) {
  if (col < 0 || col >= MATRIX_COLS) return;
  
  CascadeColumn* cascade = &cascadeColumns[col];
  cascade->stressLevel += stress;
  cascade->lastStressTime = millis();
  
  // Check if this stress causes a fracture
  if (shouldColumnFracture(col)) {
    createCascadeFracture(col);
  }
}

// Check if a column should fracture based on stress level
bool shouldColumnFracture(int col) {
  CascadeColumn* cascade = &cascadeColumns[col];
  
  // Already fractured recently
  if (cascade->hasBeenFractured) return false;
  
  // Get cascade parameters for current mode
  float stressThreshold, shockRadius;
  int shockLifespan;
  float stressDecay;
  getCascadeParameters(currentMode, &stressThreshold, &shockRadius, &shockLifespan, &stressDecay);
  
  // Stress threshold check with small random factor
  float threshold = stressThreshold + random(-10, 10);
  return cascade->stressLevel >= threshold;
}

// Create a fracture and trigger a shockwave
void createCascadeFracture(int col) {
  CascadeColumn* cascade = &cascadeColumns[col];
  
  // Mark as fractured
  cascade->hasBeenFractured = true;
  
  // Calculate fracture position
  float fractureX = col * CHAR_WIDTH + (CHAR_WIDTH / 2);
  float fractureY = columns[col].y; // Use current column position
  
  // Get cascade parameters for current mode
  float stressThreshold, shockRadius;
  int shockLifespan;
  float stressDecay;
  getCascadeParameters(currentMode, &stressThreshold, &shockRadius, &shockLifespan, &stressDecay);
  
  // Trigger shockwave from fracture point
  float intensity = cascade->stressLevel / stressThreshold; // Intensity based on stress
  triggerCascadeShockwave(fractureX, fractureY, intensity);
  
  // Reset stress level
  cascade->stressLevel = random(10, 30); // Low post-fracture stress
}

// Trigger a new shockwave from a fracture
void triggerCascadeShockwave(float x, float y, float intensity) {
  // Get cascade parameters for current mode
  float stressThreshold, shockRadius;
  int shockLifespan;
  float stressDecay;
  getCascadeParameters(currentMode, &stressThreshold, &shockRadius, &shockLifespan, &stressDecay);
  
  // Find an inactive shockwave slot
  for (int i = 0; i < MAX_CASCADE_SHOCKWAVES; i++) {
    CascadeShockwave* shock = &cascadeShockwaves[i];
    
    if (!shock->active) {
      shock->active = true;
      shock->x = x;
      shock->y = y;
      shock->radius = 0.0; // Starts at zero, expands
      shock->intensity = intensity;
      shock->birthTime = millis();
      shock->lifespan = shockLifespan; // Use dynamic lifespan
      break;
    }
  }
}

// Update cascade physics for a column
void updateCascadeColumn(int col, unsigned long currentTime) {
  CascadeColumn* cascade = &cascadeColumns[col];
  
  // Update global cascade system
  updateCascadeSystem(currentTime);
  
  // Get cascade parameters for current mode
  float stressThreshold, shockRadius;
  int shockLifespan;
  float stressDecay;
  getCascadeParameters(currentMode, &stressThreshold, &shockRadius, &shockLifespan, &stressDecay);
  
  // Decay stress over time (use dynamic decay rate)
  if (currentTime - cascade->lastStressTime > 100) { // Decay every 100ms
    cascade->stressLevel *= stressDecay;
    cascade->lastStressTime = currentTime;
  }
  
  // Reset fracture status if stress is very low
  if (cascade->stressLevel < 20.0) {
    cascade->hasBeenFractured = false;
  }
  
  // Normal column movement
  columns[col].y += CHAR_HEIGHT;
  
  // Reset if column is off screen
  if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT)) {
    initColumn(col);
    initCascadeColumn(col);
    return;
  }
  
  // Draw with cascade effects
  drawCascadeColumn(col);
}

// Draw column with cascade stress visualization
void drawCascadeColumn(int col) {
  CascadeColumn* cascade = &cascadeColumns[col];
  int baseX = col * CHAR_WIDTH;
  
  gfx->setTextSize(1);
  
  // Draw column trail with stress-based effects
  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    
    if (charY < -CHAR_HEIGHT || charY > gfx->height()) continue;
    
    // Calculate color intensity
    int intensity = 255 - (i * 255 / columns[col].length);
    intensity = max(intensity, 15);
    
    // Stress-based color modification
    float stressRatio = cascade->stressLevel / CASCADE_STRESS_THRESHOLD;
    stressRatio = constrain(stressRatio, 0.0, 2.0); // Allow over-stress
    
    uint16_t color;
    if (i == 0) {
      // Head: normal green to stressed orange-red
      if (stressRatio < 0.5) {
        // Low stress: normal green
        color = gfx->color565(0, intensity, 0);
      } else if (stressRatio < 1.0) {
        // Medium stress: yellow
        uint8_t red = intensity * (stressRatio - 0.5) * 2.0;
        color = gfx->color565(red, intensity, 0);
      } else {
        // High stress: orange-red
        uint8_t green = intensity * (2.0 - stressRatio);
        color = gfx->color565(intensity, green, 0);
      }
    } else {
      // Trail: green to red gradient based on stress
      if (stressRatio < 0.5) {
        // Low stress: normal green fade
        color = gfx->color565(0, intensity * 0.8, 0);
      } else if (stressRatio < 1.0) {
        // Medium stress: yellow fade
        uint8_t red = intensity * 0.6 * (stressRatio - 0.5) * 2.0;
        color = gfx->color565(red, intensity * 0.8, 0);
      } else {
        // High stress: red fade  
        uint8_t green = intensity * 0.4 * (2.0 - stressRatio);
        color = gfx->color565(intensity * 0.8, green, 0);
      }
    }
    
    // Add fracture glitch effects for highly stressed columns
    int drawX = baseX;
    if (stressRatio > 0.8 && random(100) < 20) {
      drawX += random(-2, 3); // Glitch offset
    }
    drawX = constrain(drawX, 0, gfx->width() - CHAR_WIDTH);
    
    // Clear previous position
    gfx->fillRect(drawX - 1, charY - CHAR_HEIGHT, CHAR_WIDTH + 2, CHAR_HEIGHT, BLACK);
    
    // Draw character
    gfx->setTextColor(color);
    gfx->setCursor(drawX, charY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
    
    // Add stress sparks for high-stress characters
    if (i < 3 && stressRatio > 0.7 && random(100) < (stressRatio * 25)) {
      uint16_t sparkColor = gfx->color565(255, 100, 0); // Orange spark
      gfx->drawPixel(drawX + random(CHAR_WIDTH), charY + random(CHAR_HEIGHT), sparkColor);
    }
  }
  
  // Debug: Draw shockwaves (every 8th column for performance)
  if (col % 8 == 0) {
    for (int i = 0; i < MAX_CASCADE_SHOCKWAVES; i++) {
      CascadeShockwave* shock = &cascadeShockwaves[i];
      
      if (!shock->active) continue;
      
      // Draw shockwave as expanding ring
      uint16_t shockColor = gfx->color565(255, 100, 0); // Orange shockwave
      
      // Draw 4 points of the shockwave circle
      int centerX = (int)shock->x;
      int centerY = (int)shock->y;
      int radius = (int)shock->radius;
      
      if (radius > 2) {
        gfx->drawPixel(centerX + radius, centerY, shockColor);
        gfx->drawPixel(centerX - radius, centerY, shockColor);
        gfx->drawPixel(centerX, centerY + radius, shockColor);
        gfx->drawPixel(centerX, centerY - radius, shockColor);
      }
    }
  }
}

// Get cascade parameters for different cascade types
void getCascadeParameters(EffectMode cascadeType, float* stressThreshold, float* shockRadius, int* shockLifespan, float* stressDecay) {
  switch (cascadeType) {
    case MATRIX_CASCADE:
      // Classic cascade - balanced parameters
      *stressThreshold = CASCADE_STRESS_THRESHOLD;
      *shockRadius = CASCADE_SHOCK_RADIUS;
      *shockLifespan = 1750; // 1.75 seconds
      *stressDecay = 0.95; // 5% decay
      break;
      
    case MATRIX_CASCADE_ICE:
      // Ice cascade - brittle, quick fractures, slow propagation
      *stressThreshold = CASCADE_STRESS_THRESHOLD * 0.7; // Fractures at 70% stress
      *shockRadius = CASCADE_SHOCK_RADIUS * 0.8; // Smaller shockwaves
      *shockLifespan = 2500; // 2.5 seconds - slow ice crack propagation
      *stressDecay = 0.98; // 2% decay - ice holds stress longer
      break;
      
    case MATRIX_CASCADE_FIRE:
      // Fire cascade - explosive, violent, fast spreading
      *stressThreshold = CASCADE_STRESS_THRESHOLD * 0.8; // Moderate threshold
      *shockRadius = CASCADE_SHOCK_RADIUS * 1.4; // Large explosive shockwaves
      *shockLifespan = 1200; // 1.2 seconds - fast fire spread
      *stressDecay = 0.92; // 8% decay - fire burns stress away quickly
      break;
      
    case MATRIX_CASCADE_TOXIC:
      // Toxic cascade - corrosive, persistent, medium spread
      *stressThreshold = CASCADE_STRESS_THRESHOLD * 0.6; // Low threshold - acid eats quickly
      *shockRadius = CASCADE_SHOCK_RADIUS * 1.1; // Slightly larger corrosive spread
      *shockLifespan = 2000; // 2.0 seconds - persistent acid
      *stressDecay = 0.97; // 3% decay - acid lingers
      break;
      
    case MATRIX_CASCADE_STORM:
      // Storm cascade - chaotic, unpredictable, extreme spread
      *stressThreshold = CASCADE_STRESS_THRESHOLD * 1.1; // High threshold - builds tension
      *shockRadius = CASCADE_SHOCK_RADIUS * 1.6; // Massive lightning shockwaves
      *shockLifespan = 1000; // 1.0 seconds - instant lightning
      *stressDecay = 0.90; // 10% decay - storm energy dissipates quickly
      break;
      
    default:
      // Fallback to classic cascade
      *stressThreshold = CASCADE_STRESS_THRESHOLD;
      *shockRadius = CASCADE_SHOCK_RADIUS;
      *shockLifespan = 1750;
      *stressDecay = 0.95;
      break;
  }
}

// Get cascade color based on stress level and cascade type
uint16_t getCascadeColor(int col, int intensity, bool isHead, EffectMode cascadeType) {
  float stressRatio = cascadeColumns[col].stressLevel / CASCADE_STRESS_THRESHOLD;
  stressRatio = constrain(stressRatio, 0.0, 2.0);
  
  switch (cascadeType) {
    case MATRIX_CASCADE_ICE:
      // Ice: Blue → Cyan → White progression
      if (isHead) {
        if (stressRatio < 0.5) {
          // Low stress: deep blue
          return gfx->color565(0, 0, intensity);
        } else if (stressRatio < 1.0) {
          // Medium stress: cyan
          uint8_t green = intensity * (stressRatio - 0.5) * 2.0;
          return gfx->color565(0, green, intensity);
        } else {
          // High stress: bright white
          uint8_t brightness = intensity * (1.0 + (stressRatio - 1.0) * 0.5);
          return gfx->color565(brightness, brightness, brightness);
        }
      } else {
        // Trail: blue fade with stress
        if (stressRatio < 0.5) {
          return gfx->color565(0, 0, intensity * 0.8);
        } else if (stressRatio < 1.0) {
          uint8_t green = intensity * 0.6 * (stressRatio - 0.5) * 2.0;
          return gfx->color565(0, green, intensity * 0.8);
        } else {
          uint8_t brightness = intensity * 0.7 * (1.0 + (stressRatio - 1.0) * 0.3);
          return gfx->color565(brightness, brightness, brightness);
        }
      }
      break;
      
    case MATRIX_CASCADE_FIRE:
      // Fire: Red → Orange → Yellow → White progression  
      if (isHead) {
        if (stressRatio < 0.3) {
          // Low stress: deep red
          return gfx->color565(intensity, 0, 0);
        } else if (stressRatio < 0.7) {
          // Medium stress: orange
          uint8_t green = intensity * (stressRatio - 0.3) * 2.5;
          return gfx->color565(intensity, green, 0);
        } else if (stressRatio < 1.2) {
          // High stress: yellow
          return gfx->color565(intensity, intensity, 0);
        } else {
          // Extreme stress: white hot
          return gfx->color565(intensity, intensity, intensity * 0.8);
        }
      } else {
        // Trail: red-orange fade
        if (stressRatio < 0.3) {
          return gfx->color565(intensity * 0.8, 0, 0);
        } else if (stressRatio < 0.7) {
          uint8_t green = intensity * 0.6 * (stressRatio - 0.3) * 2.5;
          return gfx->color565(intensity * 0.8, green, 0);
        } else if (stressRatio < 1.2) {
          return gfx->color565(intensity * 0.8, intensity * 0.8, 0);
        } else {
          return gfx->color565(intensity * 0.8, intensity * 0.8, intensity * 0.6);
        }
      }
      break;
      
    case MATRIX_CASCADE_TOXIC:
      // Toxic: Green → Yellow-Green → Yellow progression
      if (isHead) {
        if (stressRatio < 0.4) {
          // Low stress: pure green
          return gfx->color565(0, intensity, 0);
        } else if (stressRatio < 1.0) {
          // Medium stress: yellow-green
          uint8_t red = intensity * (stressRatio - 0.4) * 1.67;
          return gfx->color565(red, intensity, 0);
        } else {
          // High stress: toxic yellow
          uint8_t green = intensity * (1.5 - stressRatio * 0.5);
          return gfx->color565(intensity, green, 0);
        }
      } else {
        // Trail: green fade with toxic yellow tint
        if (stressRatio < 0.4) {
          return gfx->color565(0, intensity * 0.8, 0);
        } else if (stressRatio < 1.0) {
          uint8_t red = intensity * 0.6 * (stressRatio - 0.4) * 1.67;
          return gfx->color565(red, intensity * 0.8, 0);
        } else {
          uint8_t green = intensity * 0.6 * (1.5 - stressRatio * 0.5);
          return gfx->color565(intensity * 0.8, green, 0);
        }
      }
      break;
      
    case MATRIX_CASCADE_STORM:
      // Storm: Blue → Cyan → White → Blue-White progression
      if (isHead) {
        if (stressRatio < 0.3) {
          // Low stress: electric blue
          return gfx->color565(0, intensity * 0.5, intensity);
        } else if (stressRatio < 0.8) {
          // Medium stress: bright cyan
          return gfx->color565(0, intensity, intensity);
        } else if (stressRatio < 1.3) {
          // High stress: white lightning
          return gfx->color565(intensity, intensity, intensity);
        } else {
          // Extreme stress: blue-white electric
          uint8_t blue = intensity * (0.5 + stressRatio * 0.3);
          return gfx->color565(intensity * 0.8, intensity * 0.9, blue);
        }
      } else {
        // Trail: blue-cyan fade
        if (stressRatio < 0.3) {
          return gfx->color565(0, intensity * 0.4, intensity * 0.8);
        } else if (stressRatio < 0.8) {
          return gfx->color565(0, intensity * 0.8, intensity * 0.8);
        } else if (stressRatio < 1.3) {
          return gfx->color565(intensity * 0.8, intensity * 0.8, intensity * 0.8);
        } else {
          uint8_t blue = intensity * 0.6 * (0.5 + stressRatio * 0.3);
          return gfx->color565(intensity * 0.6, intensity * 0.7, blue);
        }
      }
      break;
      
    default:
      // Fallback to green
      return gfx->color565(0, intensity, 0);
  }
}

// === MATRIX_ENTROPY LIFECYCLE EMERGENT SIMULATION ===

// Initialize the global entropy system
void initEntropySystem() {
  entropyInitialized = true;
  entropySystem.currentPhase = ENTROPY_ORDER;
  entropySystem.phaseStartTime = millis();
  entropySystem.chaosLevel = 0.0;
  entropySystem.decayRate = 0.002; // Very gradual decay
  
  // Set phase durations (in milliseconds)
  entropySystem.phaseDuration[ENTROPY_ORDER] = 15000;    // 15s perfect order
  entropySystem.phaseDuration[ENTROPY_DECAY] = 25000;    // 25s gradual decay
  entropySystem.phaseDuration[ENTROPY_CHAOS] = 10000;    // 10s pure chaos
  entropySystem.phaseDuration[ENTROPY_COLLAPSE] = 3000;  // 3s system collapse
  entropySystem.phaseDuration[ENTROPY_REBIRTH] = 2000;   // 2s clean restart
  
  // Record original order parameters
  entropySystem.baseSpeed = 100;  // Standard update speed
  entropySystem.baseSpacing = CHAR_HEIGHT; // Standard spacing
  entropySystem.baseColor = 0x07E0; // Standard green
  
  // Initialize chaos injection parameters
  entropySystem.speedVariation = 0.0;
  entropySystem.positionJitter = 0.0; 
  entropySystem.colorDrift = 0.0;
  entropySystem.glitchFrequency = 0;
  
  Serial.println("🌀 ENTROPY SYSTEM: Order→Chaos→Rebirth cycle initialized");
}

// Initialize entropy physics for a column
void initEntropyColumn(int col) {
  EntropyColumn* entropy = &entropyColumns[col];
  
  entropy->targetSpeed = entropySystem.baseSpeed;
  entropy->actualSpeed = entropySystem.baseSpeed;
  entropy->positionOffset = 0.0;
  entropy->colorShift = 0.0;
  entropy->lastGlitch = 0;
  entropy->isGlitching = false;
}

// Update the global entropy system lifecycle
void updateEntropySystem(unsigned long currentTime) {
  if (!entropyInitialized) return;
  
  unsigned long phaseElapsed = currentTime - entropySystem.phaseStartTime;
  bool phaseComplete = (phaseElapsed >= entropySystem.phaseDuration[entropySystem.currentPhase]);
  
  // Update chaos level based on current phase
  switch(entropySystem.currentPhase) {
    case ENTROPY_ORDER:
      // Perfect order: chaos level stays at 0
      entropySystem.chaosLevel = 0.0;
      entropySystem.speedVariation = 0.0;
      entropySystem.positionJitter = 0.0;
      entropySystem.colorDrift = 0.0;
      entropySystem.glitchFrequency = 0;
      break;
      
    case ENTROPY_DECAY:
      // Gradual decay: chaos level slowly increases
      {
        float decayProgress = (float)phaseElapsed / entropySystem.phaseDuration[ENTROPY_DECAY];
        entropySystem.chaosLevel = decayProgress * decayProgress; // Quadratic growth
        entropySystem.speedVariation = entropySystem.chaosLevel * 80.0; // Speed randomness
        entropySystem.positionJitter = entropySystem.chaosLevel * 12.0; // Position chaos
        entropySystem.colorDrift = entropySystem.chaosLevel * 0.5; // Color variation
        entropySystem.glitchFrequency = (int)(entropySystem.chaosLevel * 20); // Glitch rate
      }
      break;
      
    case ENTROPY_CHAOS:
      // Pure chaos: maximum disorder
      entropySystem.chaosLevel = 1.0;
      entropySystem.speedVariation = 100.0; // Maximum speed chaos
      entropySystem.positionJitter = 20.0; // Maximum position chaos
      entropySystem.colorDrift = 1.0; // Maximum color chaos
      entropySystem.glitchFrequency = 30; // Constant glitching
      break;
      
    case ENTROPY_COLLAPSE:
      // System collapse: everything slows and dims
      entropySystem.chaosLevel = 1.5; // Overchaos
      entropySystem.speedVariation = 200.0; // Extreme speed chaos
      entropySystem.positionJitter = 40.0; // Extreme position chaos
      entropySystem.colorDrift = 2.0; // Extreme color chaos
      entropySystem.glitchFrequency = 50; // Maximum glitching
      break;
      
    case ENTROPY_REBIRTH:
      // Clean restart: rapid return to order
      {
        float rebirthProgress = (float)phaseElapsed / entropySystem.phaseDuration[ENTROPY_REBIRTH];
        entropySystem.chaosLevel = 1.0 - rebirthProgress; // Rapid decrease
        entropySystem.speedVariation = entropySystem.chaosLevel * 50.0;
        entropySystem.positionJitter = entropySystem.chaosLevel * 10.0;
        entropySystem.colorDrift = entropySystem.chaosLevel * 0.3;
        entropySystem.glitchFrequency = (int)(entropySystem.chaosLevel * 10);
      }
      break;
  }
  
  // Advance to next phase if current phase is complete
  if (phaseComplete) {
    entropySystem.currentPhase = (EntropyPhase)((entropySystem.currentPhase + 1) % 5);
    entropySystem.phaseStartTime = currentTime;
    
    // Reset all columns for new phase
    for (int i = 0; i < MATRIX_COLS; i++) {
      initEntropyColumn(i);
    }
  }
}

// Update entropy physics for a column
void updateEntropyColumn(int col, unsigned long currentTime) {
  EntropyColumn* entropy = &entropyColumns[col];
  
  // Update global entropy system
  updateEntropySystem(currentTime);
  
  // Apply chaos injection based on current entropy level
  if (entropySystem.chaosLevel > 0.0) {
    // Chaotic speed variation
    entropy->targetSpeed = entropySystem.baseSpeed + random(-(int)entropySystem.speedVariation, (int)entropySystem.speedVariation);
    
    // Smooth speed transitions  
    if (entropy->actualSpeed < entropy->targetSpeed) {
      entropy->actualSpeed += 2;
    } else if (entropy->actualSpeed > entropy->targetSpeed) {
      entropy->actualSpeed -= 2;
    }
    
    // Position jitter
    entropy->positionOffset = random(-(int)entropySystem.positionJitter, (int)entropySystem.positionJitter);
    
    // Color drift
    entropy->colorShift = (random(1000) / 1000.0) * entropySystem.colorDrift;
    
    // Random glitching
    if (!entropy->isGlitching && random(1000) < entropySystem.glitchFrequency) {
      entropy->isGlitching = true;
      entropy->lastGlitch = currentTime;
    }
    
    // End glitch after short duration
    if (entropy->isGlitching && (currentTime - entropy->lastGlitch > 100)) {
      entropy->isGlitching = false;
    }
  }
  
  // Update column position with chaos-affected speed
  if (currentTime - columns[col].lastUpdate > entropy->actualSpeed) {
    columns[col].lastUpdate = currentTime;
    columns[col].y += CHAR_HEIGHT + entropy->positionOffset;
    
    // Entropy-specific character corruption
    if (entropy->isGlitching || entropySystem.chaosLevel > 0.5) {
      // Corrupt some characters
      int corruptChance = (int)(entropySystem.chaosLevel * 30);
      for (int i = 0; i < columns[col].length; i++) {
        if (random(100) < corruptChance) {
          columns[col].chars[i] = matrixChars[random(sizeof(matrixChars) - 1)];
        }
      }
    }
    
    // Reset if column is completely off screen
    if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT)) {
      initColumn(col);
      initEntropyColumn(col);
      return;
    }
  }
  
  // Draw with entropy effects
  int baseX = col * CHAR_WIDTH;
  
  // Skip columns that would be off-screen for M5StickC (135px width)
  if (baseX >= 135) return;
  
  gfx->setTextSize(1);

  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    if (charY < -CHAR_HEIGHT || charY > gfx->height()) continue;
    
    // Apply position jitter
    int drawX = baseX + entropy->positionOffset;
    int drawY = charY;
    
    // Chaos-based bounds checking
    if (drawX < 0) drawX = 0;
    if (drawX > gfx->width() - CHAR_WIDTH) drawX = gfx->width() - CHAR_WIDTH;
    
    // Clear previous position
    gfx->fillRect(drawX - 2, drawY - CHAR_HEIGHT, CHAR_WIDTH + 4, CHAR_HEIGHT, BLACK);
    
    // Get entropy-affected color
    int intensity = 255 - (i * 255 / columns[col].length);
    intensity = max(intensity, 15);
    uint16_t color = getEntropyColor(intensity, i == 0);
    
    // Glitch effect: random color corruption
    if (entropy->isGlitching) {
      color = gfx->color565(random(256), random(256), random(256));
    }
    
    // Draw character
    gfx->setTextColor(color);
    gfx->setCursor(drawX, drawY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
  }
}

// Get color for entropy effects based on current lifecycle phase
uint16_t getEntropyColor(int intensity, bool isHead) {
  // Base color affected by chaos level and current phase
  float chaosLevel = entropySystem.chaosLevel;
  
  switch(entropySystem.currentPhase) {
    case ENTROPY_ORDER:
      // Perfect order: pure classic green
      if (isHead) return gfx->color565(0, intensity, 0);
      return gfx->color565(0, intensity * 0.8, 0);
      
    case ENTROPY_DECAY:
      // Gradual decay: green with slight color drift
      {
        float drift = chaosLevel * 0.3;
        uint8_t red = (uint8_t)(intensity * drift * 0.5);
        uint8_t green = (uint8_t)(intensity * (1.0 - drift * 0.3));
        uint8_t blue = (uint8_t)(intensity * drift * 0.2);
        return gfx->color565(red, green, blue);
      }
      
    case ENTROPY_CHAOS:
      // Pure chaos: wild color variations
      {
        uint8_t red = random(intensity/2, intensity);
        uint8_t green = random(intensity/2, intensity);
        uint8_t blue = random(intensity/4, intensity/2);
        return gfx->color565(red, green, blue);
      }
      
    case ENTROPY_COLLAPSE:
      // System collapse: dark, fading colors
      {
        float fade = 0.3; // Very dim
        uint8_t red = (uint8_t)(intensity * fade * 0.8);
        uint8_t green = (uint8_t)(intensity * fade * 0.4);
        uint8_t blue = (uint8_t)(intensity * fade * 0.2);
        return gfx->color565(red, green, blue);
      }
      
    case ENTROPY_REBIRTH:
      // Clean restart: transition back to green
      {
        unsigned long phaseElapsed = millis() - entropySystem.phaseStartTime;
        float rebirthProgress = (float)phaseElapsed / entropySystem.phaseDuration[ENTROPY_REBIRTH];
        
        // Blend from chaos colors back to green
        uint8_t red = (uint8_t)(intensity * 0.5 * (1.0 - rebirthProgress));
        uint8_t green = (uint8_t)(intensity * (0.5 + rebirthProgress * 0.5));
        uint8_t blue = (uint8_t)(intensity * 0.2 * (1.0 - rebirthProgress));
        return gfx->color565(red, green, blue);
      }
  }
  
  // Fallback
  return gfx->color565(0, intensity, 0);
}

// === MATRIX_TIDE BREATHING EMERGENT SIMULATION ===

// Initialize the global tide system
void initTideSystem() {
  // Initialize tide system parameters
  tideSystem.globalPhase = 0.0;
  tideSystem.risingTide = true;
  
  // Different tide configurations based on mode
  switch (currentMode) {
    case MATRIX_TIDE:
      // Standard green tide - 2 second breathing cycle
      tideSystem.frequency = 0.5;
      tideSystem.amplitude = 0.8;
      break;
      
    case MATRIX_TIDE_FIRE:
      // Fire tide - faster, more intense breathing
      tideSystem.frequency = 0.8; // Faster breathing
      tideSystem.amplitude = 1.0; // More intense
      break;
      
    case MATRIX_TIDE_ICE:
      // Ice tide - slower, gentler breathing
      tideSystem.frequency = 0.3; // Slower breathing
      tideSystem.amplitude = 0.6; // Gentler
      break;
      
    case MATRIX_TIDE_STORM:
      // Storm tide - erratic, powerful breathing
      tideSystem.frequency = 0.7; // Fast but not as fast as fire
      tideSystem.amplitude = 1.2; // Very intense
      break;
      
    case MATRIX_TIDE_DEEP:
      // Deep ocean tide - very slow, massive waves
      tideSystem.frequency = 0.15; // Very slow - 6.6 second cycle
      tideSystem.amplitude = 1.5; // Massive amplitude
      break;
      
    default:
      // Fallback to standard
      tideSystem.frequency = 0.5;
      tideSystem.amplitude = 0.8;
      break;
  }
  
  // Speed modulation parameters
  tideSystem.baseSpeed = CHAR_HEIGHT; // Normal speed
  tideSystem.speedMin = CHAR_HEIGHT / 3; // Slowest during low tide
  tideSystem.speedMax = CHAR_HEIGHT * 2; // Fastest during high tide
  
  // Brightness modulation parameters
  tideSystem.brightnessBase = 180; // Normal brightness
  tideSystem.brightnessMin = 60;   // Dimmest during low tide
  tideSystem.brightnessMax = 255;  // Brightest during high tide
  
  tideSystem.breathingIntensity = 1.0; // Full intensity breathing
}

// Update the global tide system
void updateTideSystem(unsigned long currentTime) {
  // Update global phase for breathing cycle
  float deltaTime = (currentTime % 1000000) / 1000.0; // Prevent overflow, convert to seconds
  tideSystem.globalPhase += tideSystem.frequency * (deltaTime / 1000.0);
  
  // Keep phase in 0-2π range
  if (tideSystem.globalPhase > TWO_PI) {
    tideSystem.globalPhase -= TWO_PI;
  }
  
  // Determine if tide is rising or falling
  float sineValue = sin(tideSystem.globalPhase);
  tideSystem.risingTide = (cos(tideSystem.globalPhase) > 0);
}

// Get current tide speed multiplier for columns
float getTideSpeedMultiplier() {
  // Use sine wave to create breathing speed effect
  float sineValue = sin(tideSystem.globalPhase);
  
  // Convert sine (-1 to +1) to speed range
  float speedRatio = 0.5 + (sineValue * tideSystem.amplitude * 0.5);
  
  // Scale to speed range
  float currentSpeed = tideSystem.speedMin + (speedRatio * (tideSystem.speedMax - tideSystem.speedMin));
  
  return currentSpeed / tideSystem.baseSpeed; // Return as multiplier
}

// Get current tide brightness for breathing effect
uint8_t getTideBrightness() {
  // Use sine wave to create breathing brightness effect  
  float sineValue = sin(tideSystem.globalPhase);
  
  // Convert sine (-1 to +1) to brightness range
  float brightnessRatio = 0.5 + (sineValue * tideSystem.amplitude * 0.5);
  
  // Scale to brightness range
  uint8_t brightness = tideSystem.brightnessMin + (brightnessRatio * (tideSystem.brightnessMax - tideSystem.brightnessMin));
  
  return brightness;
}

// === MATRIX_DRIFT NATURAL ORDER DECAY SYSTEM ===

// Initialize the drift system
void initDriftSystem() {
  // Base parameters (will be modified by variants)
  driftSystem.maxDrift = 80.0;
  driftSystem.driftAcceleration = 0.02;
  driftSystem.resetInterval = 45000; // 45 seconds
  driftSystem.lastReset = millis();
  driftSystem.isResetting = false;
  driftSystem.resetProgress = 0.0;
  driftSystem.originalSpacing = 135.0 / MATRIX_COLS;
  driftSystem.currentChaos = 0.0;
  driftSystem.orderPhase = true;
  
  // Customize parameters based on drift variation
  switch(currentMode) {
    case MATRIX_DRIFT:
      // Default settings already set above
      Serial.println("🌪️ DRIFT: Natural order decay initialized");
      break;
      
    case MATRIX_DRIFT_CHAOS:
      // Fast chaotic drift - rapid alignment collapse
      driftSystem.maxDrift = 150.0;        // More drift allowed
      driftSystem.driftAcceleration = 0.08; // 4x faster
      driftSystem.resetInterval = 20000;    // Reset faster (20s)
      Serial.println("🌪️💥 DRIFT CHAOS: Rapid alignment collapse initialized");
      break;
      
    case MATRIX_DRIFT_ORBIT:
      // Orbital drift - columns circle invisible points
      driftSystem.maxDrift = 120.0;        // Moderate drift for orbits
      driftSystem.driftAcceleration = 0.03;
      driftSystem.resetInterval = 60000;    // Longer cycle (60s)
      Serial.println("🌪️⭕ DRIFT ORBIT: Circular motion initialized");
      break;
      
    case MATRIX_DRIFT_GRAVITY:
      // Gravitational drift - columns attract/repel
      driftSystem.maxDrift = 100.0;        
      driftSystem.driftAcceleration = 0.04;
      driftSystem.resetInterval = 50000;    // 50s cycle
      Serial.println("🌪️🌌 DRIFT GRAVITY: Gravitational forces initialized");
      break;
      
    case MATRIX_DRIFT_TIDE:
      // Tidal drift - wave-like lateral motion
      driftSystem.maxDrift = 90.0;         
      driftSystem.driftAcceleration = 0.025;
      driftSystem.resetInterval = 40000;    // 40s cycle
      Serial.println("🌪️🌊 DRIFT TIDE: Tidal motion initialized");
      break;
  }
  
  // Initialize all columns with appropriate starting conditions
  for (int i = 0; i < MATRIX_COLS; i++) {
    driftSystem.driftAmount[i] = 0.0;
    
    switch(currentMode) {
      case MATRIX_DRIFT:
      case MATRIX_DRIFT_CHAOS:
        driftSystem.driftVelocity[i] = random(-100, 100) / 1000.0;
        break;
        
      case MATRIX_DRIFT_ORBIT:
        // Start with orbital velocities (perpendicular to radius)
        driftSystem.driftVelocity[i] = sin(i * 0.3) * 0.05;
        break;
        
      case MATRIX_DRIFT_GRAVITY:
        // Start with gravitational-style velocities
        driftSystem.driftVelocity[i] = (i - MATRIX_COLS/2) * 0.01;
        break;
        
      case MATRIX_DRIFT_TIDE:
        // Start with wave-like velocities
        driftSystem.driftVelocity[i] = cos(i * 0.5) * 0.03;
        break;
    }
  }
}

// Update drift system - natural order decay
void updateDriftSystem(unsigned long currentTime) {
  // Check if it's time for reset
  if (currentTime - driftSystem.lastReset > driftSystem.resetInterval) {
    if (!driftSystem.isResetting) {
      driftSystem.isResetting = true;
      driftSystem.resetProgress = 0.0;
      Serial.println("🌪️ DRIFT: Resetting to order...");
    }
  }
  
  if (driftSystem.isResetting) {
    // Smooth reset back to order
    driftSystem.resetProgress += 0.02;  // 2% per frame
    
    if (driftSystem.resetProgress >= 1.0) {
      // Reset complete
      driftSystem.isResetting = false;
      driftSystem.lastReset = currentTime;
      driftSystem.currentChaos = 0.0;
      driftSystem.orderPhase = true;
      
      // Zero all drift
      for (int i = 0; i < MATRIX_COLS; i++) {
        driftSystem.driftAmount[i] = 0.0;
        driftSystem.driftVelocity[i] = random(-100, 100) / 1000.0;  // New random velocities
      }
      
      Serial.println("🌪️ DRIFT: Order restored");
    } else {
      // Smoothly interpolate all drift back to zero
      for (int i = 0; i < MATRIX_COLS; i++) {
        driftSystem.driftAmount[i] *= (1.0 - driftSystem.resetProgress);
      }
    }
  } else {
    // Normal drift accumulation phase - different behavior per variant
    float totalDrift = 0.0;
    
    for (int i = 0; i < MATRIX_COLS; i++) {
      float acceleration = 0.0;
      
      // Different drift behaviors per variant
      switch(currentMode) {
        case MATRIX_DRIFT:
          // Classic random drift
          acceleration = random(-100, 100) / 10000.0;
          driftSystem.driftVelocity[i] *= 0.995; // Light drag
          break;
          
        case MATRIX_DRIFT_CHAOS:
          // Chaotic rapid drift - more random, less drag
          acceleration = random(-200, 200) / 8000.0;  // 2.5x stronger random force
          driftSystem.driftVelocity[i] *= 0.990;      // Less drag = more chaos
          break;
          
        case MATRIX_DRIFT_ORBIT:
          // Orbital motion - circular forces
          {
            float angle = (currentTime * 0.001 + i * 0.5) * 0.3; // Slow rotation
            float centrifugal = sin(angle) * 0.002;
            acceleration = centrifugal;
            driftSystem.driftVelocity[i] *= 0.998; // Moderate drag for stable orbits
          }
          break;
          
        case MATRIX_DRIFT_GRAVITY:
          // Gravitational attraction/repulsion
          {
            float center = MATRIX_COLS / 2.0;
            float distance = abs(i - center);
            float gravForce = (distance < center * 0.3) ? -0.001 : 0.0015; // Repel close, attract far
            acceleration = gravForce * (i - center) / distance;
            driftSystem.driftVelocity[i] *= 0.993; // Moderate drag
          }
          break;
          
        case MATRIX_DRIFT_TIDE:
          // Wave-like tidal motion
          {
            float wavePhase = currentTime * 0.0008 + i * 0.2; // Slow wave
            float tideForce = sin(wavePhase) * 0.003;
            acceleration = tideForce;
            driftSystem.driftVelocity[i] *= 0.996; // Light drag for smooth waves
          }
          break;
      }
      
      // Apply calculated acceleration
      driftSystem.driftVelocity[i] += acceleration;
      
      // Accumulate drift
      driftSystem.driftAmount[i] += driftSystem.driftVelocity[i] * driftSystem.driftAcceleration;
      
      // Clamp to maximum drift
      if (abs(driftSystem.driftAmount[i]) > driftSystem.maxDrift) {
        driftSystem.driftAmount[i] = (driftSystem.driftAmount[i] > 0) ? driftSystem.maxDrift : -driftSystem.maxDrift;
      }
      
      totalDrift += abs(driftSystem.driftAmount[i]);
    }
    
    // Update chaos level based on total drift
    driftSystem.currentChaos = totalDrift / (driftSystem.maxDrift * MATRIX_COLS);
    driftSystem.currentChaos = constrain(driftSystem.currentChaos, 0.0, 1.0);
    
    // Check if order phase is ending
    if (driftSystem.orderPhase && driftSystem.currentChaos > 0.3) {
      driftSystem.orderPhase = false;
      Serial.println("🌪️ DRIFT: Order decaying into chaos...");
    }
  }
}

// Get drift offset for a specific column
float getDriftOffset(int col) {
  if (col < 0 || col >= MATRIX_COLS) return 0.0;
  return driftSystem.driftAmount[col];
}

// === RELIC GHOST MEMORY SYSTEM IMPLEMENTATION ===

// Initialize relic system
void initRelicSystem() {
  // Clear all relics
  for (int i = 0; i < 15; i++) {
    relicSystem.relics[i].isActive = false;
  }
  
  // Clear memory bank
  for (int i = 0; i < 50; i++) {
    relicSystem.memory[i].timestamp = 0;
  }
  
  // Initialize system parameters
  relicSystem.lastSpawnTime = millis();
  relicSystem.spawnInterval = random(8000, 15000);  // 8-15 seconds between spawns
  relicSystem.spawnChance = 0.15;                   // 15% chance to spawn when triggered
  relicSystem.memoryIndex = 0;
  relicSystem.activeRelics = 0;
  
  // Visual parameters - relics are always dim and ghostly
  relicSystem.maxBrightness = 80;                   // Maximum 80/255 brightness
  relicSystem.minLifespan = 20000;                  // Minimum 20 seconds alive
  relicSystem.maxLifespan = 60000;                  // Maximum 60 seconds alive
  
  // Haunting behavior
  relicSystem.isHaunted = false;
  relicSystem.hauntingIntensity = 0.0;
  
  Serial.println("👻 RELIC: Ghost memory system initialized");
  Serial.println("👻 Ancient columns will occasionally return from the past...");
}

// Initialize signal detection system
void initSignalSystem() {
  // Clear current pattern
  signalSystem.currentPattern.active = false;
  signalSystem.currentPattern.dissolving = false;
  
  // System parameters
  signalSystem.lastTriggerTime = millis();
  signalSystem.quietPeriod = random(15000, 45000);    // 15-45 seconds between signals
  signalSystem.triggerThreshold = 0.15;              // 15% chance when conditions are right
  
  // Pattern library - enable simple patterns
  signalSystem.availablePatterns = 5;
  for (int i = 0; i < 8; i++) {
    signalSystem.patternEnabled[i] = (i < signalSystem.availablePatterns);
  }
  
  // Visual parameters
  signalSystem.signalBrightness = 200;               // Bright during pattern moments
  signalSystem.fadeSpeed = 0.05;                     // Quick dissolution
  
  Serial.println("🔍 SIGNAL: Pattern detection system initialized");
  Serial.println("🔍 Hidden structures will briefly emerge from the noise...");
}

// Store column data in memory bank for potential relic spawning
void rememberColumn(float x, char character, uint8_t mode) {
  relicSystem.memory[relicSystem.memoryIndex].x = x;
  relicSystem.memory[relicSystem.memoryIndex].character = character;
  relicSystem.memory[relicSystem.memoryIndex].mode = mode;
  relicSystem.memory[relicSystem.memoryIndex].timestamp = millis();
  
  relicSystem.memoryIndex = (relicSystem.memoryIndex + 1) % 50;
}

// Spawn a new relic ghost from old column data
void spawnRelic(float x, char character, uint8_t mode) {
  // Find an inactive relic slot
  for (int i = 0; i < 15; i++) {
    if (!relicSystem.relics[i].isActive) {
      RelicColumn& relic = relicSystem.relics[i];
      
      // Position - use remembered position with some drift
      relic.x = x + random(-30, 30);
      relic.y = random(-50, gfx->height() * 0.3);  // Start in upper area
      
      // Character and appearance - VARY BY RELIC TYPE
      relic.character = character;
      
      // Different behavior based on relic mode
      switch(mode) {
        case MATRIX_RELIC_ANCIENT:
          // Very old, barely visible
          relic.brightness = random(10, 25);           // Extremely dim
          relic.opacity = random(15, 30) / 100.0;     // 15-30% opacity
          relic.speed = random(2, 8) / 100.0;         // Very slow: 0.02-0.08 pixels/frame
          relic.deathTime = relic.birthTime + random(45000, 90000); // Lives 45-90 seconds
          relic.isPulsing = (random(100) < 20);       // 20% chance to pulse
          break;
          
        case MATRIX_RELIC_RECENT:
          // Fresh death, more visible
          relic.brightness = random(40, 80);          // Brighter
          relic.opacity = random(50, 80) / 100.0;    // 50-80% opacity  
          relic.speed = random(10, 25) / 100.0;      // Faster: 0.1-0.25 pixels/frame
          relic.deathTime = relic.birthTime + random(15000, 30000); // Lives 15-30 seconds
          relic.isPulsing = (random(100) < 70);      // 70% chance to pulse (fresh energy)
          break;
          
        case MATRIX_RELIC_SWARM:
          // Multiple generations - random characteristics
          relic.brightness = random(20, 60);          // Variable brightness
          relic.opacity = random(25, 65) / 100.0;    // Variable opacity
          relic.speed = random(5, 30) / 100.0;       // Variable speed
          relic.deathTime = relic.birthTime + random(20000, 50000); // Lives 20-50 seconds
          relic.isPulsing = true;                     // Always pulsing (swarm activity)
          break;
          
        case MATRIX_RELIC_ECHO:
          // Repeating patterns - structured movement
          relic.brightness = random(30, 70);          // Moderate brightness
          relic.opacity = random(40, 70) / 100.0;    // Good visibility for patterns
          relic.speed = random(8, 20) / 100.0;       // Consistent speed
          relic.deathTime = relic.birthTime + random(25000, 40000); // Lives 25-40 seconds  
          relic.isPulsing = true;                     // Always pulsing (echo rhythm)
          break;
          
        default: // MATRIX_RELIC
          // Standard ghost behavior
          relic.brightness = random(20, relicSystem.maxBrightness);
          relic.opacity = random(30, 70) / 100.0;
          relic.speed = random(5, 20) / 100.0;
          relic.deathTime = relic.birthTime + random(relicSystem.minLifespan, relicSystem.maxLifespan);
          relic.isPulsing = (random(100) < 40);
          break;
      }
      
      // Common properties
      relic.birthTime = millis();
      relic.originalMode = mode;
      relic.isDrifting = (random(100) < 60);         // 60% chance to drift
      relic.driftDirection = random(-100, 100) / 10000.0; // Very slow drift
      relic.phase = random(0, 628) / 100.0;          // Random phase for pulsing
      
      // Activate
      relic.isActive = true;
      relicSystem.activeRelics++;
      
      Serial.printf("👻 RELIC: Ancient column spawned at (%.1f, %.1f) - Mode %d\n", relic.x, relic.y, mode);
      break;
    }
  }
}

// Update relic system
void updateRelicSystem(unsigned long currentTime) {
  // Different spawn behavior based on relic mode
  int maxActiveRelics = 8;
  float spawnChanceMultiplier = 1.0;
  int baseSpawnInterval = 8000;
  int maxSpawnInterval = 15000;
  
  switch(currentMode) {
    case MATRIX_RELIC_ANCIENT:
      // Very rare spawns, few active
      maxActiveRelics = 3;
      spawnChanceMultiplier = 0.3;   // 30% normal chance
      baseSpawnInterval = 15000;     // 15-25s intervals  
      maxSpawnInterval = 25000;
      break;
      
    case MATRIX_RELIC_RECENT:
      // More frequent spawns, dies faster
      maxActiveRelics = 6;
      spawnChanceMultiplier = 1.5;   // 150% normal chance
      baseSpawnInterval = 5000;      // 5-10s intervals
      maxSpawnInterval = 10000;
      break;
      
    case MATRIX_RELIC_SWARM:
      // High activity, many active relics
      maxActiveRelics = 12;
      spawnChanceMultiplier = 2.0;   // 200% normal chance
      baseSpawnInterval = 3000;      // 3-8s intervals
      maxSpawnInterval = 8000;
      break;
      
    case MATRIX_RELIC_ECHO:
      // Rhythmic spawning pattern
      maxActiveRelics = 8;
      spawnChanceMultiplier = 1.2;   // 120% normal chance
      baseSpawnInterval = 6000;      // 6-12s intervals
      maxSpawnInterval = 12000;
      break;
      
    default: // MATRIX_RELIC
      // Standard behavior (keep existing values)
      break;
  }
  
  // Check if it's time to spawn new relics
  if (currentTime - relicSystem.lastSpawnTime > relicSystem.spawnInterval && 
      relicSystem.activeRelics < maxActiveRelics) {
    
    if (random(1000) < (relicSystem.spawnChance * spawnChanceMultiplier * 1000)) {
      // Try to spawn from memory
      for (int attempts = 0; attempts < 5; attempts++) {
        int memIdx = random(50);
        if (relicSystem.memory[memIdx].timestamp > 0 && 
            currentTime - relicSystem.memory[memIdx].timestamp > 10000) {  // At least 10s old
          
          spawnRelic(
            relicSystem.memory[memIdx].x,
            relicSystem.memory[memIdx].character,
            currentMode  // Pass current mode instead of stored mode for variations
          );
          break;
        }
      }
    }
    
    relicSystem.lastSpawnTime = currentTime;
    relicSystem.spawnInterval = random(baseSpawnInterval, maxSpawnInterval);
  }
  
  // Update all active relics
  relicSystem.activeRelics = 0;
  for (int i = 0; i < 15; i++) {
    RelicColumn& relic = relicSystem.relics[i];
    
    if (relic.isActive) {
      // Check if relic should die
      if (currentTime > relic.deathTime) {
        relic.isActive = false;
        Serial.printf("👻 RELIC: Ancient column faded away after %lu ms\n", 
                     currentTime - relic.birthTime);
        continue;
      }
      
      // Update position - very slow movement
      relic.y += relic.speed;
      
      // Drift sideways if enabled
      if (relic.isDrifting) {
        relic.x += relic.driftDirection;
        
        // Slowly change drift direction
        relic.driftDirection += random(-5, 5) / 100000.0;
        relic.driftDirection = constrain(relic.driftDirection, -0.01, 0.01);
      }
      
      // Pulsing effect
      if (relic.isPulsing) {
        relic.phase += 0.03;  // Slow pulse
        if (relic.phase > 6.28) relic.phase = 0;
      }
      
      // Fade out near end of life
      unsigned long timeLeft = relic.deathTime - currentTime;
      if (timeLeft < 5000) {  // Last 5 seconds
        float fadeRatio = timeLeft / 5000.0;
        relic.opacity *= 0.99;  // Gradual fade
      }
      
      // Remove if off screen (bottom)
      if (relic.y > gfx->height() + 20) {
        relic.isActive = false;
        continue;
      }
      
      relicSystem.activeRelics++;
    }
  }
  
  // Update haunting intensity based on active relics
  relicSystem.hauntingIntensity = relicSystem.activeRelics / 8.0;  // 0.0 to 1.0
  relicSystem.isHaunted = (relicSystem.activeRelics >= 3);
}

// Draw all active relics
void drawRelics() {
  for (int i = 0; i < 15; i++) {
    RelicColumn& relic = relicSystem.relics[i];
    
    if (relic.isActive) {
      // Calculate ghostly brightness
      uint8_t brightness = relic.brightness;
      
      // Apply pulsing if enabled
      if (relic.isPulsing) {
        float pulse = (sin(relic.phase) + 1.0) * 0.5;  // 0.0 to 1.0
        brightness = brightness * (0.6 + pulse * 0.4);  // 60-100% brightness pulse
      }
      
      // Apply opacity
      brightness = brightness * relic.opacity;
      
      // Dim green color for ghostly effect
      uint16_t color = gfx->color565(0, brightness * 0.6, brightness * 0.3);
      
      // Draw the relic character
      gfx->setCursor(relic.x, relic.y);
      gfx->setTextColor(color);
      gfx->print(relic.character);
      
      // Optional: Add a subtle aura/glow effect
      if (brightness > 40) {
        uint16_t auraColor = gfx->color565(0, brightness * 0.2, brightness * 0.1);
        gfx->drawPixel(relic.x - 1, relic.y, auraColor);
        gfx->drawPixel(relic.x + 6, relic.y, auraColor);
      }
    }
  }
}

// Update and draw a single matrix column (mode-aware)
void updateColumn(int col) {
  unsigned long currentTime = millis();
  
  // Update global time for new effects
  globalTime = currentTime * 0.001; // Convert to seconds
  
  // Update wind direction slowly for MATRIX_WIND
  if (currentMode == MATRIX_WIND) {
    windDirection = sin(globalTime * 0.1) * 20.0; // Slow wind direction change
    windSpeed = 0.5 + sin(globalTime * 0.15) * 0.3; // Wind speed variation
  }
  
  // Speed varies by mode
  int updateSpeed = 100;
  if (currentMode == BINARY_RAIN) updateSpeed = 80;  // Faster binary
  if (currentMode == MATRIX_SPEEDBURST) {
    // Speed burst: some columns super fast, others normal
    if (col % 4 == 0 || col % 7 == 0) {
      updateSpeed = 30;  // Super fast columns
    } else if (col % 3 == 0) {
      updateSpeed = 150; // Slower columns  
    } else {
      updateSpeed = 80;  // Normal speed
    }
  }
  
  // Broken streams behavior
  if (currentMode == MATRIX_BROKEN) {
    // Check if column should randomly pause
    if (!columnPaused[col] && random(300) == 0) {
      // Start a pause (1 in 300 chance per update)
      columnPaused[col] = true;
      pauseStartTime[col] = currentTime;
      skipDistance[col] = random(3, 8) * CHAR_HEIGHT; // Skip 3-8 character heights
      return; // Skip this update
    }
    
    // Handle paused columns
    if (columnPaused[col]) {
      if (currentTime - pauseStartTime[col] > random(500, 1500)) {
        // End pause after 0.5-1.5 seconds, skip ahead
        columnPaused[col] = false;
        columns[col].y += skipDistance[col]; // Jump ahead
      } else {
        return; // Stay paused
      }
    }
  }
  
  if (currentTime - columns[col].lastUpdate < (updateSpeed / columns[col].speed)) {
    return;
  }
  columns[col].lastUpdate = currentTime;
  
  // Special drip physics for drip modes
  if (currentMode >= MATRIX_DRIP_BLOOD && currentMode <= MATRIX_DRIP_WATER) {
    updateDripColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special fracture physics for fracture modes
  if (currentMode >= MATRIX_FRACTURE_BINARY && currentMode <= MATRIX_FRACTURE_EXPLOSIVE) {
    updateFractureColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special advanced physics for advanced modes (temporarily disabled for debugging)
  if (false && currentMode >= MATRIX_MOLTEN && currentMode <= MATRIX_BIOME) {
    updateAdvancedColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special plasma physics for plasma mode (temporarily disabled for debugging)
  if (false && currentMode == MATRIX_PLASMA) {
    updatePlasmaColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special plasma physics for plasma variations (temporarily disabled for debugging)
  if (false && currentMode >= MATRIX_PLASMA_FIRE && currentMode <= MATRIX_PLASMA_STORM) {
    updatePlasmaColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special cascade physics for cascade mode (temporarily disabled for debugging)
  if (false && currentMode == MATRIX_CASCADE) {
    updateCascadeColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special cascade physics for cascade variations (temporarily disabled for debugging)
  if (false && currentMode >= MATRIX_CASCADE_ICE && currentMode <= MATRIX_CASCADE_STORM) {
    updateCascadeColumn(col, currentTime);
    return; // Skip normal update logic
  }

  // Special entropy lifecycle for entropy mode (disabled - use normal matrix)
  if (false && currentMode == MATRIX_ENTROPY) {
    updateEntropyColumn(col, currentTime);
    return; // Skip normal update logic
  }
  
  // Special tide breathing system for tide modes (DISABLED - use normal matrix)
  if (false && currentMode >= MATRIX_TIDE && currentMode <= MATRIX_TIDE_DEEP) {
    updateTideSystem(currentTime);
    
    // Apply tide speed modulation (ensure minimum speed)
    float speedMultiplier = getTideSpeedMultiplier();
    if (speedMultiplier < 0.1) speedMultiplier = 0.5; // Prevent stopping
    int adjustedSpeed = max(1, (int)(columns[col].speed * speedMultiplier));
    
    columns[col].y += adjustedSpeed;
    return; // Skip normal update logic
  } 
  // Special drift decay system for drift modes and variations
  else if (currentMode >= MATRIX_DRIFT && currentMode <= MATRIX_DRIFT_TIDE) {
    updateDriftSystem(currentTime);
    columns[col].y += columns[col].speed;
  }
  // Special relic ghost memory system for relic modes and variations
  else if (currentMode >= MATRIX_RELIC && currentMode <= MATRIX_RELIC_ECHO) {
    updateRelicSystem(currentTime);
    columns[col].y += columns[col].speed;
  } 
  // Signal modes use normal physics but can be influenced by pattern alignment
  else if (currentMode >= MATRIX_SIGNAL && currentMode <= MATRIX_SIGNAL_GLITCH) {
    // Normal movement, but pattern system can temporarily override positioning
    columns[col].y += columns[col].speed;
  }
  else {
    // Move column down normally
    columns[col].y += CHAR_HEIGHT;
  }
  
  // Reset if column is completely off screen
  if (columns[col].y > gfx->height() + (columns[col].length * CHAR_HEIGHT)) {
    // Store column in relic memory before resetting (for all MATRIX_RELIC modes)
    if (currentMode >= MATRIX_RELIC && currentMode <= MATRIX_RELIC_ECHO) {
      rememberColumn(col * CHAR_WIDTH, columns[col].chars[0], currentMode);
    }
    
    initColumn(col);
    return;
  }
  
  // Draw the column trail with mode-specific styling
  int x = col * CHAR_WIDTH;
  gfx->setTextSize(1);
  
  for (int i = 0; i < columns[col].length; i++) {
    int charY = columns[col].y - (i * CHAR_HEIGHT);
    
    // Special positioning for new effects
    int drawX = x; // Start with base X position
    int drawY = charY; // Start with base Y position
    
    // MATRIX_PHANTOM: Trail characters drift upward independently
    if (currentMode == MATRIX_PHANTOM && i > 0) {
      drawY = charY + (i * CHAR_HEIGHT * 0.3); // Trail drifts up slower than head falls down
    }
    
    // MATRIX_WIND: Lateral sway based on wind direction and distance from viewer
    if (currentMode == MATRIX_WIND || currentMode == MATRIX_WINDSTORM || 
        currentMode == MATRIX_WINDFIRE || currentMode == MATRIX_WINDICE ||
        currentMode == MATRIX_MICROWIND) {
      float windEffect = windDirection * windSpeed;
      float proximityFactor = 1.0 - (col / (float)MATRIX_COLS * 0.5);
      
      // Different wind intensities
      if (currentMode == MATRIX_MICROWIND) {
        windEffect *= 0.3; // Gentle breeze
      }
      
      drawX = x + (int)(sin(globalTime * 2.0 + col * 0.1) * windEffect * proximityFactor);
    }
    
    // MATRIX_HEAVYWIND: Much stronger wind effect
    if (currentMode == MATRIX_HEAVYWIND) {
      float heavyWindEffect = windDirection * windSpeed * 3.0; // 3x stronger
      float proximityFactor = 1.0 - (col / (float)MATRIX_COLS * 0.5);
      // Faster wind changes
      drawX = x + (int)(sin(globalTime * 6.0 + col * 0.1) * heavyWindEffect * proximityFactor);
    }
    
    // MATRIX_RIPPLE: Sine wave distortion
    if (currentMode == MATRIX_RIPPLE || currentMode == MATRIX_RIPPLEFIRE || 
        currentMode == MATRIX_RIPPLESTORM) {
      drawX = x + (int)(sin((drawY + globalTime * 50) * 0.05) * 8.0);
    }
    
    // MATRIX_RIPPLEWIND variations: Combine ripple waves + wind sway with different physics
    if (currentMode >= MATRIX_RIPPLEWIND && currentMode <= MATRIX_RIPPLEWIND_DEEP) {
      // Base ripple component: sine wave based on Y position and time
      float baseRipple = sin((drawY + globalTime * 50) * 0.05);
      
      // Base wind component: sine wave based on time and column position  
      float baseWind = sin(globalTime * 2.0 + col * 0.1) * windDirection * windSpeed;
      
      // Mode-specific variations
      float rippleOffset, windOffset;
      
      switch(currentMode) {
        case MATRIX_RIPPLEWIND:
          // Original: balanced organic flow
          rippleOffset = baseRipple * 6.0;
          windOffset = baseWind * 4.0;
          break;
          
        case MATRIX_RIPPLEWIND_FIRE:
          // Fire: faster, more chaotic ripples with stronger wind
          rippleOffset = sin((drawY + globalTime * 80) * 0.07) * 7.0;  // Faster, larger ripples
          windOffset = baseWind * 5.0 + sin(globalTime * 4.0 + col * 0.2) * 2.0; // Extra chaos
          break;
          
        case MATRIX_RIPPLEWIND_ICE:
          // Ice: slower, crystalline ripples with gentle wind
          rippleOffset = sin((drawY + globalTime * 30) * 0.03) * 4.0;  // Slower, smaller ripples
          windOffset = baseWind * 2.5; // Gentler wind
          break;
          
        case MATRIX_RIPPLEWIND_STORM:
          // Storm: erratic lightning ripples with violent wind gusts
          {
            float lightning = (random(100) < 3) ? 8.0 : 1.0; // 3% chance of lightning surge
            rippleOffset = baseRipple * 8.0 * lightning;
            windOffset = baseWind * 6.0 + sin(globalTime * 8.0 + col * 0.3) * 3.0; // Violent gusts
          }
          break;
          
        case MATRIX_RIPPLEWIND_DEEP:
          // Deep: slow massive waves with heavy undercurrents
          rippleOffset = sin((drawY + globalTime * 20) * 0.02) * 12.0; // Very slow, very large
          windOffset = baseWind * 3.0 + sin(globalTime * 0.5 + col * 0.05) * 4.0; // Deep currents
          break;
      }
      
      // Combine both effects
      drawX = x + (int)(rippleOffset + windOffset);
    }
    
    // MATRIX_TIDAL: Much stronger ripple waves  
    if (currentMode == MATRIX_TIDAL) {
      drawX = x + (int)(sin((drawY + globalTime * 50) * 0.05) * 24.0); // 3x stronger
    }
    
    // MATRIX_VIBRATE: High-frequency small ripples
    if (currentMode == MATRIX_VIBRATE) {
      drawX = x + (int)(sin((drawY + globalTime * 200) * 0.2) * 4.0); // Fast, small
    }
    
    // MATRIX_SLOWRIPPLE: Very slow hypnotic waves
    if (currentMode == MATRIX_SLOWRIPPLE) {
      drawX = x + (int)(sin((drawY + globalTime * 10) * 0.05) * 8.0); // Very slow
    }
    
    // MATRIX_DRIFT variations: Apply accumulated drift offset  
    if (currentMode >= MATRIX_DRIFT && currentMode <= MATRIX_DRIFT_TIDE) {
      drawX = x + (int)getDriftOffset(col);
    }
    
    // MATRIX_HISTORY_WEIGHT variations: Apply spacetime deflection based on historical paths
    if (currentMode >= MATRIX_HISTORY_WEIGHT && currentMode <= MATRIX_HISTORY_DEEP) {
      drawX = x + (int)historyColumns[col].deflectionX;
    }
    
    // MATRIX_RIPPLEFRACTURE: Combine ripple waves with fracture splitting
    if (currentMode == MATRIX_RIPPLEFRACTURE) {
      // Ripple component: sine wave distortion  
      float rippleOffset = sin((drawY + globalTime * 50) * 0.05) * 8.0;
      drawX = x + (int)rippleOffset;
      
      // Fracture logic: ripple stress causes columns to fracture
      if (abs(rippleOffset) > 6.0 && random(200) == 0) { // High ripple stress + rare chance
        // Trigger fracture split for this column
        if (random(100) < 30) { // 30% chance during high stress
          // Reset this column as a "fractured" column with different properties
          columns[col].speed = random(1, 8);
          columns[col].length = random(6, 25);
        }
      }
    }
    
    // MATRIX_FIREWORKSRIPPLE: Firework particles create ripple shockwaves in matrix
    if (currentMode == MATRIX_FIREWORKSRIPPLE) {
      float totalRipple = 0.0f;
      
      // Check all active firework particles for nearby ripple effects
      for (int p = 0; p < MAX_FIREWORK_PARTICLES; p++) {
        if (!fireworkSystem.particles[p].active) continue;
        
        FireworkParticle* particle = &fireworkSystem.particles[p];
        float particleX = particle->x;
        float particleY = particle->y;
        
        // Calculate distance from particle to this column
        float colX = col * (gfx->width() / MATRIX_COLS) + (gfx->width() / MATRIX_COLS / 2);
        float distance = sqrt((colX - particleX) * (colX - particleX) + (drawY - particleY) * (drawY - particleY));
        
        // Create ripple effect if particle is close enough
        if (distance < 80.0f && particle->energy > 50) {
          // Calculate ripple strength based on particle energy and distance
          float rippleStrength = (particle->energy / 255.0f) * (80.0f - distance) / 80.0f;
          
          // Create expanding wave pattern
          float wavePhase = (globalTime * 100.0f) - (distance * 0.1f);
          float wave = sin(wavePhase * 0.05f) * rippleStrength * 12.0f;
          
          totalRipple += wave;
        }
      }
      
      drawX = x + (int)totalRipple;
    }
    
    if (drawY < -CHAR_HEIGHT || drawY > gfx->height()) continue;
    
    // Mode-specific colors and effects
    uint16_t color;
    if (i == 0) {
      // Head character colors by mode
      switch(currentMode) {
        case MATRIX_CUSTOM:
          color = 0x07FF; // Cyan/green
          break;
        case MATRIX_XSCREEN:
          color = 0x07E0; // Pure green (classic Matrix)
          break;
        case BINARY_RAIN:
          color = 0x001F; // Blue for binary
          break;
        case GLMATRIX_3D:
          color = 0x07FF; // Cyan for 3D effect
          break;
        case MATRIX_MULTICOLOR:
          // Random vibrant colors for head character
          switch(col % 6) {
            case 0: color = 0xF800; break; // Red
            case 1: color = 0x07E0; break; // Green  
            case 2: color = 0x001F; break; // Blue
            case 3: color = 0xFFE0; break; // Yellow
            case 4: color = 0xF81F; break; // Magenta
            case 5: color = 0x07FF; break; // Cyan
          }
          break;
        case MATRIX_SPEEDBURST:
          // Speed-based colors: fast=bright white, slow=dim green, normal=green
          if (col % 4 == 0 || col % 7 == 0) {
            color = 0xFFFF; // Bright white for super fast
          } else if (col % 3 == 0) {
            color = 0x0320; // Dim green for slow  
          } else {
            color = 0x07E0; // Normal green
          }
          break;
        case MATRIX_NEONGREEN:
          // Bright neon green head character
          color = 0xBFE0; // Bright neon green (boosted green with some blue)
          break;
        case MATRIX_PULSE:
          // Pulsing green head with sine wave brightness
          {
            float pulsePhase = (millis() * 0.003); // Slow pulse
            float pulseBrightness = (sin(pulsePhase) + 1.0) / 2.0; // 0.0 to 1.0
            int greenIntensity = 128 + (int)(127 * pulseBrightness); // 128-255
            color = gfx->color565(0, greenIntensity, 0);
          }
          break;
        case MATRIX_GLITCH:
          // Glitchy head with occasional white sparks and normal green
          if (random(500) == 0) {
            // Very rare white spark at head (1 in 500)
            color = 0xFFFF; // Bright white spark
          } else {
            // Normal bright green head
            color = 0x07E0;
          }
          break;
        case MATRIX_BROKEN:
          // Broken stream heads - slightly dimmer green to show "damage"
          color = 0x05A0; // Slightly dim green
          break;
        case MATRIX_RETRO:
          // Retro amber/orange terminal head - bright amber
          color = 0xFD20; // Bright amber/orange
          break;
        case MATRIX_FIRE:
          // Fire effect head - bright yellow/white (hottest part)
          color = 0xFFE0; // Bright yellow
          break;
        case MATRIX_ICE:
          // Ice effect head - bright white/cyan (coldest part)
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_TOXIC:
          // Toxic effect head - sickly bright yellow-green
          color = 0x9FE0; // Toxic yellow-green
          break;
        case MATRIX_CYBER:
          // Cyberpunk head - bright magenta/pink
          color = 0xF81F; // Bright magenta
          break;
        case MATRIX_STORM:
          // Lightning storm head - electric white/blue
          color = 0xE7FF; // Electric blue-white
          break;
        case MATRIX_BLOOD:
          // Blood effect head - dark bright red (fresh blood)
          color = 0xC800; // Dark bright red
          break;
        case MATRIX_GOLD:
          // Gold effect head - bright golden yellow
          color = 0xFFE0; // Bright gold
          break;
        case MATRIX_VOID:
          // Void effect head - bright white (will be inverted)
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_PHANTOM:
          // Phantom effect head - ghostly pale cyan
          color = 0x9FDF; // Pale cyan
          break;
        case MATRIX_ECHO:
          // Echo effect head - bright white with temporal trails
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_WIND:
          // Wind effect head - bright green (classic but swaying)
          color = 0x07E0; // Classic green
          break;
        case MATRIX_RIPPLE:
          // Ripple effect head - bright aqua blue
          color = 0x07FF; // Bright cyan/aqua
          break;
        case MATRIX_RIPPLEWIND:
          // RippleWind head - aqua-green blend for organic flow
          color = 0x3FE0; // Aqua-green blend
          break;
        case MATRIX_RIPPLEWIND_FIRE:
          // RippleWind Fire head - bright orange-red burning ocean
          color = 0xFC00; // Bright orange-red
          break;
        case MATRIX_RIPPLEWIND_ICE:
          // RippleWind Ice head - brilliant ice-white crystal
          color = 0xCFFF; // Ice-white cyan
          break;
        case MATRIX_RIPPLEWIND_STORM:
          // RippleWind Storm head - electric white lightning
          color = 0xFFFF; // Electric white
          break;
        case MATRIX_RIPPLEWIND_DEEP:
          // RippleWind Deep head - deep blue abyssal
          color = 0x001F; // Deep blue
          break;
        case MATRIX_RIPPLEFRACTURE:
          // RippleFracture head - electric cyan for shattering waves
          color = 0x07FF; // Electric cyan
          break;
        case MATRIX_DEPTHFOG:
          // DepthFog effect head - bright white (fog effect)
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_WINDSTORM:
          // WindStorm head - electric blue-white like storm
          color = 0xE7FF; // Electric blue-white
          break;
        case MATRIX_WINDFIRE:
          // WindFire head - bright yellow like fire
          color = 0xFFE0; // Bright yellow
          break;
        case MATRIX_HEAVYWIND:
          // HeavyWind head - bright green with intensity
          color = 0x87E0; // Intense green
          break;
        case MATRIX_WINDICE:
          // WindIce head - bright white like ice
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_MICROWIND:
          // MicroWind head - soft green
          color = 0x05E0; // Gentle green
          break;
        case MATRIX_TIDAL:
          // Tidal head - bright cyan/aqua
          color = 0x07FF; // Bright cyan
          break;
        case MATRIX_RIPPLEFIRE:
          // RippleFire head - bright yellow like fire
          color = 0xFFE0; // Bright yellow
          break;
        case MATRIX_VIBRATE:
          // Vibrate head - bright white for vibration
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_RIPPLESTORM:
          // RippleStorm head - electric blue-white
          color = 0xE7FF; // Electric blue-white
          break;
        case MATRIX_SLOWRIPPLE:
          // SlowRipple head - zen blue-green
          color = 0x07B0; // Calm blue-green
          break;
        case MATRIX_DRIP_BLOOD:
          // Blood drip head - dark bright red
          color = 0xC800; // Dark bright red
          break;
        case MATRIX_DRIP_HONEY:
          // Honey drip head - golden amber
          color = 0xFD60; // Golden amber
          break;
        case MATRIX_DRIP_ACID:
          // Acid drip head - toxic bright green
          color = 0x9FE0; // Toxic green
          break;
        case MATRIX_DRIP_MERCURY:
          // Mercury drip head - metallic silver
          color = 0xBDF7; // Metallic silver
          break;
        case MATRIX_DRIP_WATER:
          // Water drip head - clear blue
          color = 0x87FF; // Clear blue
          break;
        case MATRIX_FRACTURE_BINARY:
          // Binary fracture head - electric white
          color = 0xFFFF; // Bright white
          break;
        case MATRIX_FRACTURE_TRIPLE:
          // Triple fracture head - lightning blue
          color = 0x07FF; // Bright cyan
          break;
        case MATRIX_FRACTURE_CASCADE:
          // Cascade fracture head - orange energy
          color = 0xFD20; // Bright orange
          break;
        case MATRIX_FRACTURE_ORGANIC:
          // Organic fracture head - natural green
          color = 0x87E0; // Bright green
          break;
        case MATRIX_FRACTURE_EXPLOSIVE:
          // Explosive fracture head - red energy
          color = 0xF800; // Bright red
          break;
        case MATRIX_MOLTEN:
          // Molten head - lava orange
          color = 0xFD00; // Lava orange
          break;
        case MATRIX_GLASS:
          // Glass head - crystal white
          color = 0xFFFF; // Pure white
          break;
        case MATRIX_GRAVITYWELL:
          // Gravity well head - deep blue
          color = 0x001F; // Deep blue
          break;
        case MATRIX_RESONANCE:
          // Resonance head - vibrant purple
          color = 0xF81F; // Bright magenta
          break;
        case MATRIX_BIOME:
          // Biome head - varies by zone (handled in drawing)
          color = 0x07E0; // Default green
          break;
        case MATRIX_PLASMA:
          // Plasma head - electric blue energy
          color = 0x07FF; // Bright cyan
          break;
        case MATRIX_PLASMA_FIRE:
          // Fire plasma head - orange energy
          color = 0xFD00; // Orange
          break;
        case MATRIX_PLASMA_ICE:
          // Ice plasma head - white energy
          color = 0xFFFF; // Pure white
          break;
        case MATRIX_PLASMA_TOXIC:
          // Toxic plasma head - green energy
          color = 0x07E0; // Bright green
          break;
        case MATRIX_PLASMA_STORM:
          // Storm plasma head - purple energy
          color = 0xF81F; // Bright magenta
          break;
        case MATRIX_CASCADE:
          // Cascade head - bright orange stress fractures
          color = 0xFD20; // Orange-red
          break;
        case MATRIX_CASCADE_ICE:
          // Ice cascade head - bright white fractures
          color = 0xFFFF; // Pure white
          break;
        case MATRIX_CASCADE_FIRE:
          // Fire cascade head - intense orange-red
          color = 0xF800; // Bright red
          break;
        case MATRIX_CASCADE_TOXIC:
          // Toxic cascade head - bright green
          color = 0x07E0; // Bright green
          break;
        case MATRIX_CASCADE_STORM:
          // Storm cascade head - electric blue-white
          color = 0x07FF; // Cyan
          break;
        case MATRIX_ENTROPY:
          // Simple entropy head - bright chaotic color
          {
            uint8_t red = random(100, 255);
            uint8_t green = random(150, 255);
            uint8_t blue = random(50, 150);
            color = gfx->color565(red, green, blue);
          }
          break;
        case MATRIX_TIDE:
          // Tide head - bright green with breathing brightness
          {
            uint8_t brightness = getTideBrightness();
            color = gfx->color565(0, brightness, 0);
          }
          break;
        case MATRIX_TIDE_FIRE:
          // Fire tide head - breathing orange-red
          {
            uint8_t brightness = getTideBrightness();
            color = gfx->color565(brightness, brightness/2, 0);
          }
          break;
        case MATRIX_TIDE_ICE:
          // Ice tide head - breathing blue-white
          {
            uint8_t brightness = getTideBrightness();
            color = gfx->color565(brightness/4, brightness/2, brightness);
          }
          break;
        case MATRIX_TIDE_STORM:
          // Storm tide head - breathing lightning white
          {
            uint8_t brightness = getTideBrightness();
            color = gfx->color565(brightness/2, brightness/2, brightness);
          }
          break;
        case MATRIX_TIDE_DEEP:
          // Deep tide head - breathing deep blue
          {
            uint8_t brightness = getTideBrightness();
            color = gfx->color565(0, brightness/4, brightness);
          }
          break;
        case MATRIX_DRIFT:
          // Drift head - bright green with chaos-based intensity
          {
            uint8_t brightness = 255 - (driftSystem.currentChaos * 100);  // Dimmer as more chaotic
            color = gfx->color565(0, brightness, 0);
          }
          break;
        case MATRIX_DRIFT_CHAOS:
          // Drift Chaos head - red energy with high chaos intensity
          {
            uint8_t brightness = 200 + (driftSystem.currentChaos * 55);  // Brighter as more chaotic
            color = gfx->color565(brightness, 0, 0);
          }
          break;
        case MATRIX_DRIFT_ORBIT:
          // Drift Orbit head - blue orbital energy
          {
            uint8_t brightness = 200 + (sin(globalTime * 2.0) + 1.0) * 25;  // Pulsing orbital
            color = gfx->color565(0, brightness * 0.3, brightness);
          }
          break;
        case MATRIX_DRIFT_GRAVITY:
          // Drift Gravity head - purple gravitational energy
          {
            uint8_t brightness = 180 + (driftSystem.currentChaos * 75);
            color = gfx->color565(brightness, 0, brightness * 0.8);
          }
          break;
        case MATRIX_DRIFT_TIDE:
          // Drift Tide head - cyan tidal energy
          {
            uint8_t brightness = 190 + (sin(globalTime * 0.8) + 1.0) * 30;  // Tidal breathing
            color = gfx->color565(0, brightness, brightness);
          }
          break;
        case MATRIX_RELIC:
          // Relic head - dim ghostly green with subtle pulsing
          {
            uint8_t brightness = 80 + (sin(globalTime * 0.5) + 1.0) * 20;  // Slow ghost pulse
            color = gfx->color565(0, brightness * 0.6, brightness * 0.3);
          }
          break;
        case MATRIX_RELIC_ANCIENT:
          // Ancient relic - dim but visible, very slow fade
          {
            uint8_t brightness = 80 + (sin(globalTime * 0.2) + 1.0) * 20;  // More visible ancient pulse
            color = gfx->color565(brightness * 0.6, brightness * 0.4, brightness * 0.3);
          }
          break;
        case MATRIX_RELIC_RECENT:
          // Recent relic - brighter, more visible recent death
          {
            uint8_t brightness = 120 + (sin(globalTime * 0.8) + 1.0) * 30;  // Fresh death energy
            color = gfx->color565(brightness * 0.2, brightness * 0.8, brightness * 0.4);
          }
          break;
        case MATRIX_RELIC_SWARM:
          // Swarm relic - multiple generations, shifting colors
          {
            uint8_t brightness = 90 + (sin(globalTime * 1.2) + 1.0) * 40;  // Active swarm energy
            uint8_t phase = (millis() / 100) % 3;
            if (phase == 0) color = gfx->color565(0, brightness * 0.6, brightness * 0.3);        // Green generation
            else if (phase == 1) color = gfx->color565(brightness * 0.4, brightness * 0.4, 0);  // Amber generation  
            else color = gfx->color565(brightness * 0.3, 0, brightness * 0.5);                  // Purple generation
          }
          break;
        case MATRIX_RELIC_ECHO:
          // Echo relic - repeating death pattern with fade
          {
            uint8_t brightness = 100 + (sin(globalTime * 2.0) + 1.0) * 50;  // Fast echo pulse
            color = gfx->color565(brightness * 0.3, brightness * 0.7, brightness * 0.8);
          }
          break;
      }
    } else {
      // Trail fade colors by mode
      int intensity = 255 - (i * 255 / columns[col].length);
      intensity = max(intensity, 15);
      
      switch(currentMode) {
        case MATRIX_CUSTOM:
          color = gfx->color565(0, intensity, intensity/2); // Green-cyan fade
          break;
        case MATRIX_XSCREEN:
          color = gfx->color565(0, intensity, 0); // Pure green fade
          break;
        case BINARY_RAIN:
          color = gfx->color565(0, 0, intensity); // Blue fade
          break;
        case GLMATRIX_3D:
          color = gfx->color565(0, intensity, intensity/2); // Green-cyan fade for 3D
          break;
        case MATRIX_MULTICOLOR:
          // Multicolor fade matching head color
          switch(col % 6) {
            case 0: color = gfx->color565(intensity, 0, 0); break; // Red fade
            case 1: color = gfx->color565(0, intensity, 0); break; // Green fade
            case 2: color = gfx->color565(0, 0, intensity); break; // Blue fade
            case 3: color = gfx->color565(intensity, intensity, 0); break; // Yellow fade
            case 4: color = gfx->color565(intensity, 0, intensity); break; // Magenta fade
            case 5: color = gfx->color565(0, intensity, intensity); break; // Cyan fade
          }
          break;
        case MATRIX_SPEEDBURST:
          // Speed-based trail colors
          if (col % 4 == 0 || col % 7 == 0) {
            color = gfx->color565(intensity, intensity, intensity); // White fade for fast
          } else if (col % 3 == 0) {
            color = gfx->color565(0, intensity/3, 0); // Very dim green for slow
          } else {
            color = gfx->color565(0, intensity, 0); // Normal green fade
          }
          break;
        case MATRIX_NEONGREEN:
          // Neon green gradient: bright neon → lime → dark green
          if (intensity > 200) {
            // Near head: bright neon green with slight blue tint
            color = gfx->color565(0, intensity, intensity/4);
          } else if (intensity > 100) {
            // Middle: lime green 
            color = gfx->color565(intensity/4, intensity, 0);
          } else {
            // Tail: dark green
            color = gfx->color565(0, intensity/2, 0);
          }
          break;
        case MATRIX_PULSE:
          // Pulsing trail with sine wave brightness modulation
          {
            float pulsePhase = (millis() * 0.003); // Same pulse as head
            float pulseBrightness = (sin(pulsePhase) + 1.0) / 2.0; // 0.0 to 1.0
            // Apply pulse to the intensity
            int pulsedIntensity = (int)(intensity * (0.3 + 0.7 * pulseBrightness)); // 30%-100% intensity
            color = gfx->color565(0, pulsedIntensity, 0);
          }
          break;
        case MATRIX_GLITCH:
          // Glitchy trail with random color flickers
          if (random(200) == 0) {
            // 1 in 200 chance of color glitch
            int glitchType = random(3);
            switch(glitchType) {
              case 0: color = gfx->color565(intensity, 0, 0); break; // Red flicker
              case 1: color = gfx->color565(0, 0, intensity); break; // Blue flicker  
              case 2: color = gfx->color565(intensity, 0, intensity); break; // Purple flicker
            }
          } else {
            // Normal green trail fade
            color = gfx->color565(0, intensity, 0);
          }
          break;
        case MATRIX_BROKEN:
          // Broken stream trails - slightly uneven green with occasional gaps
          if (columnPaused[col]) {
            // Paused columns are dimmer
            color = gfx->color565(0, intensity/3, 0);
          } else {
            // Normal but slightly irregular green
            color = gfx->color565(0, intensity * 0.8, 0);
          }
          break;
        case MATRIX_RETRO:
          // Retro amber terminal fade: bright amber → orange → dark brown
          if (intensity > 180) {
            // Near head: bright amber with hint of red
            color = gfx->color565(intensity, intensity * 0.7, 0);
          } else if (intensity > 100) {
            // Middle: orange fade
            color = gfx->color565(intensity, intensity * 0.5, 0);
          } else {
            // Tail: dark orange/brown
            color = gfx->color565(intensity * 0.8, intensity * 0.3, 0);
          }
          break;
        case MATRIX_FIRE:
          // Fire effect: bright yellow → orange → red → dark red
          if (intensity > 200) {
            // Near head: bright yellow (hottest)
            color = gfx->color565(intensity, intensity, 0);
          } else if (intensity > 150) {
            // Hot: orange
            color = gfx->color565(intensity, intensity * 0.6, 0);
          } else if (intensity > 80) {
            // Warm: red
            color = gfx->color565(intensity, intensity * 0.2, 0);
          } else {
            // Cool: dark red
            color = gfx->color565(intensity * 0.8, 0, 0);
          }
          break;
        case MATRIX_ICE:
          // Ice effect: bright white → cyan → blue → dark blue
          if (intensity > 200) {
            // Near head: bright white (coldest)
            color = gfx->color565(intensity, intensity, intensity);
          } else if (intensity > 150) {
            // Cold: bright cyan
            color = gfx->color565(0, intensity, intensity);
          } else if (intensity > 80) {
            // Cool: blue
            color = gfx->color565(0, intensity * 0.5, intensity);
          } else {
            // Frozen: dark blue
            color = gfx->color565(0, 0, intensity * 0.8);
          }
          break;
        case MATRIX_TOXIC:
          // Toxic effect: bright yellow-green → green → sickly yellow → dark green
          if (intensity > 200) {
            // Near head: toxic yellow-green
            color = gfx->color565(intensity * 0.8, intensity, intensity * 0.3);
          } else if (intensity > 150) {
            // Toxic: bright green with yellow tint
            color = gfx->color565(intensity * 0.5, intensity, 0);
          } else if (intensity > 80) {
            // Sickly: yellow-green
            color = gfx->color565(intensity * 0.7, intensity * 0.8, 0);
          } else {
            // Putrid: dark green
            color = gfx->color565(0, intensity * 0.6, 0);
          }
          break;
        case MATRIX_CYBER:
          // Cyberpunk effect: bright magenta → pink → purple → dark purple
          if (intensity > 200) {
            // Near head: bright magenta
            color = gfx->color565(intensity, 0, intensity);
          } else if (intensity > 150) {
            // Cyber: bright pink
            color = gfx->color565(intensity, intensity * 0.3, intensity * 0.8);
          } else if (intensity > 80) {
            // Neon: purple
            color = gfx->color565(intensity * 0.8, 0, intensity);
          } else {
            // Shadow: dark purple
            color = gfx->color565(intensity * 0.5, 0, intensity * 0.6);
          }
          break;
        case MATRIX_STORM:
          // Lightning storm: electric white → blue → purple with random flickers
          if (random(100) < 5) {
            // 5% chance of lightning flicker - bright white
            color = gfx->color565(255, 255, 255);
          } else if (intensity > 200) {
            // Near head: electric blue-white
            color = gfx->color565(intensity * 0.8, intensity * 0.9, intensity);
          } else if (intensity > 150) {
            // Electric: bright blue
            color = gfx->color565(0, intensity * 0.6, intensity);
          } else if (intensity > 80) {
            // Storm: purple-blue
            color = gfx->color565(intensity * 0.4, 0, intensity);
          } else {
            // Dark storm: dark purple
            color = gfx->color565(intensity * 0.3, 0, intensity * 0.6);
          }
          break;
        case MATRIX_BLOOD:
          // Blood effect: bright red → dark red → brown → black
          if (intensity > 200) {
            // Near head: bright red (fresh blood)
            color = gfx->color565(intensity, 0, 0);
          } else if (intensity > 150) {
            // Fresh: red
            color = gfx->color565(intensity * 0.9, intensity * 0.1, 0);
          } else if (intensity > 80) {
            // Drying: dark red-brown
            color = gfx->color565(intensity * 0.7, intensity * 0.2, 0);
          } else {
            // Dried: very dark brown
            color = gfx->color565(intensity * 0.4, intensity * 0.1, 0);
          }
          break;
        case MATRIX_GOLD:
          // Gold effect: bright gold → yellow → orange → dark gold
          if (intensity > 200) {
            // Near head: bright gold
            color = gfx->color565(intensity, intensity * 0.9, 0);
          } else if (intensity > 150) {
            // Rich: golden yellow
            color = gfx->color565(intensity, intensity * 0.8, intensity * 0.1);
          } else if (intensity > 80) {
            // Warm: orange-gold
            color = gfx->color565(intensity, intensity * 0.6, 0);
          } else {
            // Deep: dark gold
            color = gfx->color565(intensity * 0.7, intensity * 0.4, 0);
          }
          break;
        case MATRIX_VOID:
          // Void effect: INVERTED - dark characters on bright background
          // Fill background first, then draw dark characters
          if (i == 0) {
            // Head - fill bright background
            gfx->fillRect(x, charY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, 
                         gfx->color565(200, 200, 255)); // Bright background
            color = gfx->color565(20, 20, 20); // Dark character
          } else {
            // Trail - fade background brightness
            int bgIntensity = intensity;
            gfx->fillRect(x, charY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT,
                         gfx->color565(bgIntensity, bgIntensity, bgIntensity + 50));
            color = gfx->color565(20, 20, 20); // Always dark character
          }
          break;
        case MATRIX_PHANTOM:
          // Phantom effect: ghostly pale colors with alpha jitter
          {
            // Random alpha jitter for ghostly effect
            float jitter = 0.7 + (random(100) / 500.0); // 0.7 to 0.9
            int ghostIntensity = (int)(intensity * jitter);
            
            if (intensity > 150) {
              // Near head: pale cyan
              color = gfx->color565(0, ghostIntensity, ghostIntensity);
            } else if (intensity > 80) {
              // Middle: pale green
              color = gfx->color565(ghostIntensity * 0.3, ghostIntensity, ghostIntensity * 0.5);
            } else {
              // Tail: very faint pale blue
              color = gfx->color565(0, ghostIntensity * 0.5, ghostIntensity * 0.8);
            }
          }
          break;
        case MATRIX_ECHO:
          // Echo effect: bright white with temporal persistence (screen clearing handles the echo)
          color = gfx->color565(intensity, intensity, intensity);
          break;
        case MATRIX_WIND:
          // Wind effect: classic green like original but with sway
          color = gfx->color565(0, intensity, 0);
          break;
        case MATRIX_RIPPLE:
          // Ripple effect: aqua blue with wave distortion
          if (intensity > 150) {
            // Near head: bright aqua
            color = gfx->color565(0, intensity, intensity);
          } else {
            // Trail: blue fade
            color = gfx->color565(0, intensity * 0.7, intensity);
          }
          break;
        case MATRIX_DEPTHFOG:
          // DepthFog effect: atmospheric fade based on screen depth
          {
            // Bottom of screen is foggy, top is sharp
            float fogFactor = 1.0 - (drawY / (float)gfx->height() * 0.6); // 0.4 to 1.0
            int fogIntensity = (int)(intensity * fogFactor);
            
            // White to blue fade in fog
            if (fogFactor > 0.8) {
              // Top area: bright white
              color = gfx->color565(fogIntensity, fogIntensity, fogIntensity);
            } else if (fogFactor > 0.6) {
              // Middle: white-blue
              color = gfx->color565(fogIntensity * 0.8, fogIntensity * 0.9, fogIntensity);
            } else {
              // Bottom fog: dim blue-gray
              color = gfx->color565(fogIntensity * 0.4, fogIntensity * 0.5, fogIntensity * 0.8);
            }
          }
          break;
        case MATRIX_WINDSTORM:
          // WindStorm: Wind sway + storm colors with lightning flickers
          if (random(100) < 5) {
            // 5% chance of lightning flicker - bright white
            color = gfx->color565(255, 255, 255);
          } else if (intensity > 200) {
            // Near head: electric blue-white
            color = gfx->color565(intensity * 0.8, intensity * 0.9, intensity);
          } else if (intensity > 150) {
            // Electric: bright blue
            color = gfx->color565(0, intensity * 0.6, intensity);
          } else if (intensity > 80) {
            // Storm: purple-blue
            color = gfx->color565(intensity * 0.4, 0, intensity);
          } else {
            // Dark storm: dark purple
            color = gfx->color565(intensity * 0.3, 0, intensity * 0.6);
          }
          break;
        case MATRIX_WINDFIRE:
          // WindFire: Wind sway + fire colors
          if (intensity > 200) {
            // Near head: bright yellow (hottest)
            color = gfx->color565(intensity, intensity, 0);
          } else if (intensity > 150) {
            // Hot: orange
            color = gfx->color565(intensity, intensity * 0.6, 0);
          } else if (intensity > 80) {
            // Warm: red
            color = gfx->color565(intensity, intensity * 0.2, 0);
          } else {
            // Cool: dark red
            color = gfx->color565(intensity * 0.8, 0, 0);
          }
          break;
        case MATRIX_HEAVYWIND:
          // HeavyWind: Strong wind with intense green
          color = gfx->color565(0, intensity, 0); // Classic green but with strong sway
          break;
        case MATRIX_WINDICE:
          // WindIce: Wind sway + ice colors
          if (intensity > 200) {
            // Near head: bright white (coldest)
            color = gfx->color565(intensity, intensity, intensity);
          } else if (intensity > 150) {
            // Cold: bright cyan
            color = gfx->color565(0, intensity, intensity);
          } else if (intensity > 80) {
            // Cool: blue
            color = gfx->color565(0, intensity * 0.5, intensity);
          } else {
            // Frozen: dark blue
            color = gfx->color565(0, 0, intensity * 0.8);
          }
          break;
        case MATRIX_MICROWIND:
          // MicroWind: Gentle breeze with soft green
          color = gfx->color565(0, intensity * 0.8, 0); // Softer green
          break;
        case MATRIX_TIDAL:
          // Tidal: Strong ripples with deep aqua colors
          if (intensity > 150) {
            // Near head: bright aqua
            color = gfx->color565(0, intensity, intensity);
          } else {
            // Trail: deep blue fade
            color = gfx->color565(0, intensity * 0.5, intensity);
          }
          break;
        case MATRIX_RIPPLEFIRE:
          // RippleFire: Ripple waves + fire colors
          if (intensity > 200) {
            // Near head: bright yellow (hottest)
            color = gfx->color565(intensity, intensity, 0);
          } else if (intensity > 150) {
            // Hot: orange
            color = gfx->color565(intensity, intensity * 0.6, 0);
          } else if (intensity > 80) {
            // Warm: red
            color = gfx->color565(intensity, intensity * 0.2, 0);
          } else {
            // Cool: dark red
            color = gfx->color565(intensity * 0.8, 0, 0);
          }
          break;
        case MATRIX_VIBRATE:
          // Vibrate: High-frequency ripples with white/electric colors
          if (intensity > 150) {
            // Bright white for vibration
            color = gfx->color565(intensity, intensity, intensity);
          } else {
            // Electric blue fade
            color = gfx->color565(0, intensity * 0.7, intensity);
          }
          break;
        case MATRIX_RIPPLESTORM:
          // RippleStorm: Ripple waves + storm colors with lightning
          if (random(100) < 5) {
            // 5% chance of lightning flicker
            color = gfx->color565(255, 255, 255);
          } else if (intensity > 200) {
            // Electric blue-white
            color = gfx->color565(intensity * 0.8, intensity * 0.9, intensity);
          } else if (intensity > 150) {
            // Bright blue
            color = gfx->color565(0, intensity * 0.6, intensity);
          } else if (intensity > 80) {
            // Purple-blue
            color = gfx->color565(intensity * 0.4, 0, intensity);
          } else {
            // Dark purple
            color = gfx->color565(intensity * 0.3, 0, intensity * 0.6);
          }
          break;
        case MATRIX_SLOWRIPPLE:
          // SlowRipple: Zen colors with slow waves
          if (intensity > 150) {
            // Calm blue-green
            color = gfx->color565(0, intensity, intensity * 0.8);
          } else {
            // Deep zen blue
            color = gfx->color565(0, intensity * 0.6, intensity);
          }
          break;
        case MATRIX_DRIP_BLOOD:
          // Blood drip: handled by getDripColor in drawDripColumn
          color = getDripColor(intensity, i == 0);
          break;
        case MATRIX_DRIP_HONEY:
          // Honey drip: handled by getDripColor in drawDripColumn  
          color = getDripColor(intensity, i == 0);
          break;
        case MATRIX_DRIP_ACID:
          // Acid drip: handled by getDripColor in drawDripColumn
          color = getDripColor(intensity, i == 0);
          break;
        case MATRIX_DRIP_MERCURY:
          // Mercury drip: handled by getDripColor in drawDripColumn
          color = getDripColor(intensity, i == 0);
          break;
        case MATRIX_DRIP_WATER:
          // Water drip: handled by getDripColor in drawDripColumn
          color = getDripColor(intensity, i == 0);
          break;
        case MATRIX_FRACTURE_BINARY:
        case MATRIX_FRACTURE_TRIPLE:
        case MATRIX_FRACTURE_CASCADE:
        case MATRIX_FRACTURE_ORGANIC:
        case MATRIX_FRACTURE_EXPLOSIVE:
          // Fracture modes: handled by getFractureColor in drawFractureColumn
          color = getFractureColor(intensity, i == 0);
          break;
        case MATRIX_MOLTEN:
        case MATRIX_GLASS:
        case MATRIX_GRAVITYWELL:
        case MATRIX_RESONANCE:
        case MATRIX_BIOME:
          // Advanced modes: handled by getAdvancedColor in drawAdvancedColumn
          color = getAdvancedColor(intensity, i == 0, col);
          break;
        case MATRIX_PLASMA:
          // Plasma mode: handled by drawPlasmaColumn
          if (i == 0) {
            color = gfx->color565(intensity * 0.3, intensity, intensity);
          } else {
            float energyFactor = (plasmaGlobalEnergy / 128.0);
            uint8_t blue = intensity * energyFactor;
            uint8_t green = intensity * 0.7 * energyFactor;
            color = gfx->color565(0, green, blue);
          }
          break;
        case MATRIX_PLASMA_FIRE:
        case MATRIX_PLASMA_ICE:
        case MATRIX_PLASMA_TOXIC:
        case MATRIX_PLASMA_STORM:
          // Plasma variations: handled by getPlasmaColor in drawPlasmaColumn
          color = getPlasmaColor(intensity, i == 0, currentMode);
          break;
        case MATRIX_CASCADE:
          // Cascade mode: handled by drawCascadeColumn
          // Stress-based coloring from green to orange-red
          {
            float stressRatio = cascadeColumns[col].stressLevel / CASCADE_STRESS_THRESHOLD;
            stressRatio = constrain(stressRatio, 0.0, 2.0);
            
            if (stressRatio < 0.5) {
              color = gfx->color565(0, intensity * 0.8, 0);
            } else if (stressRatio < 1.0) {
              uint8_t red = intensity * 0.6 * (stressRatio - 0.5) * 2.0;
              color = gfx->color565(red, intensity * 0.8, 0);
            } else {
              uint8_t green = intensity * 0.4 * (2.0 - stressRatio);
              color = gfx->color565(intensity * 0.8, green, 0);
            }
          }
          break;
        case MATRIX_CASCADE_ICE:
        case MATRIX_CASCADE_FIRE:
        case MATRIX_CASCADE_TOXIC:
        case MATRIX_CASCADE_STORM:
          // Cascade variations: get color from getCascadeColor
          color = getCascadeColor(col, intensity, i == 0, currentMode);
          break;
        case MATRIX_ENTROPY:
          // Simple entropy effect - chaotic color variations
          {
            uint8_t red = random(0, intensity/3);
            uint8_t green = random(intensity/2, intensity);
            uint8_t blue = random(0, intensity/4);
            color = gfx->color565(red, green, blue);
          }
          break;
        case MATRIX_TIDE:
          // Simple tide effect - breathing green
          color = gfx->color565(0, 200, 0);
          break;
        case MATRIX_TIDE_FIRE:
          // Simple fire tide - orange
          color = gfx->color565(255, 100, 0);
          break;
        case MATRIX_TIDE_ICE:
          // Simple ice tide - light blue  
          color = gfx->color565(100, 150, 255);
          break;
        case MATRIX_TIDE_STORM:
          // Simple storm tide - white
          color = gfx->color565(200, 200, 200);
          break;
        case MATRIX_TIDE_DEEP:
          // Simple deep tide - dark blue
          color = gfx->color565(0, 50, 150);
          break;
        case MATRIX_DRIFT:
          // Drift trail - green gradient affected by chaos level
          {
            uint8_t chaosEffect = 255 - (driftSystem.currentChaos * 150);  // Dimmer as more chaotic
            uint8_t green = (intensity * chaosEffect) / 255;
            color = gfx->color565(0, green, 0);
          }
          break;
        case MATRIX_DRIFT_CHAOS:
          // Drift Chaos trail - red gradient intensifying with chaos
          {
            uint8_t chaosBoost = 255 + (driftSystem.currentChaos * 100);  // Brighter as more chaotic
            uint8_t red = (intensity * chaosBoost) / 400;  // Scale down to prevent overflow
            red = constrain(red, 0, 255);  // Ensure valid range
            color = gfx->color565(red, 0, 0);
          }
          break;
        case MATRIX_DRIFT_ORBIT:
          // Drift Orbit trail - blue orbital fade
          {
            uint8_t blue = intensity * 0.9;
            uint8_t green = intensity * 0.4;
            color = gfx->color565(0, green, blue);
          }
          break;
        case MATRIX_DRIFT_GRAVITY:
          // Drift Gravity trail - purple gravitational fade
          {
            uint8_t purple = intensity * 0.8;
            color = gfx->color565(purple, 0, purple);
          }
          break;
        case MATRIX_DRIFT_TIDE:
          // Drift Tide trail - cyan tidal fade
          {
            uint8_t cyan = intensity * 0.9;
            color = gfx->color565(0, cyan, cyan);
          }
          break;
        case MATRIX_RELIC:
          // Relic trail - very dim ghostly green fade
          {
            uint8_t green = intensity * 0.3;  // Very dim trails
            color = gfx->color565(0, green, green * 0.5);
          }
          break;
        case MATRIX_RELIC_ANCIENT:
          // Ancient relic trail - dim but visible sepia fade
          {
            uint8_t sepia = intensity * 0.3;  // More visible ancient trails
            color = gfx->color565(sepia * 0.8, sepia * 0.6, sepia * 0.4);
          }
          break;
        case MATRIX_RELIC_RECENT:
          // Recent relic trail - brighter green fade
          {
            uint8_t green = intensity * 0.5;  // More visible recent death trails
            color = gfx->color565(green * 0.3, green, green * 0.6);
          }
          break;
        case MATRIX_RELIC_SWARM:
          // Swarm relic trail - cycling through generation colors
          {
            uint8_t phase = (millis() / 100) % 3;
            if (phase == 0) {
              uint8_t green = intensity * 0.4;
              color = gfx->color565(0, green, green * 0.5);
            } else if (phase == 1) {
              uint8_t amber = intensity * 0.4;
              color = gfx->color565(amber * 0.8, amber * 0.6, 0);
            } else {
              uint8_t purple = intensity * 0.4;
              color = gfx->color565(purple * 0.6, 0, purple);
            }
          }
          break;
        case MATRIX_RELIC_ECHO:
          // Echo relic trail - cyan-tinted echo fade
          {
            uint8_t echo = intensity * 0.4;  // Echo intensity
            color = gfx->color565(echo * 0.4, echo * 0.8, echo);
          }
          break;
        case MATRIX_SIGNAL:
          // Signal trail - bright green backdrop for pattern visibility
          {
            uint8_t green = intensity * 0.9;  // Brighter base matrix for visibility
            color = gfx->color565(0, green, green * 0.2);  // Green with slight cyan tint
          }
          break;
        case MATRIX_SIGNAL_BINARY:
          // Binary signal - stark blue/white digital patterns
          {
            uint8_t digital = intensity * 0.9;
            color = gfx->color565(digital * 0.2, digital * 0.6, digital);  // Electric blue
          }
          break;
        case MATRIX_SIGNAL_ORGANIC:
          // Organic signal - warm earth tones like living patterns  
          {
            uint8_t warmth = intensity * 0.9;
            color = gfx->color565(warmth * 0.6, warmth * 0.8, warmth * 0.3);  // Natural yellow-green
          }
          break;
        case MATRIX_SIGNAL_CRYPTIC:
          // Cryptic signal - ancient gold/amber symbols
          {
            uint8_t amber = intensity * 0.9;
            color = gfx->color565(amber, amber * 0.7, amber * 0.1);  // Ancient gold
          }
          break;
        case MATRIX_SIGNAL_GLITCH:
          // Glitch signal - corrupted colors that flicker wildly
          {
            uint8_t chaos = intensity * 0.9;
            uint8_t flicker = (millis() / 50) % 3;
            if (flicker == 0) {
              color = gfx->color565(chaos, 0, chaos * 0.5);  // Purple glitch
            } else if (flicker == 1) {
              color = gfx->color565(chaos, chaos * 0.3, 0);  // Red corruption
            } else {
              color = gfx->color565(0, chaos, chaos);  // Cyan distortion
            }
          }
          break;
        case MATRIX_HISTORY_WEIGHT:
          // History weight trail - spacetime blue-white fade with deflection effect
          {
            uint8_t spaceBlue = intensity * 0.8;
            uint8_t spaceWhite = intensity * 0.6;
            
            // Add subtle color shift based on deflection intensity
            float deflectionIntensity = abs(historyColumns[col].deflectionX) / 60.0f; // 0.0 to 1.0
            if (deflectionIntensity > 0.3f) {
              // Strong deflection: more cyan-white (higher energy)
              color = gfx->color565(spaceWhite * 0.5, spaceBlue, spaceBlue);
            } else {
              // Weak deflection: soft blue-white (normal spacetime)
              color = gfx->color565(spaceWhite * 0.4, spaceWhite * 0.6, spaceBlue);
            }
          }
          break;
        case MATRIX_HISTORY_FIRE:
          // Fire spacetime trail - hot orange-red with deflection intensity
          {
            uint8_t fireRed = intensity * 0.9;
            uint8_t fireOrange = intensity * 0.7;
            
            float deflectionIntensity = abs(historyColumns[col].deflectionX) / 60.0f;
            if (deflectionIntensity > 0.3f) {
              // Strong deflection: white-hot fire
              color = gfx->color565(fireRed, fireOrange, fireOrange * 0.3);
            } else {
              // Normal deflection: deep fire colors
              color = gfx->color565(fireRed * 0.8, fireOrange * 0.5, 0);
            }
          }
          break;
        case MATRIX_HISTORY_ICE:
          // Ice spacetime trail - frozen blue-white with crystalline effects
          {
            uint8_t iceBlue = intensity * 0.9;
            uint8_t iceCyan = intensity * 0.8;
            
            float deflectionIntensity = abs(historyColumns[col].deflectionX) / 60.0f;
            if (deflectionIntensity > 0.3f) {
              // Strong deflection: brilliant ice crystal
              color = gfx->color565(iceCyan * 0.6, iceCyan, iceBlue);
            } else {
              // Normal deflection: deep frozen blue
              color = gfx->color565(iceCyan * 0.2, iceCyan * 0.6, iceBlue);
            }
          }
          break;
        case MATRIX_HISTORY_STORM:
          // Storm spacetime trail - electric purple-white with lightning effects
          {
            uint8_t stormPurple = intensity * 0.8;
            uint8_t stormWhite = intensity * 0.9;
            
            float deflectionIntensity = abs(historyColumns[col].deflectionX) / 60.0f;
            if (deflectionIntensity > 0.3f) {
              // Strong deflection: electric white lightning
              color = gfx->color565(stormWhite, stormWhite * 0.8, stormWhite);
            } else {
              // Normal deflection: deep storm purple
              color = gfx->color565(stormPurple * 0.7, stormPurple * 0.3, stormPurple);
            }
          }
          break;
        case MATRIX_HISTORY_DEEP:
          // Deep space trail - dark matter colors with subtle gravitational effects
          {
            uint8_t deepBlue = intensity * 0.6;  // Dimmer overall
            uint8_t deepPurple = intensity * 0.5;
            
            float deflectionIntensity = abs(historyColumns[col].deflectionX) / 60.0f;
            if (deflectionIntensity > 0.3f) {
              // Strong deflection: distant starlight
              color = gfx->color565(deepPurple, deepPurple * 0.4, deepBlue);
            } else {
              // Normal deflection: deep space void
              color = gfx->color565(deepPurple * 0.3, deepPurple * 0.2, deepBlue * 0.8);
            }
          }
          break;
        default:
          // Default green fade for any unhandled modes
          color = gfx->color565(0, intensity, 0);
          break;
      }
    }
    
    // Clear previous position (except for MATRIX_ECHO which keeps temporal trails)
    if (currentMode != MATRIX_ECHO) {
      gfx->fillRect(x, charY - CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, BLACK);
    }
    
    // Draw character at special position
    gfx->setTextColor(color);
    gfx->setCursor(drawX, drawY);
    gfx->print(columns[col].chars[i % MATRIX_ROWS]);
  }
}

// Initialize GLMatrix 3D strips
void initGLMatrix() {
  for (int i = 0; i < MATRIX_COLS; i++) {
    glStrips[i].x = i * 8.0;
    glStrips[i].z = random(10, 50) / 10.0;
    glStrips[i].stripLength = random(5, 20);
    glStrips[i].fallSpeed = random(2, 8) / 10.0;
    glStrips[i].spinnerY = -random(5, 15);
    glStrips[i].lastUpdate = millis();
    
    // Initialize glyphs
    for (int j = 0; j < MATRIX_ROWS; j++) {
      glStrips[i].glyphs[j] = random(sizeof(matrixChars) - 1);
      glStrips[i].brightness[j] = random(50, 255) / 255.0;
      glStrips[i].highlight[j] = (random(100) < 10); // 10% chance of highlight
    }
  }
}

// Draw GLMatrix 3D-style effect
void drawGLMatrix() {
  unsigned long currentTime = millis();
  
  // Update camera angle for rotation effect
  cameraAngle += 0.5;
  if (cameraAngle > 360) cameraAngle = 0;
  
  // Update wave phase for brightness waves
  wavePhase += 0.1;
  if (wavePhase > TWO_PI) wavePhase = 0;
  
  // Clear screen with fade effect
  gfx->fillScreen(BLACK);
  
  // Update and draw strips with 3D-like perspective
  for (int i = 0; i < MATRIX_COLS; i++) {
    GLMatrixStrip* strip = &glStrips[i];
    
    if (currentTime - strip->lastUpdate > 100) {
      strip->lastUpdate = currentTime;
      strip->spinnerY += strip->fallSpeed;
      
      // Reset strip if it's gone too far
      if (strip->spinnerY > MATRIX_ROWS + 10) {
        strip->spinnerY = -random(5, 15);
        strip->z = random(10, 50) / 10.0;
        strip->fallSpeed = random(2, 8) / 10.0;
      }
    }
    
    // Calculate 3D perspective effect
    float perspective = 1.0 / (strip->z * 0.1 + 1.0);
    int screenX = (strip->x - 160) * perspective + 160; // Center perspective
    int charSize = max(1, (int)(perspective * 2)); // Size based on depth
    
    // Skip if strip is off screen due to perspective
    if (screenX < -20 || screenX > 340) continue;
    
    gfx->setTextSize(charSize);
    
    // Draw the strip characters
    for (int j = 0; j < strip->stripLength && j < MATRIX_ROWS; j++) {
      int charY = j * (8 * charSize) + (strip->spinnerY * 8 * charSize);
      
      if (charY < -10 || charY > 250) continue;
      
      // Calculate brightness with 3D depth and wave effect
      float baseBrightness = strip->brightness[j] * perspective;
      float waveBrightness = sin(wavePhase + j * 0.3) * 0.3 + 0.7;
      float finalBrightness = baseBrightness * waveBrightness;
      
      // Distance fading (characters further away are dimmer)
      finalBrightness *= (1.0 - (strip->z / 10.0));
      finalBrightness = constrain(finalBrightness, 0.1, 1.0);
      
      // Color calculation based on position and highlight
      uint16_t color;
      if (strip->highlight[j] || j == 0) {
        // Highlighted or lead character - bright green/white
        int intensity = (int)(255 * finalBrightness);
        color = gfx->color565(intensity/2, intensity, intensity/2);
      } else {
        // Normal character - green with depth fading
        int greenIntensity = (int)(200 * finalBrightness);
        color = gfx->color565(0, greenIntensity, 0);
      }
      
      gfx->setTextColor(color);
      gfx->setCursor(screenX, charY);
      
      // Draw character from matrix set
      char c = matrixChars[strip->glyphs[j % (sizeof(matrixChars) - 1)]];
      gfx->print(c);
      
      // Randomly change characters
      if (random(1000) < 5) {
        strip->glyphs[j] = random(sizeof(matrixChars) - 1);
      }
    }
  }
}

void drawMatrix() {
  // Route to appropriate drawing function based on mode
  if (currentMode == GLMATRIX_3D) {
    drawGLMatrix();
    return;
  }
  
  // Special screen handling for MATRIX_ECHO - don't fully clear, create temporal ghosting
  static unsigned long lastEchoClear = 0;
  if (currentMode == MATRIX_ECHO && millis() - lastEchoClear > 200) {
    // Clear with very dark fill instead of black for temporal persistence
    gfx->fillScreen(gfx->color565(3, 3, 5)); // Very dark blue-gray
    lastEchoClear = millis();
  }
  
  // Handle regular matrix modes: MATRIX_CUSTOM, MATRIX_XSCREEN, BINARY_RAIN, and all new modes
  // Update all columns for regular matrix modes
  for (int i = 0; i < MATRIX_COLS; i++) {
    updateColumn(i);
  }
  
  // Draw relic ghosts for all MATRIX_RELIC modes (drawn after normal columns)
  if (currentMode >= MATRIX_RELIC && currentMode <= MATRIX_RELIC_ECHO) {
    drawRelics();
  }
  
  // Update and draw signal patterns for MATRIX_SIGNAL mode and variations
  if (currentMode >= MATRIX_SIGNAL && currentMode <= MATRIX_SIGNAL_GLITCH) {
    updateSignalSystem(millis());
    drawSignalPattern();
  }
  
  // Update and draw firework particles for MATRIX_FIREWORKS mode and all variations including combos
  if ((currentMode >= MATRIX_FIREWORKS && currentMode <= MATRIX_FIREWORKS_ICE) ||
      (currentMode >= MATRIX_FIREWORKSWIND && currentMode <= MATRIX_FIREWORKSENTROPY)) {
    updateFireworkSystem(millis());
    drawFireworkParticles();
  }

  // Update history weight spacetime system for MATRIX_HISTORY_WEIGHT and variations
  if (currentMode >= MATRIX_HISTORY_WEIGHT && currentMode <= MATRIX_HISTORY_DEEP) {
    updateHistorySystem(millis());
  }
  
  // Randomly change some characters (mode-specific)
  if (random(100) < 5) {
    int col = random(MATRIX_COLS);
    int row = random(MATRIX_ROWS);
    
    const char* charSet;
    int charSetSize;
    
    switch(currentMode) {
      case MATRIX_CUSTOM:
        charSet = matrixChars;
        charSetSize = sizeof(matrixChars) - 1;
        break;
      case MATRIX_XSCREEN:
        charSet = xscreenChars;
        charSetSize = sizeof(xscreenChars) - 1;
        break;
      case BINARY_RAIN:
        charSet = binaryChars;
        charSetSize = sizeof(binaryChars) - 1;
        break;
      default:
        charSet = matrixChars;
        charSetSize = sizeof(matrixChars) - 1;
    }
    
    columns[col].chars[row] = charSet[random(charSetSize)];
  }
}

// Initialize Matrix Storm effect
void initMatrixStorm() {
  stormInitialized = false;
  stormTime = 0;
  
  for (int i = 0; i < STORM_DROPS; i++) {
    stormDrops[i].x = random(240);
    stormDrops[i].y = random(-500, 0);
    stormDrops[i].speed = random(20, 80) / 10.0; // 2.0 to 8.0
    stormDrops[i].length = random(6, 18);
    stormDrops[i].charType = random(4);
    stormDrops[i].colorType = random(4);
    stormDrops[i].lastUpdate = millis();
    stormDrops[i].glitchMode = false;
    stormDrops[i].glitchEnd = 0;
    
    // Initialize character trail
    for (int j = 0; j < stormDrops[i].length && j < 20; j++) {
      switch (stormDrops[i].charType) {
        case 0: // Katakana
          stormDrops[i].chars[j] = xscreenChars[random(strlen(xscreenChars))];
          break;
        case 1: // Binary
          stormDrops[i].chars[j] = (random(2)) ? '0' : '1';
          break;
        case 2: // Symbols
          stormDrops[i].chars[j] = matrixChars[random(strlen(matrixChars))];
          break;
        case 3: // Glitch characters
          stormDrops[i].chars[j] = "!@#$%^&*()<>?/\\|+=~`"[random(20)];
          break;
      }
    }
  }
  stormInitialized = true;
}

// Draw Matrix Storm hybrid effect
void drawMatrixStorm() {
  if (!stormInitialized) {
    initMatrixStorm();
  }
  
  unsigned long currentTime = millis();
  stormTime += 0.03;
  
  // Periodic clear with fade effect (simpler approach)
  static int fadeCounter = 0;
  fadeCounter++;
  if (fadeCounter > 8) { // Clear occasionally to create fade effect
    fadeCounter = 0;
    // Draw a semi-transparent overlay by filling with very dark pixels randomly
    for (int i = 0; i < 200; i++) {
      int x = random(240);
      int y = random(240);
      gfx->drawPixel(x, y, BLACK);
    }
  }
  
  // Update and draw storm drops
  for (int i = 0; i < STORM_DROPS; i++) {
    StormDrop* drop = &stormDrops[i];
    
    if (currentTime - drop->lastUpdate > 50) { // 20 FPS update
      drop->lastUpdate = currentTime;
      drop->y += drop->speed;
      
      // Check for glitch mode trigger
      if (!drop->glitchMode && random(1000) < 3) {
        drop->glitchMode = true;
        drop->glitchEnd = currentTime + random(200, 800);
      }
      
      // Turn off glitch mode
      if (drop->glitchMode && currentTime > drop->glitchEnd) {
        drop->glitchMode = false;
      }
      
      // Reset drop if off screen
      if (drop->y > 280) {
        drop->x = random(240);
        drop->y = random(-300, -50);
        drop->speed = random(20, 80) / 10.0;
        drop->length = random(6, 18);
        drop->charType = random(4);
        drop->colorType = random(4);
        drop->glitchMode = false;
        
        // Refresh character trail
        for (int j = 0; j < drop->length && j < 20; j++) {
          switch (drop->charType) {
            case 0: // Katakana
              drop->chars[j] = xscreenChars[random(strlen(xscreenChars))];
              break;
            case 1: // Binary
              drop->chars[j] = (random(2)) ? '0' : '1';
              break;
            case 2: // Symbols
              drop->chars[j] = matrixChars[random(strlen(matrixChars))];
              break;
            case 3: // Glitch characters
              drop->chars[j] = "!@#$%^&*()<>?/\\|+=~`"[random(20)];
              break;
          }
        }
      }
      
      // Randomly update some characters in the trail
      if (random(100) < 15) {
        int charIdx = random(drop->length);
        if (charIdx < 20) {
          switch (drop->charType) {
            case 0:
              drop->chars[charIdx] = xscreenChars[random(strlen(xscreenChars))];
              break;
            case 1:
              drop->chars[charIdx] = (random(2)) ? '0' : '1';
              break;
            case 2:
              drop->chars[charIdx] = matrixChars[random(strlen(matrixChars))];
              break;
            case 3:
              drop->chars[charIdx] = "!@#$%^&*()<>?/\\|+=~`"[random(20)];
              break;
          }
        }
      }
    }
    
    // Draw the drop trail
    gfx->setTextSize(1);
    for (int j = 0; j < drop->length && j < 20; j++) {
      int charY = drop->y - (j * 12);
      if (charY < -10 || charY > 250) continue;
      
      // Calculate intensity with trail fade
      int intensity = 255 - (j * 200 / drop->length);
      intensity = max(intensity, 30);
      
      // Apply glitch effect
      if (drop->glitchMode) {
        intensity = random(100, 255);
      }
      
      // Color selection with dynamic effects
      uint16_t color;
      float colorShift = sin(stormTime + drop->x * 0.01) * 0.5 + 0.5;
      
      if (j == 0) {
        // Head character - always bright
        if (drop->glitchMode) {
          color = gfx->color565(255, random(150, 255), random(150, 255));
        } else {
          color = gfx->color565(255, 255, 255); // White head
        }
      } else {
        // Trail colors
        switch (drop->colorType) {
          case 0: // Classic green with cyan highlights
            if (drop->glitchMode) {
              color = gfx->color565(intensity, 0, intensity);
            } else {
              color = gfx->color565(0, intensity, intensity * colorShift);
            }
            break;
          case 1: // Blue-purple spectrum
            color = gfx->color565(intensity * colorShift, 0, intensity);
            break;
          case 2: // Orange-red spectrum  
            color = gfx->color565(intensity, intensity * colorShift, 0);
            break;
          case 3: // Rainbow cycling
            float hue = stormTime + drop->x * 0.01 + j * 0.1;
            color = gfx->color565(
              (sin(hue) * 0.5 + 0.5) * intensity,
              (sin(hue + 2.09) * 0.5 + 0.5) * intensity,
              (sin(hue + 4.18) * 0.5 + 0.5) * intensity
            );
            break;
        }
      }
      
      gfx->setTextColor(color);
      gfx->setCursor(drop->x, charY);
      if (j < 20) {
        gfx->print(drop->chars[j]);
      }
    }
  }
  
  // Occasional screen-wide glitch effects
  if (random(1000) < 2) {
    int glitchY = random(240);
    int glitchHeight = random(10, 40);
    uint16_t glitchColor = gfx->color565(random(100, 255), random(0, 150), random(100, 255));
    
    gfx->setTextColor(glitchColor);
    for (int x = 0; x < 240; x += 16) {
      for (int y = glitchY; y < glitchY + glitchHeight && y < 135; y += 12) {
        char c = "ERROR404SYSTEM_FAILURE"[random(22)];
        gfx->setCursor(x, y);
        gfx->print(c);
      }
    }
  }
}

// Draw Lyapunov Fractal effect
void drawLyapunov() {
  static bool initialized = false;
  static unsigned long lastUpdate = 0;
  static int currentLine = 0;
  
  // Full render every 13 seconds, or initialize
  if (!initialized || (millis() - lastUpdate > 13000)) {
    gfx->fillScreen(BLACK);
    currentLine = 0;
    initialized = true;
    lastUpdate = millis();
  }
  
  // Render 12 lines per frame for faster drawing
  for (int lineCount = 0; lineCount < 12 && currentLine < 135; lineCount++, currentLine++) {
    int py = currentLine;
    
    // Lyapunov parameters (varies slightly over time)
    float timeOffset = millis() * 0.00005;
    float min_a = 2.8 + sin(timeOffset) * 0.3;
    float max_a = 3.8 + cos(timeOffset * 1.3) * 0.2;
    float min_b = 2.9 + cos(timeOffset * 0.7) * 0.2; 
    float max_b = 3.7 + sin(timeOffset * 1.1) * 0.3;
    
    float a_range = max_a - min_a;
    float b_range = max_b - min_b;
    
    int settle = 15;  // iterations to settle
    int dwell = 25;   // iterations to calculate
    float start_x = 0.5;
    
    // Forcing sequence (Morse-Thue inspired)
    int forcing[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    int forcing_len = 16;
    
    for (int px = 0; px < 240; px++) {
      // Map pixel to parameter space
      float a = min_a + (px * a_range) / 240.0;
      float b = min_b + (py * b_range) / 135.0;
      
      float x = start_x;
      
      // Settle phase - let the system stabilize
      for (int i = 0; i < settle; i++) {
        float r = forcing[i % forcing_len] ? b : a;
        x = r * x * (1.0 - x);  // logistic map
        if (x <= 0.001 || x >= 0.999) break; // Avoid edge cases
      }
      
      // Calculate Lyapunov exponent
      float total = 0.0;
      int valid_iterations = 0;
      
      for (int i = 0; i < dwell; i++) {
        float r = forcing[i % forcing_len] ? b : a;
        x = r * x * (1.0 - x);  // logistic map
        
        if (x > 0.001 && x < 0.999) {
          float derivative = r - (2.0 * r * x);  // derivative of logistic map
          if (abs(derivative) > 0.001) {
            total += log(abs(derivative));
            valid_iterations++;
          }
        } else {
          break; // System went unstable
        }
      }
      
      if (valid_iterations > 0) {
        float lyapunov = total / valid_iterations;
        
        // Enhanced color mapping with time-based shifts
        uint16_t color;
        float colorTime = millis() * 0.001;
        
        if (lyapunov < -0.1) {
          // Strongly stable - deep blues shifting to purples
          float intensity = constrain((-lyapunov - 0.1) * 100, 0, 255);
          float hueShift = sin(colorTime * 0.3) * 30;
          color = gfx->color565(
            (int)(sin((240 + hueShift) * PI/180) * 0.5 + 0.5) * intensity,
            (int)(sin((240 + hueShift + 120) * PI/180) * 0.5 + 0.5) * intensity,
            intensity
          );
        } else if (lyapunov < 0) {
          // Weakly stable - cyan to blue
          float intensity = constrain((-lyapunov) * 200, 50, 255);
          color = gfx->color565(0, intensity * 0.7, intensity);
        } else if (lyapunov < 0.1) {
          // Near zero - green (edge of chaos)
          float intensity = constrain((0.1 - abs(lyapunov)) * 500, 100, 255);
          color = gfx->color565(intensity * 0.3, intensity, intensity * 0.2);
        } else {
          // Chaotic - warm colors (reds, oranges, yellows)
          float intensity = constrain(lyapunov * 80, 0, 255);
          float hueShift = sin(colorTime * 0.5) * 60;
          if (lyapunov > 0.3) {
            // Highly chaotic - bright reds and whites
            color = gfx->color565(intensity, intensity * 0.3, intensity * 0.1);
          } else {
            // Moderately chaotic - oranges and yellows
            color = gfx->color565(intensity, intensity * 0.6, 0);
          }
        }
        
        gfx->drawPixel(px, py, color);
      } else {
        // No valid calculation - use dark gray
        gfx->drawPixel(px, py, gfx->color565(20, 20, 20));
      }
    }
    
    // Check for mode change during rendering - M5StickC uses buttons instead
    M5.update();
    if (M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()) return;
  }
  
  // Reset line counter when we reach the bottom
  if (currentLine >= 135) {
    currentLine = 0;
    delay(1000); // Pause before regenerating
  }
}

// Multi-layer plasma effect with interference patterns
void drawPlasmaEffect() {
  static float time = 0;
  static float phase1 = 0;
  static float phase2 = 0;
  static float phase3 = 0;
  static int colorCycle = 0;
  static unsigned long lastColorChange = 0;
  
  time += 0.08;
  phase1 += 0.04;
  phase2 += 0.06;
  phase3 += 0.03;
  
  // Color palette cycling every 5 seconds
  if (millis() - lastColorChange > 5000) {
    colorCycle = (colorCycle + 1) % 5;
    lastColorChange = millis();
  }
  
  // Render plasma field
  for (int y = 0; y < 135; y += 2) { // Skip every other line for speed
    for (int x = 0; x < 240; x += 2) { // Skip every other pixel for speed
      
      // Multiple sine wave layers for interference
      float nx = (float)x / 40.0;  // Normalized coordinates
      float ny = (float)y / 30.0;
      
      // Primary plasma layer - large waves
      float plasma1 = sin(nx * 2.0 + time) + 
                     cos(ny * 1.5 + phase1) + 
                     sin(sqrt(nx*nx + ny*ny) * 3.0 + time);
      
      // Secondary layer - interference patterns  
      float plasma2 = sin((nx + ny) * 1.8 + phase2) +
                     cos((nx - ny) * 2.2 + time * 0.7) +
                     sin(nx * 3.5 + cos(ny * 2.8 + phase3));
      
      // Tertiary layer - fine detail
      float plasma3 = sin(nx * 4.0 + time * 1.2) * cos(ny * 3.7 + phase1) +
                     cos((nx * ny) * 0.8 + phase2) +
                     sin(distance(nx, ny, 8.0, 6.0) * 4.0 + time * 0.9);
      
      // Combine layers with different weights
      float combined = (plasma1 * 0.5) + (plasma2 * 0.3) + (plasma3 * 0.2);
      
      // Normalize to 0-1 range
      combined = (combined + 3.0) / 6.0;
      combined = constrain(combined, 0.0, 1.0);
      
      // Apply color palettes
      uint16_t color;
      switch (colorCycle) {
        case 0: // Fire palette
          {
            int r = (int)(255 * pow(combined, 0.8));
            int g = (int)(255 * pow(combined, 2.0));
            int b = (int)(255 * pow(combined, 4.0));
            color = gfx->color565(r, g, b);
          }
          break;
        case 1: // Ocean palette  
          {
            int r = (int)(255 * pow(combined, 4.0));
            int g = (int)(255 * pow(combined, 1.5));
            int b = (int)(255 * pow(combined, 0.6));
            color = gfx->color565(r, g, b);
          }
          break;
        case 2: // Electric palette
          {
            int r = (int)(255 * sin(combined * PI) * combined);
            int g = (int)(255 * combined);
            int b = (int)(255 * (1.0 - combined) * combined * 2);
            color = gfx->color565(r, g, b);
          }
          break;
        case 3: // Cosmic palette
          {
            int r = (int)(255 * (0.5 + 0.5 * sin(combined * PI * 2 + time)));
            int g = (int)(255 * (0.5 + 0.5 * cos(combined * PI * 3 + phase1)));  
            int b = (int)(255 * (0.5 + 0.5 * sin(combined * PI * 4 + phase2)));
            color = gfx->color565(r, g, b);
          }
          break;
        case 4: // Nebula palette
          {
            float rainbow = combined * 6.0;
            int section = (int)rainbow;
            float fract = rainbow - section;
            
            int r, g, b;
            switch(section % 6) {
              case 0: r = 255; g = (int)(255*fract); b = 0; break;
              case 1: r = (int)(255*(1-fract)); g = 255; b = 0; break;
              case 2: r = 0; g = 255; b = (int)(255*fract); break;
              case 3: r = 0; g = (int)(255*(1-fract)); b = 255; break;
              case 4: r = (int)(255*fract); g = 0; b = 255; break;
              default: r = 255; g = 0; b = (int)(255*(1-fract)); break;
            }
            
            // Add brightness modulation
            float brightness = 0.3 + 0.7 * combined;
            r = (int)(r * brightness);
            g = (int)(g * brightness);
            b = (int)(b * brightness);
            
            color = gfx->color565(r, g, b);
          }
          break;
      }
      
      // Draw 2x2 pixel blocks for speed
      gfx->drawPixel(x, y, color);
      gfx->drawPixel(x+1, y, color);
      gfx->drawPixel(x, y+1, color);
      gfx->drawPixel(x+1, y+1, color);
    }
    
    // Allow interruption for mode switching - M5StickC uses buttons
    M5.update();
    if (M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()) return;
  }
}

// Initialize the 3D Sphere effect
void initSphere3D() {
  sphereInitialized = true;
  
  // Initialize sphere properties
  sphere.radius = 80.0; // Large sphere
  sphere.centerX = 120;  // M5StickC screen center X (240/2)
  sphere.centerY = 67;   // M5StickC screen center Y (135/2)
  
  // Initial rotation angles
  sphere.rotX = 0.0;
  sphere.rotY = 0.0; 
  sphere.rotZ = 0.0;
  
  // Rotation speeds (degrees per frame)
  sphere.dRotX = 0.5 + random(15) / 10.0; // 0.5-2.0 deg/frame
  sphere.dRotY = 1.0 + random(20) / 10.0; // 1.0-3.0 deg/frame
  sphere.dRotZ = 0.3 + random(10) / 10.0; // 0.3-1.3 deg/frame
  
  sphere.time = 0.0;
  sphere.lastUpdate = millis();
  
  // Choose surface type randomly
  sphere.surfaceType = random(4); // 0-3 different styles
  
  // Color based on surface type
  switch(sphere.surfaceType) {
    case 0: sphere.baseColor = gfx->color565(0, 255, 255); break;    // Cyan wireframe
    case 1: sphere.baseColor = gfx->color565(255, 100, 0); break;    // Orange solid
    case 2: sphere.baseColor = gfx->color565(100, 255, 100); break;  // Green textured
    case 3: sphere.baseColor = gfx->color565(255, 255, 255); break;  // White disco
  }
  
  // Clear screen with black space background
  gfx->fillScreen(BLACK);
}

// Draw 3D rotating sphere with different surface styles
void drawSphere3D() {
  if (!sphereInitialized) {
    initSphere3D();
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - sphere.lastUpdate < 50) return; // ~20fps for smooth rotation
  
  // Clear screen
  gfx->fillScreen(BLACK);
  
  // Update rotation angles
  sphere.rotX += sphere.dRotX;
  sphere.rotY += sphere.dRotY;
  sphere.rotZ += sphere.dRotZ;
  sphere.time += 0.1;
  
  // Keep angles in 0-360 range
  while (sphere.rotX >= 360.0) sphere.rotX -= 360.0;
  while (sphere.rotY >= 360.0) sphere.rotY -= 360.0;
  while (sphere.rotZ >= 360.0) sphere.rotZ -= 360.0;
  
  // Convert to radians
  float rx = sphere.rotX * PI / 180.0;
  float ry = sphere.rotY * PI / 180.0;
  float rz = sphere.rotZ * PI / 180.0;
  
  // Draw sphere using latitude/longitude grid
  int latitudes = 16;  // Number of horizontal lines
  int longitudes = 24; // Number of vertical lines
  
  for (int lat = 0; lat < latitudes; lat++) {
    for (int lon = 0; lon < longitudes; lon++) {
      
      // Calculate sphere coordinates
      float phi = (lat / (float)latitudes) * PI;           // Latitude angle (0 to PI)
      float theta = (lon / (float)longitudes) * 2 * PI;    // Longitude angle (0 to 2*PI)
      
      // 3D sphere point (before rotation)
      float x = sphere.radius * sin(phi) * cos(theta);
      float y = sphere.radius * sin(phi) * sin(theta);
      float z = sphere.radius * cos(phi);
      
      // Apply 3D rotations
      // Rotate around X axis
      float y1 = y * cos(rx) - z * sin(rx);
      float z1 = y * sin(rx) + z * cos(rx);
      
      // Rotate around Y axis
      float x2 = x * cos(ry) + z1 * sin(ry);
      float z2 = -x * sin(ry) + z1 * cos(ry);
      
      // Rotate around Z axis
      float x3 = x2 * cos(rz) - y1 * sin(rz);
      float y3 = x2 * sin(rz) + y1 * cos(rz);
      
      // Project 3D point to 2D screen (simple orthographic projection)
      int screenX = sphere.centerX + (int)x3;
      int screenY = sphere.centerY + (int)y3;
      
      // Check if point is visible (front-facing)
      if (z2 > 0 && screenX >= 0 && screenX < 240 && screenY >= 0 && screenY < 135) {
        
        // Calculate color/brightness based on surface type and depth
        uint16_t pixelColor;
        float depth = (z2 + sphere.radius) / (2.0 * sphere.radius); // 0.0 to 1.0
        
        switch (sphere.surfaceType) {
          
          case 0: // Wireframe mode
            if ((lat % 2 == 0) || (lon % 3 == 0)) {
              float brightness = 0.5 + 0.5 * depth;
              int r = (sphere.baseColor >> 11) * brightness;
              int g = ((sphere.baseColor >> 5) & 0x3F) * brightness;
              int b = (sphere.baseColor & 0x1F) * brightness;
              pixelColor = gfx->color565(r << 3, g << 2, b << 3);
            } else {
              continue; // Skip non-wireframe pixels
            }
            break;
            
          case 1: // Solid shaded mode
            {
              float brightness = 0.3 + 0.7 * depth;
              int r = ((sphere.baseColor >> 11) & 0x1F) * brightness;
              int g = ((sphere.baseColor >> 5) & 0x3F) * brightness;  
              int b = (sphere.baseColor & 0x1F) * brightness;
              pixelColor = gfx->color565(r << 3, g << 2, b << 3);
            }
            break;
            
          case 2: // Textured/patterned mode
            {
              // Create texture pattern based on coordinates
              bool pattern = ((lat + lon + (int)(sphere.time * 2)) % 3) == 0;
              float brightness = 0.4 + 0.6 * depth;
              if (pattern) brightness *= 1.5;
              
              int r = ((sphere.baseColor >> 11) & 0x1F) * brightness;
              int g = ((sphere.baseColor >> 5) & 0x3F) * brightness;
              int b = (sphere.baseColor & 0x1F) * brightness;
              pixelColor = gfx->color565(r << 3, g << 2, b << 3);
            }
            break;
            
          case 3: // Disco ball mode
            {
              // Mirror tiles that reflect different colors
              bool isMirror = ((lat % 2) == (lon % 2));
              if (isMirror) {
                // Bright mirror tile
                float hue = (lat * 30 + lon * 15 + sphere.time * 10); 
                while (hue >= 360) hue -= 360;
                
                // Simple HSV to RGB conversion for rainbow effect
                int h = (int)(hue / 60) % 6;
                float f = (hue / 60) - h;
                int p = 0;
                int q = (int)(255 * (1 - f) * depth);
                int t = (int)(255 * f * depth);
                int v = (int)(255 * depth);
                
                switch(h) {
                  case 0: pixelColor = gfx->color565(v, t, p); break;
                  case 1: pixelColor = gfx->color565(q, v, p); break;
                  case 2: pixelColor = gfx->color565(p, v, t); break;
                  case 3: pixelColor = gfx->color565(p, q, v); break;
                  case 4: pixelColor = gfx->color565(t, p, v); break;
                  default: pixelColor = gfx->color565(v, p, q); break;
                }
              } else {
                // Dark grout between mirrors
                pixelColor = gfx->color565(20, 20, 20);
              }
            }
            break;
        }
        
        // Draw the pixel with some anti-aliasing
        gfx->drawPixel(screenX, screenY, pixelColor);
        if (depth > 0.7) { // Only anti-alias front-facing surfaces
          gfx->drawPixel(screenX + 1, screenY, pixelColor);
          gfx->drawPixel(screenX, screenY + 1, pixelColor);
        }
      }
    }
  }
  
  sphere.lastUpdate = currentTime;
}

// Initialize the Enhanced Wireframe Sphere effect
void initWireSphere() {
  wireSphereInitialized = true;
  
  // Initialize sphere properties
  wireSphere.radius = 90.0; // Slightly larger than dot sphere
  wireSphere.centerX = 120;  // M5StickC center X
  wireSphere.centerY = 67;   // M5StickC center Y
  
  // Different rotation speeds for variety
  wireSphere.rotX = 0.0;
  wireSphere.rotY = 0.0;
  wireSphere.rotZ = 0.0;
  
  // Slower, more hypnotic rotation
  wireSphere.dRotX = 0.3 + random(8) / 10.0;  // 0.3-1.1 deg/frame
  wireSphere.dRotY = 0.5 + random(10) / 10.0; // 0.5-1.5 deg/frame
  wireSphere.dRotZ = 0.2 + random(6) / 10.0;  // 0.2-0.8 deg/frame
  
  // Color and animation
  wireSphere.colorShift = 0.0;
  wireSphere.pulsePhase = 0.0;
  wireSphere.lastUpdate = millis();
  
  // Choose wireframe mode
  wireSphere.wireMode = random(4); // 0-3 different styles
  wireSphere.lineThickness = 1.0;
  
  // Clear screen
  gfx->fillScreen(BLACK);
}

// Draw enhanced wireframe sphere with color shifting lines
void drawWireSphere() {
  if (!wireSphereInitialized) {
    initWireSphere();
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - wireSphere.lastUpdate < 60) return; // ~16fps for smooth color transitions
  
  // Clear screen with subtle gradient background
  gfx->fillScreen(gfx->color565(5, 5, 15)); // Deep space blue
  
  // Update rotation and animation
  wireSphere.rotX += wireSphere.dRotX;
  wireSphere.rotY += wireSphere.dRotY;
  wireSphere.rotZ += wireSphere.dRotZ;
  wireSphere.colorShift += 2.0; // Color cycle speed
  wireSphere.pulsePhase += 0.15;
  
  // Keep values in range
  while (wireSphere.rotX >= 360.0) wireSphere.rotX -= 360.0;
  while (wireSphere.rotY >= 360.0) wireSphere.rotY -= 360.0;
  while (wireSphere.rotZ >= 360.0) wireSphere.rotZ -= 360.0;
  while (wireSphere.colorShift >= 360.0) wireSphere.colorShift -= 360.0;
  
  // Dynamic radius pulsing
  float pulseRadius = wireSphere.radius + sin(wireSphere.pulsePhase) * 15.0;
  
  // Convert to radians
  float rx = wireSphere.rotX * PI / 180.0;
  float ry = wireSphere.rotY * PI / 180.0;
  float rz = wireSphere.rotZ * PI / 180.0;
  
  // Draw wireframe based on mode
  switch (wireSphere.wireMode) {
    
    case 0: // Latitude/Longitude grid with color shifting
      {
        int latLines = 12;
        int lonLines = 16;
        
        // Draw latitude lines (horizontal circles)
        for (int lat = 1; lat < latLines; lat++) {
          float phi = (lat / (float)latLines) * PI;
          float circleRadius = sin(phi) * pulseRadius;
          float circleY = cos(phi) * pulseRadius;
          
          // Color based on latitude and time
          float hue = wireSphere.colorShift + lat * 30;
          uint16_t color = hsvToRgb565(hue, 0.8, 0.9);
          
          drawWireCircle3D(0, circleY, circleRadius, rx, ry, rz, color);
        }
        
        // Draw longitude lines (vertical semicircles)
        for (int lon = 0; lon < lonLines; lon++) {
          float theta = (lon / (float)lonLines) * 2 * PI;
          
          // Color based on longitude and time
          float hue = wireSphere.colorShift + lon * 22.5;
          uint16_t color = hsvToRgb565(hue, 0.7, 0.8);
          
          drawWireMeridian3D(theta, pulseRadius, rx, ry, rz, color);
        }
      }
      break;
      
    case 1: // Geodesic triangular mesh
      {
        int segments = 8;
        float hueBase = wireSphere.colorShift;
        
        for (int i = 0; i < segments; i++) {
          for (int j = 0; j < segments; j++) {
            float phi1 = (i / (float)segments) * PI;
            float phi2 = ((i + 1) / (float)segments) * PI;
            float theta1 = (j / (float)segments) * 2 * PI;
            float theta2 = ((j + 1) / (float)segments) * 2 * PI;
            
            // Calculate colors with shifting patterns
            float hue = hueBase + i * 40 + j * 25;
            uint16_t color = hsvToRgb565(hue, 0.9, 0.7);
            
            // Draw triangle edges
            drawWireLine3D(phi1, theta1, phi2, theta1, pulseRadius, rx, ry, rz, color);
            drawWireLine3D(phi1, theta1, phi1, theta2, pulseRadius, rx, ry, rz, color);
            drawWireLine3D(phi1, theta2, phi2, theta2, pulseRadius, rx, ry, rz, color);
          }
        }
      }
      break;
      
    case 2: // Spiral pattern
      {
        int spirals = 6;
        int pointsPerSpiral = 32;
        
        for (int s = 0; s < spirals; s++) {
          float hue = wireSphere.colorShift + s * 60;
          uint16_t color = hsvToRgb565(hue, 0.8, 0.9);
          
          for (int p = 0; p < pointsPerSpiral - 1; p++) {
            float t1 = (p / (float)pointsPerSpiral) * 4 * PI;
            float t2 = ((p + 1) / (float)pointsPerSpiral) * 4 * PI;
            
            float phi1 = t1 + s * PI / 3;
            float theta1 = t1 * 3 + s * PI / 3;
            float phi2 = t2 + s * PI / 3;
            float theta2 = t2 * 3 + s * PI / 3;
            
            drawWireLine3D(phi1, theta1, phi2, theta2, pulseRadius, rx, ry, rz, color);
          }
        }
      }
      break;
      
    case 3: // Atomic orbitals pattern
      {
        // Multiple orbital rings at different angles
        for (int orbit = 0; orbit < 8; orbit++) {
          float orbitAngle = orbit * PI / 4;
          float hue = wireSphere.colorShift + orbit * 45;
          uint16_t color = hsvToRgb565(hue, 0.9, 0.8);
          
          drawWireOrbit3D(orbitAngle, pulseRadius * 0.8, rx, ry, rz, color);
        }
      }
      break;
  }
  
  wireSphere.lastUpdate = currentTime;
}

// Helper function to draw a 3D wire circle
void drawWireCircle3D(float centerY, float offsetY, float radius, float rx, float ry, float rz, uint16_t color) {
  int segments = 32;
  for (int i = 0; i < segments; i++) {
    float angle1 = (i / (float)segments) * 2 * PI;
    float angle2 = ((i + 1) / (float)segments) * 2 * PI;
    
    // Circle points
    float x1 = radius * cos(angle1);
    float y1 = centerY + offsetY;
    float z1 = radius * sin(angle1);
    
    float x2 = radius * cos(angle2);
    float y2 = centerY + offsetY;
    float z2 = radius * sin(angle2);
    
    // Apply rotations and project
    drawRotatedLine(x1, y1, z1, x2, y2, z2, rx, ry, rz, color);
  }
}

// Helper function to draw a 3D meridian (longitude line)
void drawWireMeridian3D(float theta, float radius, float rx, float ry, float rz, uint16_t color) {
  int segments = 24;
  for (int i = 0; i < segments; i++) {
    float phi1 = (i / (float)segments) * PI;
    float phi2 = ((i + 1) / (float)segments) * PI;
    
    float x1 = radius * sin(phi1) * cos(theta);
    float y1 = radius * cos(phi1);
    float z1 = radius * sin(phi1) * sin(theta);
    
    float x2 = radius * sin(phi2) * cos(theta);
    float y2 = radius * cos(phi2);
    float z2 = radius * sin(phi2) * sin(theta);
    
    drawRotatedLine(x1, y1, z1, x2, y2, z2, rx, ry, rz, color);
  }
}

// Helper function to draw line between two spherical coordinates
void drawWireLine3D(float phi1, float theta1, float phi2, float theta2, float radius, float rx, float ry, float rz, uint16_t color) {
  float x1 = radius * sin(phi1) * cos(theta1);
  float y1 = radius * cos(phi1);
  float z1 = radius * sin(phi1) * sin(theta1);
  
  float x2 = radius * sin(phi2) * cos(theta2);
  float y2 = radius * cos(phi2);
  float z2 = radius * sin(phi2) * sin(theta2);
  
  drawRotatedLine(x1, y1, z1, x2, y2, z2, rx, ry, rz, color);
}

// Helper function to draw orbital rings
void drawWireOrbit3D(float orbitAngle, float radius, float rx, float ry, float rz, uint16_t color) {
  int segments = 48;
  for (int i = 0; i < segments; i++) {
    float t1 = (i / (float)segments) * 2 * PI;
    float t2 = ((i + 1) / (float)segments) * 2 * PI;
    
    // Orbital ring in tilted plane
    float x1 = radius * cos(t1);
    float y1 = radius * sin(t1) * cos(orbitAngle);
    float z1 = radius * sin(t1) * sin(orbitAngle);
    
    float x2 = radius * cos(t2);
    float y2 = radius * sin(t2) * cos(orbitAngle);
    float z2 = radius * sin(t2) * sin(orbitAngle);
    
    drawRotatedLine(x1, y1, z1, x2, y2, z2, rx, ry, rz, color);
  }
}

// Helper function to apply 3D rotation and draw line
void drawRotatedLine(float x1, float y1, float z1, float x2, float y2, float z2, float rx, float ry, float rz, uint16_t color) {
  // Rotate first point
  float y1r = y1 * cos(rx) - z1 * sin(rx);
  float z1r = y1 * sin(rx) + z1 * cos(rx);
  float x1r = x1 * cos(ry) + z1r * sin(ry);
  float z1r2 = -x1 * sin(ry) + z1r * cos(ry);
  float x1f = x1r * cos(rz) - y1r * sin(rz);
  float y1f = x1r * sin(rz) + y1r * cos(rz);
  
  // Rotate second point
  float y2r = y2 * cos(rx) - z2 * sin(rx);
  float z2r = y2 * sin(rx) + z2 * cos(rx);
  float x2r = x2 * cos(ry) + z2r * sin(ry);
  float z2r2 = -x2 * sin(ry) + z2r * cos(ry);
  float x2f = x2r * cos(rz) - y2r * sin(rz);
  float y2f = x2r * sin(rz) + y2r * cos(rz);
  
  // Project to screen and draw if both points are visible
  if (z1r2 > -wireSphere.radius/2 && z2r2 > -wireSphere.radius/2) {
    int sx1 = wireSphere.centerX + (int)x1f;
    int sy1 = wireSphere.centerY + (int)y1f;
    int sx2 = wireSphere.centerX + (int)x2f;
    int sy2 = wireSphere.centerY + (int)y2f;
    
    if (sx1 >= 0 && sx1 < 240 && sy1 >= 0 && sy1 < 135 &&
        sx2 >= 0 && sx2 < 240 && sy2 >= 0 && sy2 < 135) {
      
      // Draw line with Bresenham's algorithm
      drawLine(sx1, sy1, sx2, sy2, color);
    }
  }
}

// Simple line drawing function
void drawLine(int x1, int y1, int x2, int y2, uint16_t color) {
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;
  
  int x = x1, y = y1;
  while (true) {
    gfx->drawPixel(x, y, color);
    
    if (x == x2 && y == y2) break;
    
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }
}

// Initialize Spheremonics effect
void initSpheremonics() {
  spheremonicsInitialized = true;
  
  spheremonics.centerX = 120;  // M5StickC center X
  spheremonics.centerY = 67;   // M5StickC center Y
  spheremonics.scale = 40.0; // Moderate size
  
  // Rotation
  spheremonics.rotX = 0.0;
  spheremonics.rotY = 0.0;
  spheremonics.rotZ = 0.0;
  
  // Slow, organic rotation
  spheremonics.dRotX = 0.4 + random(6) / 10.0;  // 0.4-1.0 deg/frame
  spheremonics.dRotY = 0.6 + random(8) / 10.0;  // 0.6-1.4 deg/frame
  spheremonics.dRotZ = 0.2 + random(4) / 10.0;  // 0.2-0.6 deg/frame
  
  // Initialize parameters (start with something interesting)
  spheremonics.m[0] = 1; spheremonics.m[1] = 2;
  spheremonics.m[2] = 1; spheremonics.m[3] = 1; 
  spheremonics.m[4] = 2; spheremonics.m[5] = 1;
  spheremonics.m[6] = 1; spheremonics.m[7] = 1;
  
  // Copy to targets
  for (int i = 0; i < 8; i++) {
    spheremonics.targetM[i] = spheremonics.m[i];
  }
  
  spheremonics.morphProgress = 1.0; // Start with no morph in progress
  spheremonics.colorShift = 0.0;
  spheremonics.renderMode = random(3); // Random rendering style
  
  spheremonics.lastUpdate = millis();
  spheremonics.lastMorph = millis();
  
  gfx->fillScreen(BLACK);
}

// Spheremonics evaluation function
void sphereEval(float theta, float phi, float* x, float* y, float* z) {
  // Interpolate parameters during morphing
  float currentM[8];
  for (int i = 0; i < 8; i++) {
    currentM[i] = spheremonics.m[i] + 
                  (spheremonics.targetM[i] - spheremonics.m[i]) * spheremonics.morphProgress;
  }
  
  // Calculate radius using spherical harmonics formula
  float r = 0.0;
  r += pow(sin(currentM[0] * phi), currentM[1]);
  r += pow(cos(currentM[2] * phi), currentM[3]);
  r += pow(sin(currentM[4] * theta), currentM[5]);
  r += pow(cos(currentM[6] * theta), currentM[7]);
  
  // Clamp radius to prevent extreme values
  r = constrain(r, 0.1, 4.0);
  
  // Convert spherical to cartesian coordinates
  *x = r * sin(phi) * cos(theta) * spheremonics.scale;
  *y = r * cos(phi) * spheremonics.scale;
  *z = r * sin(phi) * sin(theta) * spheremonics.scale;
}

// Draw morphing spheremonics
void drawSpheremonics() {
  if (!spheremonicsInitialized) {
    initSpheremonics();
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - spheremonics.lastUpdate < 80) return; // ~12fps for smooth morphing
  
  // Clear screen with subtle background
  gfx->fillScreen(gfx->color565(8, 4, 16)); // Dark purple space
  
  // Update morphing
  if (spheremonics.morphProgress < 1.0) {
    spheremonics.morphProgress += 0.02; // Slow morph speed
    if (spheremonics.morphProgress >= 1.0) {
      spheremonics.morphProgress = 1.0;
      // Copy targets to current when morph completes
      for (int i = 0; i < 8; i++) {
        spheremonics.m[i] = spheremonics.targetM[i];
      }
    }
  }
  
  // Start new morph every 8 seconds
  if (currentTime - spheremonics.lastMorph > 8000 && spheremonics.morphProgress >= 1.0) {
    // Pick one random parameter to change
    int paramToChange = random(8);
    
    // Move parameter in current direction or choose new direction
    int change = (random(2) == 0) ? -1 : 1;
    spheremonics.targetM[paramToChange] = constrain(
      spheremonics.m[paramToChange] + change, 0, 3);
    
    spheremonics.morphProgress = 0.0;
    spheremonics.lastMorph = currentTime;
  }
  
  // Update rotation and color
  spheremonics.rotX += spheremonics.dRotX;
  spheremonics.rotY += spheremonics.dRotY; 
  spheremonics.rotZ += spheremonics.dRotZ;
  spheremonics.colorShift += 1.5;
  
  // Keep values in range
  while (spheremonics.rotX >= 360.0) spheremonics.rotX -= 360.0;
  while (spheremonics.rotY >= 360.0) spheremonics.rotY -= 360.0;
  while (spheremonics.rotZ >= 360.0) spheremonics.rotZ -= 360.0;
  while (spheremonics.colorShift >= 360.0) spheremonics.colorShift -= 360.0;
  
  // Convert to radians
  float rx = spheremonics.rotX * PI / 180.0;
  float ry = spheremonics.rotY * PI / 180.0;
  float rz = spheremonics.rotZ * PI / 180.0;
  
  // Render the spheremonic surface
  int phiSteps = 24;   // Latitude resolution
  int thetaSteps = 32; // Longitude resolution
  
  switch (spheremonics.renderMode) {
    case 0: // Dot mode
      for (int p = 0; p < phiSteps; p++) {
        for (int t = 0; t < thetaSteps; t++) {
          float phi = (p / (float)phiSteps) * PI;
          float theta = (t / (float)thetaSteps) * 2 * PI;
          
          float x, y, z;
          sphereEval(theta, phi, &x, &y, &z);
          
          // Apply 3D rotations
          float y1 = y * cos(rx) - z * sin(rx);
          float z1 = y * sin(rx) + z * cos(rx);
          float x2 = x * cos(ry) + z1 * sin(ry);
          float z2 = -x * sin(ry) + z1 * cos(ry);
          float x3 = x2 * cos(rz) - y1 * sin(rz);
          float y3 = x2 * sin(rz) + y1 * cos(rz);
          
          // Project to screen
          int sx = spheremonics.centerX + (int)x3;
          int sy = spheremonics.centerY + (int)y3;
          
          if (sx >= 0 && sx < 240 && sy >= 0 && sy < 135 && z2 > -50) {
            // Color based on position and morphing
            float hue = spheremonics.colorShift + p * 15 + t * 11;
            float brightness = 0.5 + 0.5 * (z2 + 50) / 100.0;
            uint16_t color = hsvToRgb565(hue, 0.8, brightness);
            
            // Draw larger dots for better visibility
            gfx->drawPixel(sx, sy, color);
            gfx->drawPixel(sx+1, sy, color);
            gfx->drawPixel(sx, sy+1, color);
            gfx->drawPixel(sx+1, sy+1, color);
          }
        }
      }
      break;
      
    case 1: // Wireframe mode
      // Draw latitude lines
      for (int p = 1; p < phiSteps; p += 2) {
        float phi = (p / (float)phiSteps) * PI;
        float hue = spheremonics.colorShift + p * 20;
        uint16_t color = hsvToRgb565(hue, 0.9, 0.8);
        
        for (int t = 0; t < thetaSteps; t++) {
          float theta1 = (t / (float)thetaSteps) * 2 * PI;
          float theta2 = ((t + 1) / (float)thetaSteps) * 2 * PI;
          
          float x1, y1, z1, x2, y2, z2;
          sphereEval(theta1, phi, &x1, &y1, &z1);
          sphereEval(theta2, phi, &x2, &y2, &z2);
          
          drawSpheremonicLine(x1, y1, z1, x2, y2, z2, rx, ry, rz, color);
        }
      }
      
      // Draw longitude lines
      for (int t = 0; t < thetaSteps; t += 3) {
        float theta = (t / (float)thetaSteps) * 2 * PI;
        float hue = spheremonics.colorShift + t * 12;
        uint16_t color = hsvToRgb565(hue, 0.7, 0.9);
        
        for (int p = 0; p < phiSteps; p++) {
          float phi1 = (p / (float)phiSteps) * PI;
          float phi2 = ((p + 1) / (float)phiSteps) * PI;
          
          float x1, y1, z1, x2, y2, z2;
          sphereEval(theta, phi1, &x1, &y1, &z1);
          sphereEval(theta, phi2, &x2, &y2, &z2);
          
          drawSpheremonicLine(x1, y1, z1, x2, y2, z2, rx, ry, rz, color);
        }
      }
      break;
      
    case 2: // Filled/shaded mode
      for (int p = 0; p < phiSteps - 1; p += 2) {
        for (int t = 0; t < thetaSteps - 1; t += 2) {
          float phi = (p / (float)phiSteps) * PI;
          float theta = (t / (float)thetaSteps) * 2 * PI;
          
          float x, y, z;
          sphereEval(theta, phi, &x, &y, &z);
          
          // Apply rotations
          float y1 = y * cos(rx) - z * sin(rx);
          float z1 = y * sin(rx) + z * cos(rx);
          float x2 = x * cos(ry) + z1 * sin(ry);
          float z2 = -x * sin(ry) + z1 * cos(ry);
          float x3 = x2 * cos(rz) - y1 * sin(rz);
          float y3 = x2 * sin(rz) + y1 * cos(rz);
          
          int sx = spheremonics.centerX + (int)x3;
          int sy = spheremonics.centerY + (int)y3;
          
          if (sx >= 0 && sx < 318 && sy >= 0 && sy < 238 && z2 > -50) {
            float hue = spheremonics.colorShift + p * 18 + t * 10;
            float brightness = 0.4 + 0.6 * (z2 + 50) / 100.0;
            uint16_t color = hsvToRgb565(hue, 0.8, brightness);
            
            // Draw small filled squares for surface effect
            gfx->fillRect(sx, sy, 3, 3, color);
          }
        }
      }
      break;
  }
  
  spheremonics.lastUpdate = currentTime;
}

// Helper function to draw 3D line for spheremonics wireframe
void drawSpheremonicLine(float x1, float y1, float z1, float x2, float y2, float z2, 
                        float rx, float ry, float rz, uint16_t color) {
  // Apply rotations to both points
  // Point 1
  float y1r = y1 * cos(rx) - z1 * sin(rx);
  float z1r = y1 * sin(rx) + z1 * cos(rx);
  float x1r = x1 * cos(ry) + z1r * sin(ry);
  float z1r2 = -x1 * sin(ry) + z1r * cos(ry);
  float x1f = x1r * cos(rz) - y1r * sin(rz);
  float y1f = x1r * sin(rz) + y1r * cos(rz);
  
  // Point 2  
  float y2r = y2 * cos(rx) - z2 * sin(rx);
  float z2r = y2 * sin(rx) + z2 * cos(rx);
  float x2r = x2 * cos(ry) + z2r * sin(ry);
  float z2r2 = -x2 * sin(ry) + z2r * cos(ry);
  float x2f = x2r * cos(rz) - y2r * sin(rz);
  float y2f = x2r * sin(rz) + y2r * cos(rz);
  
  // Draw if both points are reasonably visible
  if (z1r2 > -80 && z2r2 > -80) {
    int sx1 = spheremonics.centerX + (int)x1f;
    int sy1 = spheremonics.centerY + (int)y1f;
    int sx2 = spheremonics.centerX + (int)x2f;
    int sy2 = spheremonics.centerY + (int)y2f;
    
    if (sx1 >= 0 && sx1 < 240 && sy1 >= 0 && sy1 < 135 &&
        sx2 >= 0 && sx2 < 240 && sy2 >= 0 && sy2 < 135) {
      drawLine(sx1, sy1, sx2, sy2, color);
    }
  }
}

// Initialize Dot Morph effect
void initDotMorph() {
  dotMorphInitialized = true;
  
  dotMorph.centerX = 120;  // M5StickC center X  
  dotMorph.centerY = 67;   // M5StickC center Y
  dotMorph.scale = 50.0; // Slightly larger for better dot visibility
  dotMorph.dotDensity = 36; // Dense point cloud
  
  // Slower rotation for better viewing
  dotMorph.rotX = 0.0;
  dotMorph.rotY = 0.0;
  dotMorph.rotZ = 0.0;
  
  dotMorph.dRotX = 0.3 + random(5) / 10.0;  // 0.3-0.8 deg/frame
  dotMorph.dRotY = 0.4 + random(6) / 10.0;  // 0.4-1.0 deg/frame
  dotMorph.dRotZ = 0.2 + random(4) / 10.0;  // 0.2-0.6 deg/frame
  
  // Start with interesting shape
  dotMorph.m[0] = 2; dotMorph.m[1] = 1;
  dotMorph.m[2] = 1; dotMorph.m[3] = 2; 
  dotMorph.m[4] = 1; dotMorph.m[5] = 1;
  dotMorph.m[6] = 2; dotMorph.m[7] = 1;
  
  // Copy to targets
  for (int i = 0; i < 8; i++) {
    dotMorph.targetM[i] = dotMorph.m[i];
  }
  
  dotMorph.morphProgress = 1.0;
  dotMorph.colorShift = 0.0;
  dotMorph.pulseBrightness = 1.0;
  
  dotMorph.lastUpdate = millis();
  dotMorph.lastMorph = millis();
  
  gfx->fillScreen(BLACK);
}

// Dot morph evaluation function
void dotMorphEval(float theta, float phi, float* x, float* y, float* z) {
  // Interpolate parameters during morphing
  float currentM[8];
  for (int i = 0; i < 8; i++) {
    currentM[i] = dotMorph.m[i] + 
                  (dotMorph.targetM[i] - dotMorph.m[i]) * dotMorph.morphProgress;
  }
  
  // Calculate radius using spherical harmonics formula
  float r = 0.0;
  r += pow(sin(currentM[0] * phi), currentM[1]);
  r += pow(cos(currentM[2] * phi), currentM[3]);
  r += pow(sin(currentM[4] * theta), currentM[5]);
  r += pow(cos(currentM[6] * theta), currentM[7]);
  
  // Clamp radius to prevent extreme values
  r = constrain(r, 0.2, 3.5);
  
  // Convert spherical to cartesian coordinates
  *x = r * sin(phi) * cos(theta) * dotMorph.scale;
  *y = r * cos(phi) * dotMorph.scale;
  *z = r * sin(phi) * sin(theta) * dotMorph.scale;
}

// Draw dense dot cloud morphing spheremonics
void drawDotMorph() {
  if (!dotMorphInitialized) {
    initDotMorph();
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - dotMorph.lastUpdate < 70) return; // ~14fps for smooth morphing
  
  // Clear screen with starfield background
  gfx->fillScreen(gfx->color565(2, 2, 8)); // Very dark blue
  
  // Update morphing
  if (dotMorph.morphProgress < 1.0) {
    dotMorph.morphProgress += 0.015; // Slightly slower morph for better visibility
    if (dotMorph.morphProgress >= 1.0) {
      dotMorph.morphProgress = 1.0;
      // Copy targets to current when morph completes
      for (int i = 0; i < 8; i++) {
        dotMorph.m[i] = dotMorph.targetM[i];
      }
    }
  }
  
  // Start new morph every 10 seconds for better observation
  if (currentTime - dotMorph.lastMorph > 10000 && dotMorph.morphProgress >= 1.0) {
    // Pick one random parameter to change
    int paramToChange = random(8);
    
    // Move parameter by 1 in random direction
    int change = (random(2) == 0) ? -1 : 1;
    dotMorph.targetM[paramToChange] = constrain(
      dotMorph.m[paramToChange] + change, 0, 3);
    
    dotMorph.morphProgress = 0.0;
    dotMorph.lastMorph = currentTime;
  }
  
  // Update rotation and effects
  dotMorph.rotX += dotMorph.dRotX;
  dotMorph.rotY += dotMorph.dRotY; 
  dotMorph.rotZ += dotMorph.dRotZ;
  dotMorph.colorShift += 2.0; // Faster color cycling for dots
  dotMorph.pulseBrightness = 0.7 + 0.3 * sin(currentTime * 0.003); // Gentle pulsing
  
  // Keep values in range
  while (dotMorph.rotX >= 360.0) dotMorph.rotX -= 360.0;
  while (dotMorph.rotY >= 360.0) dotMorph.rotY -= 360.0;
  while (dotMorph.rotZ >= 360.0) dotMorph.rotZ -= 360.0;
  while (dotMorph.colorShift >= 360.0) dotMorph.colorShift -= 360.0;
  
  // Convert to radians
  float rx = dotMorph.rotX * PI / 180.0;
  float ry = dotMorph.rotY * PI / 180.0;
  float rz = dotMorph.rotZ * PI / 180.0;
  
  // Render dense dot cloud
  int phiSteps = dotMorph.dotDensity;   // Latitude resolution
  int thetaSteps = dotMorph.dotDensity * 4 / 3; // Longitude resolution (denser)
  
  for (int p = 0; p < phiSteps; p++) {
    for (int t = 0; t < thetaSteps; t++) {
      float phi = (p / (float)phiSteps) * PI;
      float theta = (t / (float)thetaSteps) * 2 * PI;
      
      float x, y, z;
      dotMorphEval(theta, phi, &x, &y, &z);
      
      // Apply 3D rotations
      float y1 = y * cos(rx) - z * sin(rx);
      float z1 = y * sin(rx) + z * cos(rx);
      float x2 = x * cos(ry) + z1 * sin(ry);
      float z2 = -x * sin(ry) + z1 * cos(ry);
      float x3 = x2 * cos(rz) - y1 * sin(rz);
      float y3 = x2 * sin(rz) + y1 * cos(rz);
      
      // Project to screen
      int sx = dotMorph.centerX + (int)x3;
      int sy = dotMorph.centerY + (int)y3;
      
      if (sx >= 0 && sx < 318 && sy >= 0 && sy < 238 && z2 > -80) {
        // Color based on surface position and time
        float hue = dotMorph.colorShift + p * 12 + t * 8;
        
        // Depth-based brightness with pulsing
        float depth = (z2 + 80) / 160.0; // 0.0 to 1.0
        float brightness = (0.4 + 0.6 * depth) * dotMorph.pulseBrightness;
        
        // Surface curvature affects saturation
        float saturation = 0.6 + 0.4 * abs(sin(phi * 3) * cos(theta * 2));
        
        uint16_t color = hsvToRgb565(hue, saturation, brightness);
        
        // Draw different sized dots based on depth and morphing state
        int dotSize = 1;
        if (z2 > 20) dotSize = 2; // Larger dots for front surface
        if (dotMorph.morphProgress < 0.5) dotSize++; // Larger during active morph
        
        switch (dotSize) {
          case 1:
            gfx->drawPixel(sx, sy, color);
            break;
          case 2:
            gfx->drawPixel(sx, sy, color);
            gfx->drawPixel(sx+1, sy, color);
            gfx->drawPixel(sx, sy+1, color);
            gfx->drawPixel(sx+1, sy+1, color);
            break;
          case 3:
            // 3x3 dot for active morphing
            for (int dy = 0; dy < 3; dy++) {
              for (int dx = 0; dx < 3; dx++) {
                if (sx + dx < 240 && sy + dy < 135) {
                  gfx->drawPixel(sx + dx, sy + dy, color);
                }
              }
            }
            break;
        }
      }
    }
  }
  
  // Add some random "star" dots in background for depth
  for (int i = 0; i < 20; i++) {
    int sx = random(240);
    int sy = random(240);
    uint16_t starColor = gfx->color565(
      random(50) + 30, random(50) + 30, random(100) + 50);
    gfx->drawPixel(sx, sy, starColor);
  }
  
  dotMorph.lastUpdate = currentTime;
}

// Wire Cube removed to save RAM and simplify

// Helper function to draw dotted line (used by hypercube)
void drawDottedLine(int x1, int y1, int x2, int y2, uint16_t color) {
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;
  
  int x = x1, y = y1;
  int dotCount = 0;
  
  while (true) {
    // Draw every other pixel for dotted effect
    if ((dotCount % 4) < 2) {
      gfx->drawPixel(x, y, color);
    }
    dotCount++;
    
    if (x == x2 && y == y2) break;
    
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }
}

// Initialize Hypercube (4D tesseract) effect
void initHypercube() {
  hypercubeInitialized = true;
  
  hypercube.centerX = 120;  // M5StickC center X
  hypercube.centerY = 67;   // M5StickC center Y
  hypercube.observerZ = 3.0;
  hypercube.scale = 60.0;
  
  // Initialize 4D rotation angles
  hypercube.angleXY = 0.0; hypercube.angleXZ = 0.0; hypercube.angleYZ = 0.0;
  hypercube.angleXW = 0.0; hypercube.angleYW = 0.0; hypercube.angleZW = 0.0;
  
  // Set rotation speeds (in radians per frame)
  hypercube.dAngleXY = 0.008 + random(10) / 1000.0; // ~0.008-0.018 rad/frame
  hypercube.dAngleXZ = 0.006 + random(8) / 1000.0;  // ~0.006-0.014 rad/frame
  hypercube.dAngleYZ = 0.004 + random(6) / 1000.0;  // ~0.004-0.010 rad/frame
  hypercube.dAngleXW = 0.005 + random(7) / 1000.0;  // ~0.005-0.012 rad/frame
  hypercube.dAngleYW = 0.007 + random(9) / 1000.0;  // ~0.007-0.016 rad/frame
  hypercube.dAngleZW = 0.003 + random(5) / 1000.0;  // ~0.003-0.008 rad/frame
  
  hypercube.colorShift = 0.0;
  hypercube.renderStyle = random(3); // Different rendering modes
  
  // Initialize 4D tesseract vertices (16 points in 4D space)
  // Each vertex is (±1, ±1, ±1, ±1)
  for (int i = 0; i < 16; i++) {
    hypercube.vertices4D[i][0] = (i & 1) ? 1.0 : -1.0;        // X
    hypercube.vertices4D[i][1] = (i & 2) ? 1.0 : -1.0;        // Y
    hypercube.vertices4D[i][2] = (i & 4) ? 1.0 : -1.0;        // Z
    hypercube.vertices4D[i][3] = (i & 8) ? 1.0 : -1.0;        // W
  }
  
  hypercube.lastUpdate = millis();
  gfx->fillScreen(BLACK);
}

// 4D rotation and projection functions
void rotate4D(float* point, float angleXY, float angleXZ, float angleYZ,
              float angleXW, float angleYW, float angleZW) {
  float x = point[0], y = point[1], z = point[2], w = point[3];
  
  // Apply 6 possible 4D rotations
  
  // XY plane rotation
  float cosXY = cos(angleXY), sinXY = sin(angleXY);
  float newX = x * cosXY - y * sinXY;
  float newY = x * sinXY + y * cosXY;
  x = newX; y = newY;
  
  // XZ plane rotation
  float cosXZ = cos(angleXZ), sinXZ = sin(angleXZ);
  newX = x * cosXZ - z * sinXZ;
  float newZ = x * sinXZ + z * cosXZ;
  x = newX; z = newZ;
  
  // YZ plane rotation
  float cosYZ = cos(angleYZ), sinYZ = sin(angleYZ);
  newY = y * cosYZ - z * sinYZ;
  newZ = y * sinYZ + z * cosYZ;
  y = newY; z = newZ;
  
  // XW plane rotation (4D!)
  float cosXW = cos(angleXW), sinXW = sin(angleXW);
  newX = x * cosXW - w * sinXW;
  float newW = x * sinXW + w * cosXW;
  x = newX; w = newW;
  
  // YW plane rotation (4D!)
  float cosYW = cos(angleYW), sinYW = sin(angleYW);
  newY = y * cosYW - w * sinYW;
  newW = y * sinYW + w * cosYW;
  y = newY; w = newW;
  
  // ZW plane rotation (4D!)
  float cosZW = cos(angleZW), sinZW = sin(angleZW);
  newZ = z * cosZW - w * sinZW;
  newW = z * sinZW + w * cosZW;
  z = newZ; w = newW;
  
  point[0] = x; point[1] = y; point[2] = z; point[3] = w;
}

void projectTo2D(float x, float y, float z, float w, int* screenX, int* screenY, float* depth) {
  // First project 4D to 3D (perspective projection along W axis)
  float w_scale = 2.0 / (2.0 + w); // W-distance scaling
  float x3D = x * w_scale;
  float y3D = y * w_scale;
  float z3D = z * w_scale;
  
  // Then project 3D to 2D (perspective projection along Z axis)
  float z_scale = hypercube.scale / (hypercube.observerZ - z3D);
  *screenX = hypercube.centerX + (int)(x3D * z_scale);
  *screenY = hypercube.centerY + (int)(y3D * z_scale);
  *depth = z3D; // Store 3D depth for visibility testing
}

// Draw 4D hypercube (tesseract)
void drawHypercube() {
  if (!hypercubeInitialized) {
    initHypercube();
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - hypercube.lastUpdate < 40) return; // ~25fps
  
  // Clear screen
  gfx->fillScreen(gfx->color565(5, 0, 15)); // Deep purple space
  
  // Update rotation angles
  hypercube.angleXY += hypercube.dAngleXY;
  hypercube.angleXZ += hypercube.dAngleXZ;
  hypercube.angleYZ += hypercube.dAngleYZ;
  hypercube.angleXW += hypercube.dAngleXW;
  hypercube.angleYW += hypercube.dAngleYW;
  hypercube.angleZW += hypercube.dAngleZW;
  hypercube.colorShift += 3.0;
  
  // Keep angles in reasonable range
  if (hypercube.angleXY > 2*PI) hypercube.angleXY -= 2*PI;
  if (hypercube.angleXZ > 2*PI) hypercube.angleXZ -= 2*PI;
  if (hypercube.angleYZ > 2*PI) hypercube.angleYZ -= 2*PI;
  if (hypercube.angleXW > 2*PI) hypercube.angleXW -= 2*PI;
  if (hypercube.angleYW > 2*PI) hypercube.angleYW -= 2*PI;
  if (hypercube.angleZW > 2*PI) hypercube.angleZW -= 2*PI;
  while (hypercube.colorShift >= 360.0) hypercube.colorShift -= 360.0;
  
  // Rotate and project all 16 vertices
  for (int i = 0; i < 16; i++) {
    float rotated[4] = {
      hypercube.vertices4D[i][0],
      hypercube.vertices4D[i][1], 
      hypercube.vertices4D[i][2],
      hypercube.vertices4D[i][3]
    };
    
    // Apply 4D rotation
    rotate4D(rotated, hypercube.angleXY, hypercube.angleXZ, hypercube.angleYZ,
             hypercube.angleXW, hypercube.angleYW, hypercube.angleZW);
    
    // Project to 2D screen
    projectTo2D(rotated[0], rotated[1], rotated[2], rotated[3],
               &hypercube.projected[i][0], &hypercube.projected[i][1],
               &hypercube.depths[i]);
  }
  
  // Draw edges based on tesseract connectivity
  // A tesseract has 32 edges connecting vertices that differ by exactly 1 bit
  int edgeCount = 0;
  for (int i = 0; i < 16; i++) {
    for (int j = i + 1; j < 16; j++) {
      // Check if vertices differ by exactly 1 bit (Hamming distance = 1)
      int diff = i ^ j;
      int bitCount = 0;
      int temp = diff;
      while (temp) {
        bitCount += temp & 1;
        temp >>= 1;
      }
      
      if (bitCount == 1) { // These vertices should be connected
        int x1 = hypercube.projected[i][0];
        int y1 = hypercube.projected[i][1];
        int x2 = hypercube.projected[j][0];
        int y2 = hypercube.projected[j][1];
        
        // Check if both points are on screen
        if (x1 >= 0 && x1 < 240 && y1 >= 0 && y1 < 135 &&
            x2 >= 0 && x2 < 240 && y2 >= 0 && y2 < 135) {
          
          // Calculate edge properties
          float avgDepth = (hypercube.depths[i] + hypercube.depths[j]) / 2.0;
          float hue = hypercube.colorShift + edgeCount * 11;
          float brightness = 0.4 + 0.6 * (avgDepth + 2.0) / 4.0;
          brightness = constrain(brightness, 0.2, 1.0);
          
          uint16_t edgeColor = hsvToRgb565(hue, 0.9, brightness);
          
          switch (hypercube.renderStyle) {
            case 0: // Simple lines
              drawLine(x1, y1, x2, y2, edgeColor);
              break;
              
            case 1: // Thick lines based on depth
              drawLine(x1, y1, x2, y2, edgeColor);
              if (avgDepth > 0) { // Front edges get thickness
                drawLine(x1+1, y1, x2+1, y2, edgeColor);
                drawLine(x1, y1+1, x2, y2+1, edgeColor);
              }
              break;
              
            case 2: // Dotted for back, solid for front
              if (avgDepth > -0.5) {
                drawLine(x1, y1, x2, y2, edgeColor);
              } else {
                drawDottedLine(x1, y1, x2, y2, edgeColor);
              }
              break;
          }
        }
        edgeCount++;
      }
    }
  }
  
  // Draw vertex points
  for (int i = 0; i < 16; i++) {
    int x = hypercube.projected[i][0];
    int y = hypercube.projected[i][1];
    
    if (x >= 2 && x < 318 && y >= 2 && y < 238) {
      float hue = hypercube.colorShift + i * 22.5;
      float brightness = 0.6 + 0.4 * (hypercube.depths[i] + 2.0) / 4.0;
      uint16_t vertexColor = hsvToRgb565(hue, 1.0, brightness);
      
      // Draw vertex as filled circle
      int radius = (hypercube.depths[i] > 0) ? 2 : 1;
      for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
          if (dx*dx + dy*dy <= radius*radius) {
            gfx->drawPixel(x + dx, y + dy, vertexColor);
          }
        }
      }
    }
  }
  
  hypercube.lastUpdate = currentTime;
}

// Initialize XJack effect - simplified version
void initXJack() {
  jackTime = 0;
  gfx->fillScreen(BLACK);
}

// Draw XJack - simplified dramatic version like CYD project
void drawXJack() {
  jackTime += 0.05;
  
  gfx->fillScreen(BLACK);
  
  // "All work and no play makes Jack a dull boy" typing effect
  const char* jackText[] = {
    "All work and no play makes Jack a dull boy.",
    "All work and no play makes Jack a dull boy.",
    "All WORK and no PLAY makes Jack a dull boy.",
    "ALL WORK AND NO PLAY MAKES JACK A DULL BOY!",
    "All work and no play makes Jack a dull boy.",
    "All work and no play makes Jack a dull boy."
  };
  int numLines = sizeof(jackText) / sizeof(jackText[0]);
  
  // Typing animation
  float typingSpeed = jackTime * 10;
  int currentLine = (int)(typingSpeed / 50) % numLines;
  int currentChar = ((int)typingSpeed % 50);
  
  gfx->setTextSize(1);
  
  // Draw previous lines
  for (int line = 0; line < currentLine && line < 20; line++) {
    gfx->setCursor(5, 10 + line * 12);
    
    // Glitch effect on random lines
    if (sin(jackTime * 3 + line) > 0.8) {
      gfx->setTextColor(gfx->color565(255, 100, 100)); // Red glitch
    } else {
      gfx->setTextColor(gfx->color565(200, 200, 200)); // Normal
    }
    
    int lineIndex = line % numLines;
    gfx->print(jackText[lineIndex]);
  }
  
  // Draw current line being typed
  if (currentLine < 20) {
    gfx->setCursor(5, 10 + currentLine * 12);
    gfx->setTextColor(gfx->color565(255, 255, 100)); // Bright typing
    
    char buffer[50];
    int lineLength = strlen(jackText[currentLine % numLines]);
    int charsToShow = min(currentChar, lineLength);
    
    strncpy(buffer, jackText[currentLine % numLines], charsToShow);
    buffer[charsToShow] = '\0';
    gfx->print(buffer);
    
    // Cursor blink
    if (sin(jackTime * 8) > 0) {
      gfx->print("_");
    }
  }
  
  // Typewriter key strike effects
  if (currentChar > 0 && sin(jackTime * 20) > 0.7) {
    for (int strike = 0; strike < 5; strike++) {
      int strikeX = 50 + strike * 40 + sin(jackTime * 15 + strike) * 10;
      int strikeY = 200 + cos(jackTime * 12 + strike) * 5;
      
      gfx->fillRect(strikeX, strikeY, 8, 3, gfx->color565(150, 150, 150));
    }
  }
  
  // Psychological tension effects - RED SCREEN FLASH
  if (sin(jackTime * 0.5) > 0.9) {
    // Screen flicker
    gfx->fillScreen(gfx->color565(255, 0, 0));
    delay(10);
  }
  
  gfx->setCursor(10, 225);
  gfx->setTextColor(gfx->color565(255, 255, 255));
  gfx->print("XJACK - The Shining");
}

// Initialize Abstractile effect - Simple version
void initAbstractile() {
  abstractileTime = 0;
  gfx->fillScreen(BLACK);
}

// Draw Abstractile - Abstract tile patterns like CYD project
void drawAbstractile() {
  abstractileTime += 0.04;
  
  gfx->fillScreen(BLACK);
  
  // Abstract tile patterns
  int tileSize = 20;
  int tilesX = 240 / tileSize + 1;
  int tilesY = 135 / tileSize + 1;
  
  for (int tx = 0; tx < tilesX; tx++) {
    for (int ty = 0; ty < tilesY; ty++) {
      float tileTime = abstractileTime + tx * 0.1 + ty * 0.15;
      
      int x = tx * tileSize;
      int y = ty * tileSize;
      
      // Abstract pattern based on position and time
      float pattern = sin(tileTime + tx * 0.3) * cos(tileTime + ty * 0.4);
      
      // Tile type selection
      int tileType = (int)(pattern * 4 + tileTime * 2) % 6;
      
      uint16_t tileColor = gfx->color565(
        128 + 127 * sin(tileTime + tx * 0.2),
        128 + 127 * cos(tileTime + ty * 0.3),
        128 + 127 * sin(tileTime + (tx + ty) * 0.1)
      );
      
      switch (tileType) {
        case 0: // Solid fill
          gfx->fillRect(x, y, tileSize - 1, tileSize - 1, tileColor);
          break;
          
        case 1: // Diagonal lines
          for (int line = 0; line < tileSize; line += 3) {
            drawLine(x + line, y, x, y + line, tileColor);
            drawLine(x + tileSize, y + line, x + line, y + tileSize, tileColor);
          }
          break;
          
        case 2: // Concentric squares
          for (int square = 2; square < tileSize / 2; square += 3) {
            gfx->drawRect(x + square, y + square, 
                         tileSize - 2 * square, tileSize - 2 * square, tileColor);
          }
          break;
          
        case 3: // Cross pattern
          drawLine(x + tileSize/2, y, x + tileSize/2, y + tileSize, tileColor);
          drawLine(x, y + tileSize/2, x + tileSize, y + tileSize/2, tileColor);
          break;
          
        case 4: // Dots
          for (int dot = 0; dot < 4; dot++) {
            int dotX = x + 4 + (dot % 2) * 10;
            int dotY = y + 4 + (dot / 2) * 10;
            gfx->fillCircle(dotX, dotY, 2, tileColor);
          }
          break;
          
        case 5: // Random pixels
          for (int pixel = 0; pixel < tileSize; pixel++) {
            int pixelX = x + random(tileSize);
            int pixelY = y + random(tileSize);
            gfx->drawPixel(pixelX, pixelY, tileColor);
          }
          break;
      }
      
      // Highlight active tiles
      if (sin(tileTime * 4) > 0.8) {
        gfx->drawRect(x, y, tileSize, tileSize, WHITE);
      }
    }
  }
}

// Initialize Truchet effect - Simple version  
void initTruchet() {
  truchetTime = 0;
  gfx->fillScreen(BLACK);
}

// Draw Truchet - Curved and angular tile patterns like CYD project
void drawTruchet() {
  truchetTime += 0.02;
  
  gfx->fillScreen(BLACK);
  
  // Truchet tile patterns
  int tileSize = 20;
  int tilesX = 240 / tileSize + 1;
  int tilesY = 135 / tileSize + 1;
  
  for (int tileX = 0; tileX < tilesX; tileX++) {
    for (int tileY = 0; tileY < tilesY; tileY++) {
      int x = tileX * tileSize;
      int y = tileY * tileSize;
      
      // Tile animation phase
      float tileTime = truchetTime + tileX * 0.1 + tileY * 0.15;
      
      // Tile type based on position and time
      int tileType = ((int)(tileTime * 2) + tileX + tileY) % 4;
      
      uint16_t tileColor = gfx->color565(
        100 + 100 * sin(tileTime + tileX * 0.3),
        100 + 100 * sin(tileTime + tileY * 0.4), 
        100 + 100 * sin(tileTime + (tileX + tileY) * 0.2)
      );
      
      switch (tileType) {
        case 0: // Quarter circles - top-left to bottom-right
          for (int r = 2; r < tileSize/2; r += 2) {
            gfx->drawCircle(x, y, r, tileColor);
            gfx->drawCircle(x + tileSize, y + tileSize, r, tileColor);
          }
          break;
          
        case 1: // Quarter circles - top-right to bottom-left  
          for (int r = 2; r < tileSize/2; r += 2) {
            gfx->drawCircle(x + tileSize, y, r, tileColor);
            gfx->drawCircle(x, y + tileSize, r, tileColor);
          }
          break;
          
        case 2: // Diagonal lines
          drawLine(x, y, x + tileSize, y + tileSize, tileColor);
          drawLine(x + 2, y, x + tileSize, y + tileSize - 2, tileColor);
          drawLine(x, y + 2, x + tileSize - 2, y + tileSize, tileColor);
          break;
          
        case 3: // Anti-diagonal lines
          drawLine(x + tileSize, y, x, y + tileSize, tileColor);
          drawLine(x + tileSize - 2, y, x, y + tileSize - 2, tileColor);
          drawLine(x + tileSize, y + 2, x + 2, y + tileSize, tileColor);
          break;
      }
      
      // Highlight active tiles
      if (sin(tileTime * 4) > 0.8) {
        gfx->drawRect(x, y, tileSize, tileSize, WHITE);
      }
    }
  }
}

// Initialize Binary Horizon effect
void initBinaryHorizon() {
  horizonTime = 0;
  gfx->fillScreen(BLACK);
}

// Draw Binary Horizon - Binary data streams rising from horizon like CYD project
void drawBinaryHorizon() {
  horizonTime += 0.05;
  
  gfx->fillScreen(BLACK);
  
  // Binary data horizon
  int horizonY = 120 + sin(horizonTime * 0.5) * 20;
  
  // Draw horizon line
  gfx->drawLine(0, horizonY, 240, horizonY, gfx->color565(0, 255, 0));
  
  // Binary data streams above horizon
  for (int stream = 0; stream < 15; stream++) {
    float streamTime = horizonTime * 2 + stream * 0.4;
    int streamX = 20 + stream * 20;
    
    // Data blocks moving up from horizon
    for (int block = 0; block < 8; block++) {
      float blockTime = streamTime + block * 0.3;
      int blockY = horizonY - block * 15 - (int)(streamTime * 20) % 60;
      
      if (blockY > 10 && blockY < horizonY) {
        // Binary digit (0 or 1)
        bool isOne = sin(blockTime * 5 + stream) > 0;
        
        uint16_t digitColor = gfx->color565(
          isOne ? 255 : 100,
          255 - block * 20,
          isOne ? 100 : 255
        );
        
        gfx->setTextSize(1);
        gfx->setTextColor(digitColor);
        gfx->setCursor(streamX, blockY);
        gfx->printf("%d", isOne ? 1 : 0);
        
        // Connecting lines
        if (block > 0) {
          int prevBlockY = horizonY - (block - 1) * 15 - (int)(streamTime * 20) % 60;
          if (prevBlockY < horizonY) {
            gfx->drawLine(streamX + 3, blockY, streamX + 3, prevBlockY + 8, digitColor);
          }
        }
      }
    }
  }
  
  // Binary data streams below horizon (inverted)
  for (int stream = 0; stream < 12; stream++) {
    float streamTime = horizonTime * 1.5 + stream * 0.5;
    int streamX = 25 + stream * 25;
    
    for (int block = 0; block < 5; block++) {
      float blockTime = streamTime + block * 0.4;
      int blockY = horizonY + block * 12 + (int)(streamTime * 15) % 40;
      
      if (blockY < 230 && blockY > horizonY) {
        bool isZero = sin(blockTime * 4 + stream * 0.7) < 0;
        
        uint16_t digitColor = gfx->color565(
          isZero ? 80 : 200,
          150 - block * 15,
          isZero ? 200 : 80
        );
        
        gfx->setTextSize(1);
        gfx->setTextColor(digitColor);
        gfx->setCursor(streamX, blockY);
        gfx->printf("%d", isZero ? 0 : 1);
      }
    }
  }
}

// Initialize Font Glide effect
void initFontglide() {
  glideTime = 0;
  gfx->fillScreen(BLACK);
}

// Draw Font Glide - Flurry of random symbols and letters gliding
void drawFontglide() {
  glideTime += 0.04;
  
  gfx->fillScreen(BLACK);
  
  // Array of random symbols and letters for variety
  const char glideChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()+=<>?";
  int numChars = sizeof(glideChars) - 1; // -1 for null terminator
  
  // Create a flurry of 20 gliding characters
  for (int i = 0; i < 20; i++) {
    // Select character based on time and position for variation
    int charIndex = (int)(glideTime * 3 + i * 7) % numChars;
    char c = glideChars[charIndex];
    
    // Character position with unique wave motion per character
    float charPhase = glideTime + i * 0.5;
    float waveSpeed1 = 0.7 + (i % 5) * 0.1; // Varied wave speeds
    float waveSpeed2 = 0.5 + (i % 7) * 0.08;
    
    // Distributed positions across screen
    int baseX = 20 + (i % 8) * 35; // 8 columns
    int baseY = 40 + (i / 8) * 50;  // Multiple rows
    
    // Complex gliding motion - each character has unique path
    int glideX = baseX + sin(charPhase * waveSpeed1 + i * 0.3) * 40;
    int glideY = baseY + cos(charPhase * waveSpeed2 + i * 0.7) * 30;
    
    // Keep characters on screen
    if (glideX < 10) glideX = 10;
    if (glideX > 300) glideX = 300;
    if (glideY < 10) glideY = 10;
    if (glideY > 220) glideY = 220;
    
    // Character size variation - different patterns per character
    float sizePhase = charPhase * (1.0 + i * 0.1);
    int charSize = 1 + (int)(sin(sizePhase) * 1.5 + 1.5); // 1-3 range
    if (charSize < 1) charSize = 1;
    if (charSize > 3) charSize = 3;
    
    // Individual color cycling for each character
    float colorPhase = charPhase * 2 + i * 0.8;
    int r = 128 + sin(colorPhase) * 127;
    int g = 128 + sin(colorPhase + 2.094 + i * 0.3) * 127;
    int b = 128 + sin(colorPhase + 4.188 + i * 0.5) * 127;
    
    // Draw character with trail effect (shorter for performance)
    for (int trail = 0; trail < 3; trail++) {
      float trailPhase = charPhase - trail * 0.15;
      int trailX = baseX + sin(trailPhase * waveSpeed1 + i * 0.3) * 40;
      int trailY = baseY + cos(trailPhase * waveSpeed2 + i * 0.7) * 30;
      
      // Keep trail on screen too
      if (trailX < 10) trailX = 10;
      if (trailX > 300) trailX = 300;
      if (trailY < 10) trailY = 10;
      if (trailY > 220) trailY = 220;
      
      float trailAlpha = 1.0 - trail / 3.0;
      int trailR = r * trailAlpha;
      int trailG = g * trailAlpha;
      int trailB = b * trailAlpha;
      
      gfx->setTextColor(gfx->color565(trailR, trailG, trailB));
      gfx->setTextSize(charSize);
      gfx->setCursor(trailX - 6, trailY - 8);
      gfx->printf("%c", c);
    }
    
    // Main character
    gfx->setTextColor(gfx->color565(r, g, b));
    gfx->setTextSize(charSize);
    gfx->setCursor(glideX - 6, glideY - 8);
    gfx->printf("%c", c);
  }
}

// Initialize Epicycle effect - Simple version
void initEpicycle() {
  // Simple epicycle needs no complex initialization
  gfx->fillScreen(BLACK);
}

// Draw Epicycle - Enhanced thickness version
void drawEpicycle() {
  gfx->fillScreen(BLACK);
  
  static float epicycleTime = 0;
  epicycleTime += 0.03;
  
  int centerX = gfx->width() / 2;
  int centerY = gfx->height() / 2;
  
  // Main circle (deferent) - double thickness
  int mainRadius = 60;
  uint16_t mainColor = gfx->color565(100, 100, 100);
  gfx->drawCircle(centerX, centerY, mainRadius, mainColor);
  gfx->drawCircle(centerX, centerY, mainRadius + 1, mainColor);
  
  // Multiple epicycles
  for (int epic = 0; epic < 3; epic++) {
    float baseAngle = epicycleTime * (epic + 1) * 0.7;
    int epicycleRadius = 20 + epic * 10;
    
    // Center of epicycle on main circle
    int epicX = centerX + cos(baseAngle) * mainRadius;
    int epicY = centerY + sin(baseAngle) * mainRadius;
    
    // Draw epicycle circle - double thickness
    uint16_t epicColor = gfx->color565(150 - epic * 30, 150 + epic * 30, 255 - epic * 50);
    gfx->drawCircle(epicX, epicY, epicycleRadius, epicColor);
    gfx->drawCircle(epicX, epicY, epicycleRadius + 1, epicColor);
    
    // Point on epicycle
    float epicAngle = epicycleTime * (5 - epic) * 2; // Different speeds
    int pointX = epicX + cos(epicAngle) * epicycleRadius;
    int pointY = epicY + sin(epicAngle) * epicycleRadius;
    
    // Draw the traced path - double thickness trails
    for (int trail = 0; trail < 50; trail++) {
      float trailTime = epicycleTime - trail * 0.02;
      float trailBaseAngle = trailTime * (epic + 1) * 0.7;
      float trailEpicAngle = trailTime * (5 - epic) * 2;
      
      int trailEpicX = centerX + cos(trailBaseAngle) * mainRadius;
      int trailEpicY = centerY + sin(trailBaseAngle) * mainRadius;
      int trailPointX = trailEpicX + cos(trailEpicAngle) * epicycleRadius;
      int trailPointY = trailEpicY + sin(trailEpicAngle) * epicycleRadius;
      
      int alpha = 255 - trail * 5;
      if (alpha > 0) {
        uint16_t trailColor = gfx->color565(alpha/4 + epic * 20, alpha/4, alpha/2);
        // Double thickness trail points
        gfx->fillCircle(trailPointX, trailPointY, 2, trailColor);
      }
    }
    
    // Current point - larger and brighter
    gfx->fillCircle(pointX, pointY, 5, 
                    gfx->color565(255 - epic * 50, 100 + epic * 50, 255));
  }
}

void loop() {
  M5.update(); // Update M5 button states
  
  // Power button (PWR): Previous effect (scroll backward)
  if (M5.BtnPWR.wasPressed()) {
    if (millis() - lastPress > BUTTON_DEBOUNCE) {
      currentMode = (EffectMode)((currentMode - 1 + TOTAL_MODES) % TOTAL_MODES);
      Serial.printf("PWR Button: Previous mode %d\n", currentMode);
      lastPress = millis();
      gfx->fillScreen(BLACK);
      
      // Reinitialize for new mode
      if (currentMode == GLMATRIX_3D) {
        initGLMatrix();
      } else {
        for(int i = 0; i < MATRIX_COLS; i++) {
          initColumn(i);
          columns[i].y = random(-100, gfx->height());
          columnPaused[i] = false;
          pauseStartTime[i] = 0;
          skipDistance[i] = 0;
        }
      }
      lastModeSwitch = millis();
    }
  }
  
  // Button B: Next effect (scroll forward) 
  if (M5.BtnB.wasPressed()) {
    if (millis() - lastPress > BUTTON_DEBOUNCE) {
      currentMode = (EffectMode)((currentMode + 1) % TOTAL_MODES);
      Serial.printf("Button B: Next mode %d\n", currentMode);
      lastPress = millis();
      gfx->fillScreen(BLACK);
      
      // Reinitialize for new mode
      if (currentMode == GLMATRIX_3D) {
        initGLMatrix();
      } else {
        for(int i = 0; i < MATRIX_COLS; i++) {
          initColumn(i);
          columns[i].y = random(-100, gfx->height());
          columnPaused[i] = false;
          pauseStartTime[i] = 0;
          skipDistance[i] = 0;
        }
      }
      lastModeSwitch = millis();
    }
  }
  
  // Button A: Toggle auto-scroll (placeholder for now)
  if (M5.BtnA.wasPressed()) {
    if (millis() - lastPress > BUTTON_DEBOUNCE) {
      autoScroll = !autoScroll;
      Serial.printf("Auto-scroll: %s\n", autoScroll ? "ENABLED (30s per mode)" : "DISABLED");
      
      // Visual feedback
      gfx->fillRect(0, 0, 135, 20, autoScroll ? 0x07E0 : 0xF800); // Green or Red
      gfx->setTextColor(BLACK);
      gfx->setTextSize(1);
      gfx->setCursor(5, 5);
      gfx->printf("AUTO: %s", autoScroll ? "ON" : "OFF");
      delay(1000);
      gfx->fillRect(0, 0, 135, 20, BLACK); // Clear message
      
      lastModeSwitch = millis();
      lastPress = millis();
    }
  }
  
  // Auto-scroll mode switching
  if (autoScroll && (millis() - lastModeSwitch > autoSwitchInterval)) {
    Serial.println("Auto-scroll: switching to next mode");
    currentMode = (EffectMode)((currentMode + 1) % TOTAL_MODES);
    gfx->fillScreen(BLACK);
    
    // Reinitialize for new mode
    if (currentMode == GLMATRIX_3D) {
      initGLMatrix();
    } else {
      for(int i = 0; i < MATRIX_COLS; i++) {
        initColumn(i);
        columns[i].y = random(-100, gfx->height());
        columnPaused[i] = false;
        pauseStartTime[i] = 0;
        skipDistance[i] = 0;
      }
    }
    lastModeSwitch = millis();
  }
  
  // Run the current matrix effect
  drawMatrix();
  
  // Frame rate control
  delay(50);
}

// === SIGNAL PATTERN DETECTION SYSTEM ===

void updateSignalSystem(unsigned long currentTime) {
  // Check if it's time for a new pattern
  if (!signalSystem.currentPattern.active && 
      currentTime - signalSystem.lastTriggerTime > signalSystem.quietPeriod) {
    
    // Random chance to trigger a pattern
    if (random(1000) < (signalSystem.triggerThreshold * 1000)) {
      triggerSignalPattern();
      signalSystem.lastTriggerTime = currentTime;
    }
  }
  
  // Update active pattern
  if (signalSystem.currentPattern.active) {
    unsigned long elapsed = currentTime - signalSystem.currentPattern.startTime;
    
    if (elapsed > signalSystem.currentPattern.duration) {
      // Pattern time is up - start dissolving
      signalSystem.currentPattern.dissolving = true;
      
      // Fade out the pattern
      signalSystem.currentPattern.intensity -= signalSystem.fadeSpeed;
      
      if (signalSystem.currentPattern.intensity <= 0) {
        // Pattern fully dissolved
        signalSystem.currentPattern.active = false;
        signalSystem.currentPattern.dissolving = false;
        signalSystem.quietPeriod = random(15000, 45000); // Next quiet period
        
        Serial.println("🔍 SIGNAL: Pattern dissolved back into chaos");
      }
    } else {
      // Pattern is building or stable
      float progress = elapsed / (float)signalSystem.currentPattern.duration;
      if (progress < 0.3) {
        // Building phase
        signalSystem.currentPattern.intensity = progress / 0.3;
      } else if (progress > 0.7) {
        // Dissolving phase  
        signalSystem.currentPattern.intensity = (1.0 - progress) / 0.3;
      } else {
        // Stable phase
        signalSystem.currentPattern.intensity = 1.0;
      }
    }
  }
}

void triggerSignalPattern() {
  signalSystem.currentPattern.active = true;
  signalSystem.currentPattern.startTime = millis();
  signalSystem.currentPattern.duration = random(1000, 3000);  // 1-3 seconds
  signalSystem.currentPattern.patternType = random(signalSystem.availablePatterns);
  signalSystem.currentPattern.intensity = 0.0;
  signalSystem.currentPattern.dissolving = false;
  
  // Random pattern parameters
  signalSystem.currentPattern.centerX = random(50, 270);     // Random center X
  signalSystem.currentPattern.centerY = random(50, 190);     // Random center Y  
  signalSystem.currentPattern.scale = random(20, 60);        // Random size
  signalSystem.currentPattern.rotation = random(360);       // Random rotation
  
  Serial.println("🔍 SIGNAL: Hidden pattern manifesting...");
  Serial.print("   Pattern type: ");
  Serial.print(signalSystem.currentPattern.patternType);
  Serial.print(", Duration: ");
  Serial.print(signalSystem.currentPattern.duration);
  Serial.println("ms");
}

void drawSignalPattern() {
  if (!signalSystem.currentPattern.active || signalSystem.currentPattern.intensity <= 0) {
    return;
  }
  
  float intensity = signalSystem.currentPattern.intensity;
  uint8_t brightness = signalSystem.signalBrightness * intensity;
  
  int cx = signalSystem.currentPattern.centerX;
  int cy = signalSystem.currentPattern.centerY;
  int size = signalSystem.currentPattern.scale * intensity;
  
  // Choose pattern set based on current signal mode
  if (currentMode == MATRIX_SIGNAL) {
    // Original signal patterns - geometric shapes
    switch (signalSystem.currentPattern.patternType) {
      case 0: // Cross pattern
        gfx->drawLine(cx, cy - size, cx, cy + size, 
                     gfx->color565(brightness, brightness, brightness));
        gfx->drawLine(cx - size, cy, cx + size, cy, 
                     gfx->color565(brightness, brightness, brightness));
        break;
      case 1: // Circle pattern
        gfx->drawCircle(cx, cy, size, 
                       gfx->color565(brightness, brightness, brightness));
        break;
      case 2: // Diamond pattern
        gfx->drawLine(cx, cy - size, cx + size, cy, 
                     gfx->color565(brightness, brightness, brightness));
        gfx->drawLine(cx + size, cy, cx, cy + size, 
                     gfx->color565(brightness, brightness, brightness));
        gfx->drawLine(cx, cy + size, cx - size, cy, 
                     gfx->color565(brightness, brightness, brightness));
        gfx->drawLine(cx - size, cy, cx, cy - size, 
                     gfx->color565(brightness, brightness, brightness));
        break;
      case 3: // Triangle pattern  
        gfx->drawLine(cx, cy - size, cx + size, cy + size/2, 
                     gfx->color565(brightness, brightness, brightness));
        gfx->drawLine(cx + size, cy + size/2, cx - size, cy + size/2, 
                     gfx->color565(brightness, brightness, brightness));
        gfx->drawLine(cx - size, cy + size/2, cx, cy - size, 
                     gfx->color565(brightness, brightness, brightness));
        break;
      case 4: // Grid pattern
        int spacing = size / 3;
        for (int i = -1; i <= 1; i++) {
          gfx->drawLine(cx + i * spacing, cy - size, cx + i * spacing, cy + size,
                       gfx->color565(brightness, brightness, brightness));
          gfx->drawLine(cx - size, cy + i * spacing, cx + size, cy + i * spacing,
                       gfx->color565(brightness, brightness, brightness));
        }
        break;
    }
  }
  else if (currentMode == MATRIX_SIGNAL_BINARY) {
    // Binary patterns - digital/computer shapes
    switch (signalSystem.currentPattern.patternType) {
      case 0: // Binary grid
        for (int i = 0; i < size; i += 4) {
          gfx->drawPixel(cx - size/2 + i, cy - size/2, 
                        gfx->color565(0, brightness, brightness));
          gfx->drawPixel(cx - size/2 + i, cy + size/2, 
                        gfx->color565(0, brightness, brightness));
        }
        for (int i = 0; i < size; i += 4) {
          gfx->drawPixel(cx - size/2, cy - size/2 + i, 
                        gfx->color565(0, brightness, brightness));
          gfx->drawPixel(cx + size/2, cy - size/2 + i, 
                        gfx->color565(0, brightness, brightness));
        }
        break;
      case 1: // Square brackets
        gfx->drawLine(cx - size, cy - size, cx - size, cy + size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx - size, cy - size, cx - size/2, cy - size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx - size, cy + size, cx - size/2, cy + size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx + size, cy - size, cx + size, cy + size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx + size, cy - size, cx + size/2, cy - size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx + size, cy + size, cx + size/2, cy + size,
                     gfx->color565(0, brightness, brightness));
        break;
      case 2: // Binary tree
        gfx->drawLine(cx, cy, cx - size/2, cy + size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx, cy, cx + size/2, cy + size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx, cy, cx, cy - size,
                     gfx->color565(0, brightness, brightness));
        break;
      case 3: // Circuit pattern
        gfx->drawRect(cx - size/2, cy - size/2, size, size,
                     gfx->color565(0, brightness, brightness));
        gfx->drawLine(cx - size/4, cy - size/4, cx + size/4, cy + size/4,
                     gfx->color565(0, brightness, brightness));
        break;
      case 4: // Data packet
        gfx->fillRect(cx - size/3, cy - size/4, size/6, size/2,
                     gfx->color565(0, brightness/2, brightness));
        gfx->fillRect(cx, cy - size/4, size/6, size/2,
                     gfx->color565(0, brightness/2, brightness));
        gfx->fillRect(cx + size/3, cy - size/4, size/6, size/2,
                     gfx->color565(0, brightness/2, brightness));
        break;
    }
  }
  else if (currentMode == MATRIX_SIGNAL_ORGANIC) {
    // Organic patterns - natural, flowing shapes
    switch (signalSystem.currentPattern.patternType) {
      case 0: // Spiral
        for (int angle = 0; angle < 360; angle += 20) {
          float rad = angle * PI / 180.0;
          float r = (angle / 360.0) * size;
          int x = cx + cos(rad) * r;
          int y = cy + sin(rad) * r;
          gfx->drawPixel(x, y, gfx->color565(brightness/2, brightness, brightness/3));
        }
        break;
      case 1: // Leaf pattern
        for (int i = 0; i < 8; i++) {
          float angle = i * PI / 4.0;
          int x1 = cx + cos(angle) * size/3;
          int y1 = cy + sin(angle) * size/3;
          int x2 = cx + cos(angle) * size;
          int y2 = cy + sin(angle) * size;
          gfx->drawLine(cx, cy, x2, y2, 
                       gfx->color565(brightness/2, brightness, brightness/3));
        }
        break;
      case 2: // Wave pattern
        for (int x = cx - size; x < cx + size; x += 2) {
          int y = cy + sin((x - cx) * 0.2) * size/3;
          gfx->drawPixel(x, y, gfx->color565(brightness/2, brightness, brightness/3));
        }
        break;
      case 3: // Flower
        for (int petal = 0; petal < 6; petal++) {
          float angle = petal * PI / 3.0;
          for (int r = 0; r < size; r += 3) {
            int x = cx + cos(angle) * r;
            int y = cy + sin(angle) * r;
            gfx->drawPixel(x, y, gfx->color565(brightness/2, brightness, brightness/3));
          }
        }
        break;
      case 4: // Tree branches
        gfx->drawLine(cx, cy + size, cx, cy, 
                     gfx->color565(brightness/2, brightness, brightness/3));
        gfx->drawLine(cx, cy, cx - size/2, cy - size/2,
                     gfx->color565(brightness/2, brightness, brightness/3));
        gfx->drawLine(cx, cy, cx + size/2, cy - size/2,
                     gfx->color565(brightness/2, brightness, brightness/3));
        break;
    }
  }
  else if (currentMode == MATRIX_SIGNAL_CRYPTIC) {
    // Cryptic patterns - ancient symbols and mysterious glyphs
    switch (signalSystem.currentPattern.patternType) {
      case 0: // Eye symbol
        gfx->drawCircle(cx, cy, size/2, gfx->color565(brightness, brightness/2, 0));
        gfx->fillCircle(cx, cy, size/4, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx - size, cy, cx - size/2, cy, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx + size/2, cy, cx + size, cy, gfx->color565(brightness, brightness/2, 0));
        break;
      case 1: // Runic cross
        gfx->drawLine(cx, cy - size, cx, cy + size, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx - size, cy, cx + size, cy, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx - size/3, cy - size/3, cx + size/3, cy + size/3, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx + size/3, cy - size/3, cx - size/3, cy + size/3, gfx->color565(brightness, brightness/2, 0));
        break;
      case 2: // Ankh
        gfx->drawCircle(cx, cy - size/3, size/4, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx, cy - size/6, cx, cy + size, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx - size/3, cy + size/3, cx + size/3, cy + size/3, gfx->color565(brightness, brightness/2, 0));
        break;
      case 3: // Pentagram
        for (int i = 0; i < 5; i++) {
          float angle1 = i * 2 * PI / 5.0 - PI/2;
          float angle2 = ((i + 2) % 5) * 2 * PI / 5.0 - PI/2;
          int x1 = cx + cos(angle1) * size;
          int y1 = cy + sin(angle1) * size;
          int x2 = cx + cos(angle2) * size;
          int y2 = cy + sin(angle2) * size;
          gfx->drawLine(x1, y1, x2, y2, gfx->color565(brightness, brightness/2, 0));
        }
        break;
      case 4: // Ancient key
        gfx->drawLine(cx - size, cy, cx + size/2, cy, gfx->color565(brightness, brightness/2, 0));
        gfx->drawCircle(cx - size/2, cy, size/4, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx + size/2, cy - size/3, cx + size/2, cy, gfx->color565(brightness, brightness/2, 0));
        gfx->drawLine(cx + size/3, cy - size/4, cx + size/2, cy - size/4, gfx->color565(brightness, brightness/2, 0));
        break;
    }
  }
  else if (currentMode == MATRIX_SIGNAL_GLITCH) {
    // Glitch patterns - corrupted, distorted shapes
    uint8_t noise = (millis() / 50) % 5;
    int glitchOffset = noise - 2;
    
    switch (signalSystem.currentPattern.patternType) {
      case 0: // Corrupted cross
        gfx->drawLine(cx + glitchOffset, cy - size, cx - glitchOffset, cy + size, 
                     gfx->color565(brightness, 0, brightness/2));
        gfx->drawLine(cx - size + glitchOffset, cy - glitchOffset, cx + size - glitchOffset, cy + glitchOffset, 
                     gfx->color565(brightness, brightness/3, 0));
        break;
      case 1: // Broken circle
        for (int angle = 0; angle < 360; angle += 10) {
          if ((angle + noise * 30) % 90 < 45) {  // Skip segments randomly
            float rad = angle * PI / 180.0;
            int x = cx + cos(rad) * size + glitchOffset;
            int y = cy + sin(rad) * size - glitchOffset;
            gfx->drawPixel(x, y, gfx->color565(brightness, 0, brightness));
          }
        }
        break;
      case 2: // Static rectangle
        for (int i = 0; i < size; i += 3) {
          for (int j = 0; j < size; j += 3) {
            if ((i + j + noise) % 7 < 3) {
              gfx->drawPixel(cx - size/2 + i + glitchOffset, cy - size/2 + j - glitchOffset, 
                           gfx->color565(0, brightness, brightness));
            }
          }
        }
        break;
      case 3: // Fragmenting triangle
        {
          int vertices[6] = {
            cx + glitchOffset, cy - size - glitchOffset,
            cx + size - glitchOffset, cy + size/2 + glitchOffset,
            cx - size + glitchOffset, cy + size/2 - glitchOffset
          };
          for (int i = 0; i < 3; i++) {
            int next = (i + 1) % 3;
            if ((i + noise) % 2 == 0) {  // Randomly skip lines
              gfx->drawLine(vertices[i*2], vertices[i*2+1], 
                           vertices[next*2], vertices[next*2+1],
                           gfx->color565(brightness, brightness/3, 0));
            }
          }
        }
        break;
      case 4: // Digital noise
        for (int i = 0; i < 20; i++) {
          int x = cx + random(-size, size);
          int y = cy + random(-size, size);
          uint8_t colorNoise = (noise + i) % 3;
          if (colorNoise == 0) {
            gfx->drawPixel(x, y, gfx->color565(brightness, 0, 0));
          } else if (colorNoise == 1) {
            gfx->drawPixel(x, y, gfx->color565(0, brightness, 0));
          } else {
            gfx->drawPixel(x, y, gfx->color565(0, 0, brightness));
          }
        }
        break;
    }
  }
}

// === FIREWORKS PRESSURE RELEASE SYSTEM ===

void initFireworkSystem() {
  Serial.println("🎆 FIREWORKS: Pressure release system initialized");
  
  // Clear all particles
  for (int i = 0; i < MAX_FIREWORK_PARTICLES; i++) {
    fireworkSystem.particles[i].active = false;
  }
  
  fireworkSystem.activeParticles = 0;
  fireworkSystem.burstActive = false;
  fireworkSystem.lastBurstTime = 0;
  fireworkSystem.burstCooldown = 2000; // 2 second minimum between bursts
  
  // Burst parameters
  fireworkSystem.particlesPerBurst = 12; // Moderate burst size
  fireworkSystem.initialVelocityRange = 4.0; // Initial speed range
  fireworkSystem.particleLifespan = 25; // 25 frames lifespan
  
  // Trigger conditions - rare and meaningful
  fireworkSystem.energyThreshold = 200.0; // High energy needed to burst
  fireworkSystem.randomBurstChance = 0.001; // 0.1% chance per frame
  fireworkSystem.cascadeTriggerEnabled = true;
  
  // Visual settings
  fireworkSystem.burstColor = gfx->color565(0, 255, 100); // Bright Matrix green
  fireworkSystem.gravityStrength = FIREWORK_GRAVITY;
  fireworkSystem.dragStrength = FIREWORK_DRAG;
}

void updateFireworkSystem(unsigned long currentTime) {
  // Get physics modifiers based on fireworks mode
  float gravityModifier = 1.0f;
  float dragModifier = 1.0f;
  
  if (currentMode == MATRIX_FIREWORKS_SLOWMO) {
    gravityModifier = 0.5f; // Slower gravity for dramatic effect
    dragModifier = 1.1f; // More drag
  } else if (currentMode == MATRIX_FIREWORKS_ICE) {
    gravityModifier = 1.2f; // Heavier ice shards
    dragModifier = 0.95f; // Less drag - ice cuts through air
  }
  
  // Update existing particles
  for (int i = 0; i < MAX_FIREWORK_PARTICLES; i++) {
    if (!fireworkSystem.particles[i].active) continue;
    
    FireworkParticle* p = &fireworkSystem.particles[i];
    
    // Apply physics: drag and gravity (with mode modifiers)
    p->vx *= (fireworkSystem.dragStrength * dragModifier);  // Air resistance
    p->vy += (fireworkSystem.gravityStrength * gravityModifier);  // Gravity
    
    // FIREWORKS COMBINATION EFFECTS: Add wind and other special physics
    if (currentMode >= MATRIX_FIREWORKSWIND && currentMode <= MATRIX_FIREWORKSENTROPY) {
      switch(currentMode) {
        case MATRIX_FIREWORKSWIND:
          // Wind effect: add horizontal drift based on global wind
          {
            float globalTime = millis() * 0.001f;
            float windForce = sin(globalTime * 0.3f) * 0.15f; // Gentle wind variation
            p->vx += windForce;
            // Add vertical wind gusts occasionally
            if (random(100) < 2) { // 2% chance of gust
              p->vy += random(-20, 10) / 100.0f; // Upward or downward gust
            }
          }
          break;
          
        case MATRIX_FIREWORKSRIPPLE:
          // Ripple effect: particles will create wave disturbances in columns
          // (Ripple effects will be applied to column positioning)
          break;
          
        case MATRIX_FIREWORKSFRACTURE:
          // Fracture effect: high-energy particles trigger column fractures
          if (p->energy > 150 && random(800) == 0) { // High energy particles rarely fracture
            int nearCol = (int)(p->x / (gfx->width() / MATRIX_COLS));
            if (nearCol >= 0 && nearCol < MATRIX_COLS && random(100) < 25) {
              // 25% chance to fracture nearby column
              columns[nearCol].speed = random(2, 8);
              columns[nearCol].length = random(8, 25);
            }
          }
          break;
          
        case MATRIX_FIREWORKSENTROPY:
          // Entropy effect: particles create chaos in matrix behavior
          if (p->energy > 100 && random(600) == 0) {
            // Particles occasionally randomize nearby columns
            int nearCol = (int)(p->x / (gfx->width() / MATRIX_COLS));
            if (nearCol >= 0 && nearCol < MATRIX_COLS) {
              columns[nearCol].y = random(MATRIX_ROWS);  // Entropy chaos!
            }
          }
          break;
      }
    }
    
    // Update position
    p->x += p->vx;
    p->y += p->vy;
    
    // Age particle
    p->life--;
    if (p->life <= 0) {
      p->active = false;
      fireworkSystem.activeParticles--;
      continue;
    }
    
    // Fade energy over time
    if (p->life < fireworkSystem.particleLifespan / 3) {
      p->energy = (p->energy * p->life) / (fireworkSystem.particleLifespan / 3);
    }
    
    // Remove particles that fall off screen
    if (p->y > gfx->height() + 10) {
      p->active = false;
      fireworkSystem.activeParticles--;
    }
  }
  
  // Check if we should trigger a burst (rare events only!)
  bool shouldBurst = false;
  float burstX = 0, burstY = 0;
  
  // Cooldown check
  if (currentTime - fireworkSystem.lastBurstTime < fireworkSystem.burstCooldown) {
    return; // Still cooling down
  }
  
  // Check columns for over-energized heads
  for (int col = 0; col < MATRIX_COLS; col++) {
    if (columns[col].y > 60 && columns[col].y < 180) { // Mid-screen area
      // Calculate column "energy" based on brightness and activity
      float energy = columns[col].speed * 30 + columns[col].length * 10;
      
      // Random energy spike
      if (random(100) == 0) energy += random(100, 300);
      
      if (energy > fireworkSystem.energyThreshold) {
        shouldBurst = true;
        burstX = col * CHAR_WIDTH + CHAR_WIDTH/2;
        burstY = columns[col].y;
        Serial.printf("🎆 Column %d over-energized (%.1f) - BURST!\n", col, energy);
        
        // Reset column to prevent immediate re-burst
        initColumn(col);
        break;
      }
    }
  }
  
  // Random burst chance (very low)
  if (!shouldBurst && random(100000) < (fireworkSystem.randomBurstChance * 100000)) {
    shouldBurst = true;
    burstX = random(50, gfx->width() - 50);
    burstY = random(60, 180);
    Serial.println("🎆 Random burst triggered!");
  }
  
  // Trigger burst if conditions met
  if (shouldBurst) {
    triggerFireworkBurst(burstX, burstY, fireworkSystem.burstColor);
    fireworkSystem.lastBurstTime = currentTime;
  }
}

void triggerFireworkBurst(float x, float y, uint16_t color) {
  Serial.printf("🎆 BOOM at (%.1f, %.1f)\n", x, y);
  
  // Get burst parameters based on firework type
  int particlesToSpawn = fireworkSystem.particlesPerBurst;
  float speedMultiplier = 1.0f;
  float angleRange = 2 * PI; // Full circle by default
  float angleOffset = 0;
  bool cascadeEffect = false;
  
  // Customize burst based on fireworks mode
  switch(currentMode) {
    case MATRIX_FIREWORKS_FAN:
      // Fan-shaped directional burst (120 degree arc)
      angleRange = PI * 0.67f; // 120 degrees
      angleOffset = -angleRange / 2.0f; // Center the fan
      break;
      
    case MATRIX_FIREWORKS_CASCADE:
      // Chain reaction - can trigger nearby columns
      cascadeEffect = true;
      speedMultiplier = 1.2f; // Slightly more energetic
      break;
      
    case MATRIX_FIREWORKS_SLOWMO:
      // Dramatic slow-motion burst
      speedMultiplier = 0.6f; // Slower particles
      particlesToSpawn = (int)(particlesToSpawn * 1.5f); // More particles
      break;
      
    case MATRIX_FIREWORKS_ICE:
      // Ice crystal shards - sharp angular bursts
      angleRange = 2 * PI; // Full circle but sharp angles
      speedMultiplier = 0.8f; // Brittle, less energetic
      break;
      
    default: // MATRIX_FIREWORKS - classic circular burst
      break;
  }
  
  // Find available particles
  int particlesSpawned = 0;
  for (int i = 0; i < MAX_FIREWORK_PARTICLES && particlesSpawned < particlesToSpawn; i++) {
    if (fireworkSystem.particles[i].active) continue;
    
    FireworkParticle* p = &fireworkSystem.particles[i];
    p->active = true;
    p->x = x;
    p->y = y;
    
    // Generate burst pattern based on mode
    float angle;
    float speed = random(200, 400) / 100.0 * speedMultiplier;
    
    if (currentMode == MATRIX_FIREWORKS_ICE) {
      // Sharp angular ice shards - use cardinal and diagonal directions
      float iceAngles[] = {0, PI/4, PI/2, 3*PI/4, PI, 5*PI/4, 3*PI/2, 7*PI/4};
      angle = iceAngles[particlesSpawned % 8];
    } else {
      // Normal burst pattern
      angle = angleOffset + (particlesSpawned * angleRange) / particlesToSpawn;
    }
    
    p->vx = cos(angle) * speed;
    p->vy = sin(angle) * speed - 2.0; // Initial upward kick
    
    // Mode-specific particle properties
    if (currentMode == MATRIX_FIREWORKS_SLOWMO) {
      p->life = fireworkSystem.particleLifespan * 2; // Longer life for slow-mo
    } else if (currentMode == MATRIX_FIREWORKS_ICE) {
      p->life = fireworkSystem.particleLifespan / 2; // Shorter life for brittle ice
    } else {
      p->life = fireworkSystem.particleLifespan + random(-5, 5); // Slight variation
    }
    
    p->energy = 200 + random(55); // Bright particles
    p->character = matrixChars[random(sizeof(matrixChars) - 1)];
    p->color = color;
    p->scale = 1.0;
    
    particlesSpawned++;
    fireworkSystem.activeParticles++;
  }
  
  // Cascade effect - trigger nearby column stress (simplified approach)
  if (cascadeEffect && currentMode == MATRIX_FIREWORKS_CASCADE) {
    int colIndex = (int)(x / (gfx->width() / MATRIX_COLS));
    for (int i = -2; i <= 2; i++) {
      int neighborCol = colIndex + i;
      if (neighborCol >= 0 && neighborCol < MATRIX_COLS && neighborCol != colIndex) {
        // Increase chance of nearby columns bursting by accelerating them temporarily
        if (random(100) < 30) { // 30% chance to affect neighbor
          columns[neighborCol].speed = min(columns[neighborCol].speed + 2, 15);
        }
      }
    }
  }
  
  fireworkSystem.burstActive = true;
  Serial.printf("🎆 Spawned %d particles\n", particlesSpawned);
  
  fireworkSystem.burstActive = true;
  Serial.printf("🎆 Spawned %d particles\n", particlesSpawned);
}

void drawFireworkParticles() {
  for (int i = 0; i < MAX_FIREWORK_PARTICLES; i++) {
    if (!fireworkSystem.particles[i].active) continue;
    
    FireworkParticle* p = &fireworkSystem.particles[i];
    
    // Skip off-screen particles
    if (p->x < 0 || p->x >= gfx->width() || p->y < 0 || p->y > gfx->height()) {
      continue;
    }
    
    // Calculate brightness based on energy and life
    uint8_t brightness = constrain(p->energy, 0, 255);
    brightness = (brightness * p->life) / fireworkSystem.particleLifespan;
    
    // Create color based on fireworks mode and brightness
    uint16_t drawColor;
    
    switch(currentMode) {
      case MATRIX_FIREWORKS_FAN:
        // Bright orange-yellow fan burst
        drawColor = gfx->color565(brightness, brightness * 0.7, brightness * 0.2);
        break;
        
      case MATRIX_FIREWORKS_CASCADE:
        // Red-orange cascade energy
        drawColor = gfx->color565(brightness, brightness * 0.5, 0);
        break;
        
      case MATRIX_FIREWORKS_SLOWMO:
        // Bright white slow-motion drama
        drawColor = gfx->color565(brightness, brightness, brightness * 0.9);
        break;
        
      case MATRIX_FIREWORKS_ICE:
        // Icy blue-white crystals
        drawColor = gfx->color565(brightness * 0.3, brightness * 0.8, brightness);
        break;
        
      case MATRIX_FIREWORKSWIND:
        // Wind-blown particles: shifting purple-pink colors
        drawColor = gfx->color565(brightness * 0.9, brightness * 0.4, brightness * 0.8);
        break;
        
      case MATRIX_FIREWORKSRIPPLE:
        // Ripple shockwave particles: electric yellow-gold
        drawColor = gfx->color565(brightness, brightness * 0.9, brightness * 0.2);
        break;
        
      case MATRIX_FIREWORKSFRACTURE:
        // Fracturing particles: chaotic multi-color flashes
        {
          uint8_t colorChoice = (millis() / 100 + i) % 4; // Cycle colors
          switch(colorChoice) {
            case 0: drawColor = gfx->color565(brightness, brightness * 0.3, 0); break; // Red
            case 1: drawColor = gfx->color565(0, brightness, brightness * 0.5); break; // Cyan
            case 2: drawColor = gfx->color565(brightness, brightness, 0); break; // Yellow
            default: drawColor = gfx->color565(brightness * 0.7, 0, brightness); break; // Magenta
          }
        }
        break;
        
      case MATRIX_FIREWORKSENTROPY:
        // Entropy chaos particles: dark red-black decay colors
        drawColor = gfx->color565(brightness * 0.8, brightness * 0.2, brightness * 0.1);
        break;
        
      default: // MATRIX_FIREWORKS - classic matrix green
        if (fireworkSystem.burstColor == gfx->color565(0, 255, 100)) { // Matrix green
          drawColor = gfx->color565(0, brightness, brightness/3);
        } else {
          drawColor = gfx->color565(brightness, brightness, brightness);
        }
        break;
    }
    
    // Clear old position (approximate)
    gfx->fillRect((int)p->x - 1, (int)p->y - 1, CHAR_WIDTH + 2, CHAR_HEIGHT + 2, BLACK);
    
    // Draw particle
    gfx->setTextColor(drawColor);
    gfx->setTextSize(1);
    gfx->setCursor((int)p->x, (int)p->y);
    gfx->print(p->character);
    
    // Optional: Add spark effect for very bright particles
    if (brightness > 180) {
      gfx->drawPixel((int)p->x + random(-2, 2), (int)p->y + random(-2, 2), drawColor);
    }
  }
}

// === HISTORY WEIGHT SPACETIME SYSTEM IMPLEMENTATION ===
void initHistorySystem() {
  historyInitialized = true;
  historySystem.lastUpdate = millis();
  historySystem.maxHeat = 0.0;

  // Set different parameters based on the specific history mode
  switch(currentMode) {
    case MATRIX_HISTORY_WEIGHT:
      // Original spacetime - balanced physics
      historySystem.decayRate = 0.985f;
      historySystem.influenceRadius = 3.0f;
      historySystem.forceStrength = 2.0f;
      Serial.println("🌌 HISTORY WEIGHT: Spacetime memory system initialized");
      break;
      
    case MATRIX_HISTORY_FIRE:
      // Fire spacetime - faster decay, stronger repulsion
      historySystem.decayRate = 0.975f;      // Faster decay (fire burns out)
      historySystem.influenceRadius = 4.0f;  // Larger influence (heat spreads)
      historySystem.forceStrength = 3.0f;    // Stronger forces (explosive heat)
      Serial.println("🔥 HISTORY FIRE: Thermal spacetime initialized - hot paths repel!");
      break;
      
    case MATRIX_HISTORY_ICE:
      // Ice spacetime - slower decay, brittle crystalline effects
      historySystem.decayRate = 0.995f;      // Very slow decay (ice persists)
      historySystem.influenceRadius = 2.5f;  // Smaller influence (brittle, localized)
      historySystem.forceStrength = 1.5f;    // Weaker forces (brittle structure)
      Serial.println("❄️ HISTORY ICE: Frozen spacetime initialized - crystalline memory!");
      break;
      
    case MATRIX_HISTORY_STORM:
      // Storm spacetime - chaotic decay, electric field effects
      historySystem.decayRate = 0.980f;      // Medium-fast decay (energy dissipates)
      historySystem.influenceRadius = 5.0f;  // Large influence (electric fields)
      historySystem.forceStrength = 2.5f;    // Strong but chaotic forces
      Serial.println("⚡ HISTORY STORM: Electric spacetime initialized - lightning memory!");
      break;
      
    case MATRIX_HISTORY_DEEP:
      // Deep space - very slow massive effects
      historySystem.decayRate = 0.998f;      // Extremely slow decay (deep time)
      historySystem.influenceRadius = 6.0f;  // Massive influence (gravity wells)
      historySystem.forceStrength = 1.0f;    // Weaker but persistent forces (deep gravity)
      Serial.println("🌌 HISTORY DEEP: Deep space initialized - cosmic gravitational memory!");
      break;
  }

  // Clear the spacetime heatmap
  for (int y = 0; y < HEATMAP_HEIGHT; y++) {
    for (int x = 0; x < HEATMAP_WIDTH; x++) {
      historySystem.heatmap[y][x] = 0.0;
    }
  }

  // Initialize all columns
  for (int i = 0; i < MATRIX_COLS; i++) {
    initHistoryColumn(i);
  }
}

void initHistoryColumn(int col) {
  historyColumns[col].deflectionX = 0.0;
  historyColumns[col].deflectionVelocity = 0.0;
  historyColumns[col].lastForceX = 0.0;
  
  // Set different mass properties based on mode
  switch(currentMode) {
    case MATRIX_HISTORY_WEIGHT:
      historyColumns[col].mass = 1.0;  // Standard mass
      break;
    case MATRIX_HISTORY_FIRE:
      historyColumns[col].mass = 1.5;  // Heavier mass (more heat contribution)
      break;
    case MATRIX_HISTORY_ICE:
      historyColumns[col].mass = 0.8;  // Lighter mass (brittle structure)
      break;
    case MATRIX_HISTORY_STORM:
      historyColumns[col].mass = 1.2;  // Medium-heavy mass (electric charge)
      break;
    case MATRIX_HISTORY_DEEP:
      historyColumns[col].mass = 2.0;  // Heavy mass (gravitational influence)
      break;
  }
}

void updateHistorySystem(unsigned long currentTime) {
  if (!historyInitialized) return;
  
  float deltaTime = (currentTime - historySystem.lastUpdate) / 1000.0;
  historySystem.lastUpdate = currentTime;
  
  // Update maximum heat tracking
  historySystem.maxHeat = 0.0;
  
  // Decay all heatmap values (spacetime memory fades)
  for (int y = 0; y < HEATMAP_HEIGHT; y++) {
    for (int x = 0; x < HEATMAP_WIDTH; x++) {
      historySystem.heatmap[y][x] *= historySystem.decayRate;
      if (historySystem.heatmap[y][x] > historySystem.maxHeat) {
        historySystem.maxHeat = historySystem.heatmap[y][x];
      }
    }
  }
  
  // Update each column's physics
  for (int i = 0; i < MATRIX_COLS; i++) {
    if (columns[i].y > 0) { // Only if column is active
      updateHistoryColumn(i, currentTime);
    }
  }
}

void updateHistoryColumn(int col, unsigned long currentTime) {
  if (!historyInitialized) return;
  
  // Calculate current screen position
  float colX = col * (gfx->width() / MATRIX_COLS) + (gfx->width() / MATRIX_COLS / 2);
  float colY = columns[col].y * CHAR_HEIGHT;
  
  // Record this column's path in spacetime heatmap
  recordColumnPath(col, colX, colY);
  
  // Apply forces from spacetime history
  applyHistoryForces(col, currentTime);
}

void recordColumnPath(int col, float x, float y) {
  // Convert screen coordinates to heatmap coordinates
  int heatX = constrain((int)(x * HEATMAP_WIDTH / gfx->width()), 0, HEATMAP_WIDTH - 1);
  int heatY = constrain((int)(y * HEATMAP_HEIGHT / gfx->height()), 0, HEATMAP_HEIGHT - 1);
  
  // Add heat based on column mass and activity
  float heatContribution = historyColumns[col].mass * 0.1f;
  
  // Add heat to a small area around the column (not just single pixel)
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      int hx = heatX + dx;
      int hy = heatY + dy;
      
      if (hx >= 0 && hx < HEATMAP_WIDTH && hy >= 0 && hy < HEATMAP_HEIGHT) {
        float distance = sqrt(dx*dx + dy*dy);
        float falloff = 1.0f / (1.0f + distance);
        historySystem.heatmap[hy][hx] += heatContribution * falloff;
        
        // Cap maximum heat
        if (historySystem.heatmap[hy][hx] > 10.0f) {
          historySystem.heatmap[hy][hx] = 10.0f;
        }
      }
    }
  }
}

void applyHistoryForces(int col, unsigned long currentTime) {
  // Calculate current screen position
  float colX = col * (gfx->width() / MATRIX_COLS) + (gfx->width() / MATRIX_COLS / 2);
  float colY = columns[col].y * CHAR_HEIGHT;
  
  // Calculate forces from nearby heatmap regions
  float totalForceX = 0.0;
  int samples = 0;
  
  // Sample the heatmap in a radius around current position
  int centerHeatX = (int)(colX * HEATMAP_WIDTH / gfx->width());
  int centerHeatY = (int)(colY * HEATMAP_HEIGHT / gfx->height());
  
  int checkRadius = (int)historySystem.influenceRadius;
  
  for (int dy = -checkRadius; dy <= checkRadius; dy++) {
    for (int dx = -checkRadius; dx <= checkRadius; dx++) {
      int heatX = centerHeatX + dx;
      int heatY = centerHeatY + dy;
      
      if (heatX >= 0 && heatX < HEATMAP_WIDTH && heatY >= 0 && heatY < HEATMAP_HEIGHT) {
        float distance = sqrt(dx*dx + dy*dy);
        if (distance <= historySystem.influenceRadius && distance > 0) {
          float heat = historySystem.heatmap[heatY][heatX];
          
          if (heat > 0.1f) { // Only consider significant heat
            // Force calculation based on heat intensity
            float forceDirection = (heat < 3.0f) ? -1.0f : 1.0f; // Low heat attracts, high heat repels
            float forceMagnitude = historySystem.forceStrength * heat / (distance * distance + 1.0f);
            
            // X-direction force (deflection)
            float forceX = forceDirection * forceMagnitude * dx / distance;
            totalForceX += forceX;
            samples++;
          }
        }
      }
    }
  }
  
  // Apply averaged force to column deflection
  if (samples > 0) {
    totalForceX /= samples;
    
    // Apply force with momentum physics
    historyColumns[col].deflectionVelocity += totalForceX * 0.1f;
    historyColumns[col].deflectionVelocity *= 0.95f; // Damping
    historyColumns[col].deflectionX += historyColumns[col].deflectionVelocity;
    
    // Limit deflection range
    historyColumns[col].deflectionX = constrain(historyColumns[col].deflectionX, -60.0f, 60.0f);
    
    historyColumns[col].lastForceX = totalForceX;
  }
}

uint16_t getHistoryColor(int intensity, bool isHead) {
  // Base color is soft blue-white (spacetime/gravity theme)
  if (isHead) {
    return gfx->color565(intensity * 0.7, intensity * 0.9, intensity);
  } else {
    return gfx->color565(intensity * 0.4, intensity * 0.6, intensity * 0.8);
  }
}