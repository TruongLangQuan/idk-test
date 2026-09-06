#include <M5StickCPlus2.h>
#include <math.h>
#include <Preferences.h>

// Display setup from Halloween Tetris
#define Disp M5.Lcd

// Biorhythm cycles (in days)
#define PHYSICAL_CYCLE 23
#define EMOTIONAL_CYCLE 28
#define INTELLECTUAL_CYCLE 33

// App states
enum AppState {
  WELCOME_SCREEN,
  BIRTH_DATE_INPUT,
  CURRENT_DATE_INPUT,
  BIORHYTHM_DISPLAY,
  GRAPH_VIEW,
  INFO_SCREEN
};

// Date structure
struct Date {
  int year;
  int month;
  int day;
};

// Date field enum for cleaner field switching
enum DateField { FIELD_MONTH, FIELD_DAY, FIELD_YEAR };

// Global variables
AppState currentState = WELCOME_SCREEN;
Date birthDate = {1970, 1, 1};  // Default birth date
Date currentDate = {2024, 12, 10};  // Default current date - user must set this
bool birthDateSet = false;
bool currentDateSet = false;
int currentGraph = 0;  // 0=Physical, 1=Emotional, 2=Intellectual
int infoPage = 0;      // 0=Physical, 1=Emotional, 2=Intellectual info
DateField currentBirthField = FIELD_MONTH;  // For birth date input
DateField currentTodayField = FIELD_MONTH;  // For today date input
Preferences preferences;

// Remove the hardcoded date function - we'll let user set it manually

// Function declarations
void drawWelcomeScreen();
void drawBirthDateInput();
void drawCurrentDateInput();
void drawBiorhythmScreen();
void drawSingleGraphScreen();
void drawInfoScreen();
void drawLargeBiorhythmCurve(int x, int y, int width, int height, int centerDay, int cycle, uint16_t color);
void drawMiniBiorhythmGraph(int x, int y, int width, int height);
void drawBiorhythmCurve(int x, int y, int width, int height, int centerDay, int cycle, uint16_t color);
void handleWelcomeScreen();
void handleBirthDateInput();
void handleCurrentDateInput();
void handleBiorhythmDisplay();
void handleGraphView();
void handleInfoScreen();
void saveBirthDate();
void loadBirthDate();
void saveCurrentDate();
void loadCurrentDate();
int daysBetween(Date birth, Date current);
float calculateBiorhythm(int daysAlive, int cycleDays);
int biorhythmToPercent(float value);
uint16_t getBiorhythmColor(float value);

void setup() {
  auto cfg = M5.config();
  
  // Disable power management that might interfere with buttons
  cfg.clear_display = true;
  cfg.output_power = false;
  cfg.internal_imu = false;  
  cfg.internal_rtc = false;
  cfg.internal_spk = false;
  cfg.internal_mic = false;
  
  M5.begin(cfg);
  
  // Initialize display
  Disp.setRotation(0);  // Portrait mode
  Disp.fillScreen(BLACK);
  
  // Initialize preferences for storing dates
  preferences.begin("biorhythm", false);
  
  // Load saved dates if they exist
  loadBirthDate();
  loadCurrentDate();
  
  // Display welcome screen
  drawWelcomeScreen();
}

void loop() {
  M5.update();
  
  // Handle different app states
  switch(currentState) {
    case WELCOME_SCREEN:
      handleWelcomeScreen();
      break;
    case BIRTH_DATE_INPUT:
      handleBirthDateInput();
      break;
    case CURRENT_DATE_INPUT:
      handleCurrentDateInput();
      break;
    case BIORHYTHM_DISPLAY:
      handleBiorhythmDisplay();
      break;
    case GRAPH_VIEW:
      handleGraphView();
      break;
    case INFO_SCREEN:
      handleInfoScreen();
      break;
  }
  
  delay(50); // Shorter delay for better responsiveness
}

// ========== BIORHYTHM CALCULATION FUNCTIONS ==========

// Calculate days between two dates (simplified)
int daysBetween(Date birth, Date current) {
  // Simple approximation - good enough for biorhythms
  // More accurate calculation could account for leap years
  int birthDays = birth.year * 365 + birth.month * 30 + birth.day;
  int currentDays = current.year * 365 + current.month * 30 + current.day;
  return currentDays - birthDays;
}

// Calculate biorhythm value (-1.0 to 1.0)
float calculateBiorhythm(int daysAlive, int cycleDays) {
  return sin(2.0 * PI * daysAlive / cycleDays);
}

// Convert biorhythm value to percentage (-100% to +100%)
int biorhythmToPercent(float value) {
  return (int)(value * 100);
}

// Get biorhythm color based on value
uint16_t getBiorhythmColor(float value) {
  if (value > 0.5) return 0x07E0;      // Green (high positive)
  else if (value > 0) return 0xFFE0;   // Yellow (low positive)  
  else if (value > -0.5) return 0xFC00; // Orange (low negative)
  else return 0xF800;                  // Red (high negative)
}

void handleGraphView() {
  if (M5.BtnA.wasClicked()) {
    // Cycle through graphs: Physical -> Emotional -> Intellectual -> Back to main
    currentGraph++;
    if (currentGraph > 2) {
      currentGraph = 0;
      currentState = BIORHYTHM_DISPLAY;
      drawBiorhythmScreen();
    } else {
      drawSingleGraphScreen();
    }
  }
  
  // B button for date navigation in graph mode
  if (M5.BtnB.wasClicked()) {
    currentDate.day++;
    if (currentDate.day > 31) {
      currentDate.day = 1;
      currentDate.month++;
      if (currentDate.month > 12) {
        currentDate.month = 1;
        currentDate.year++;
      }
    }
    drawSingleGraphScreen();
  }
}

// ========== DATA PERSISTENCE FUNCTIONS ==========

void saveBirthDate() {
  preferences.putInt("birthYear", birthDate.year);
  preferences.putInt("birthMonth", birthDate.month);
  preferences.putInt("birthDay", birthDate.day);
  preferences.putBool("birthDateSet", true);
}

void loadBirthDate() {
  if (preferences.getBool("birthDateSet", false)) {
    birthDate.year = preferences.getInt("birthYear", 1970);
    birthDate.month = preferences.getInt("birthMonth", 1);
    birthDate.day = preferences.getInt("birthDay", 1);
    birthDateSet = true;
  }
}

void saveCurrentDate() {
  preferences.putInt("currentYear", currentDate.year);
  preferences.putInt("currentMonth", currentDate.month);
  preferences.putInt("currentDay", currentDate.day);
  preferences.putBool("currentDateSet", true);
}

void loadCurrentDate() {
  if (preferences.getBool("currentDateSet", false)) {
    currentDate.year = preferences.getInt("currentYear", 2024);
    currentDate.month = preferences.getInt("currentMonth", 12);
    currentDate.day = preferences.getInt("currentDay", 10);
    currentDateSet = true;
  }
}

// ========== UI STATE HANDLERS ==========

void handleWelcomeScreen() {
  // Check for info window activation (hold both buttons)
  if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
    delay(1000); // Wait for hold
    if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
      currentState = INFO_SCREEN;
      infoPage = 0;
      drawInfoScreen();
      return;
    }
  }
  
  // Use wasClicked instead of wasPressed - this might be the issue!
  if (M5.BtnA.wasClicked()) {
    if (birthDateSet && currentDateSet) {
      currentState = BIORHYTHM_DISPLAY;
      drawBiorhythmScreen();
    } else if (!birthDateSet) {
      currentState = BIRTH_DATE_INPUT;
      drawBirthDateInput();
    } else if (!currentDateSet) {
      currentState = CURRENT_DATE_INPUT;
      drawCurrentDateInput();
    }
  }
  
  // Reset dates option
  if (M5.BtnB.wasClicked() && (birthDateSet || currentDateSet)) {
    birthDateSet = false;
    currentDateSet = false;
    preferences.putBool("birthDateSet", false);
    preferences.putBool("currentDateSet", false);
    drawWelcomeScreen();
  }
}

void handleBirthDateInput() {
  // M5 button (main button) - increment current field (KEEP THIS AS IS)
  if (M5.BtnA.wasClicked()) {
    if (currentBirthField == FIELD_MONTH) {
      birthDate.month++;
      if (birthDate.month > 12) birthDate.month = 1;
    }
    else if (currentBirthField == FIELD_DAY) {
      birthDate.day++;
      if (birthDate.day > 31) birthDate.day = 1;
    }
    else if (currentBirthField == FIELD_YEAR) {
      birthDate.year++;
      if (birthDate.year > 2035) birthDate.year = 1970;
    }
    drawBirthDateInput();
  }
  
  // LEFT button (PWR) - change fields using your clean approach
  if (M5.BtnPWR.wasClicked()) {
    currentBirthField = static_cast<DateField>((currentBirthField + 1) % 3);
    drawBirthDateInput(); // Refresh display to show new selection
  }
  
  // RIGHT button (B button) - save when on any field, single click
  if (M5.BtnB.wasClicked()) {
    // Save the birth date
    saveBirthDate();
    birthDateSet = true;
    // Now go to current date input if not set
    if (!currentDateSet) {
      currentState = CURRENT_DATE_INPUT;
      drawCurrentDateInput();
    } else {
      currentState = BIORHYTHM_DISPLAY;
      drawBiorhythmScreen();
    }
  }
}

void handleCurrentDateInput() {
  // M5 button - increment current field (KEEP THIS AS IS)
  if (M5.BtnA.wasClicked()) {
    if (currentTodayField == FIELD_MONTH) {
      currentDate.month++;
      if (currentDate.month > 12) currentDate.month = 1;
    }
    else if (currentTodayField == FIELD_DAY) {
      currentDate.day++;
      if (currentDate.day > 31) currentDate.day = 1;
    }
    else if (currentTodayField == FIELD_YEAR) {
      currentDate.year++;
      if (currentDate.year > 2035) currentDate.year = 2020;
    }
    drawCurrentDateInput();
  }
  
  // LEFT button (PWR) - change fields  
  if (M5.BtnPWR.wasClicked()) {
    currentTodayField = static_cast<DateField>((currentTodayField + 1) % 3);
    drawCurrentDateInput();
  }
  
  // RIGHT button (B button) - save, single click
  if (M5.BtnB.wasClicked()) {
    // Save the current date
    saveCurrentDate();
    currentDateSet = true;
    currentState = BIORHYTHM_DISPLAY;
    drawBiorhythmScreen();
  }
}

void handleBiorhythmDisplay() {
  if (M5.BtnA.wasClicked()) {
    currentState = GRAPH_VIEW;
    currentGraph = 0; // Start with Physical
    drawSingleGraphScreen();
  }
  
  if (M5.BtnB.wasClicked()) {
    // Navigate to next day
    currentDate.day++;
    if (currentDate.day > 31) {
      currentDate.day = 1;
      currentDate.month++;
      if (currentDate.month > 12) {
        currentDate.month = 1;
        currentDate.year++;
      }
    }
    drawBiorhythmScreen();
  }
}

void handleInfoScreen() {
  if (M5.BtnA.wasClicked()) {
    infoPage = (infoPage + 1) % 3; // Cycle through Physical, Emotional, Intellectual
    drawInfoScreen();
  }
  
  if (M5.BtnB.wasClicked()) {
    currentState = WELCOME_SCREEN;
    drawWelcomeScreen();
  }
}

// ========== DISPLAY FUNCTIONS FROM HALLOWEEN TETRIS ==========

void drawWelcomeScreen() {
  Disp.fillScreen(BLACK);
  
  // Title
  Disp.setTextColor(0x07E0); // Green color
  Disp.setTextSize(2);
  Disp.drawString("BIORHYTHM", 15, 30, 1);
  Disp.drawString("CALCULATOR", 5, 55, 1);
  
  // Status
  Disp.setTextColor(WHITE);
  Disp.setTextSize(1);
  
  if (birthDateSet && currentDateSet) {
    Disp.drawString("Ready to calculate!", 10, 85, 1);
    // Remove the Birth/Today display lines that show "Failed" - cleaner look
    Disp.drawString("M5: View rhythms", 15, 110, 1);
    Disp.drawString("B: Reset dates", 15, 130, 1);
    Disp.drawString("PWR+M5: Info", 15, 150, 1);
  } else if (!birthDateSet) {
    Disp.drawString("Set your birth date", 15, 100, 1);
    Disp.drawString("M5: Set birth date", 15, 120, 1);
  } else if (!currentDateSet) {
    Disp.drawString("Set today's date", 20, 100, 1);
    Disp.drawString("M5: Set today's date", 10, 120, 1);
  }
  
  // Info and battery
  Disp.setTextColor(0x8410);
  Disp.drawString("Hold M5+B: Info", 20, 185, 1);
  Disp.drawString("Battery:", 10, 8, 1);
  Disp.drawString((String)M5.Power.getBatteryLevel()+"%", 70, 8, 1);
}

void drawCurrentDateInput() {
  Disp.fillScreen(BLACK);
  
  // Get current edit field for highlighting
  
  // Title
  Disp.setTextColor(0x07FF); // Cyan
  Disp.setTextSize(2);
  Disp.drawString("TODAY'S DATE", 5, 30, 1);
  
  // Instructions
  Disp.setTextColor(WHITE);
  Disp.setTextSize(1);
  Disp.drawString("Set the current date", 10, 60, 1);
  
  // Date input with field highlighting (USA format: MM/DD on line 1, YYYY on line 2)
  Disp.setTextSize(1);
  
  // Month (field 0)
  if (currentTodayField == FIELD_MONTH) Disp.setTextColor(0x07E0); // Green when selected
  else Disp.setTextColor(WHITE);
  String monthStr = (currentDate.month < 10 ? "0" : "") + String(currentDate.month);
  Disp.drawString(monthStr, 20, 90, 1);
  
  // Separator
  Disp.setTextColor(WHITE);
  Disp.drawString("/", 40, 90, 1);
  
  // Day (field 1)  
  if (currentTodayField == FIELD_DAY) Disp.setTextColor(0x07E0);
  else Disp.setTextColor(WHITE);
  String dayStr = (currentDate.day < 10 ? "0" : "") + String(currentDate.day);
  Disp.drawString(dayStr, 50, 90, 1);
  
  // Year on new line (field 2) - Full 4 digits with plenty of room
  if (currentTodayField == FIELD_YEAR) Disp.setTextColor(0x07E0);
  else Disp.setTextColor(WHITE);
  Disp.drawString(String(currentDate.year), 35, 110, 1);
  
  // Instructions
  Disp.setTextColor(0x07E0);
  Disp.setTextSize(1);
  Disp.drawString("M5: Increment", 25, 140, 1);
  Disp.drawString("PWR: Change field", 20, 160, 1);
  Disp.drawString("B: Save & Continue", 20, 180, 1);
}

void drawInfoScreen() {
  Disp.fillScreen(BLACK);
  
  String title, description1, description2, description3;
  uint16_t color;
  
  switch(infoPage) {
    case 0: // Physical
      title = "PHYSICAL CYCLE";
      color = 0xF800; // Red
      description1 = "23-day cycle";
      description2 = "Body strength, stamina";
      description3 = "and coordination";
      break;
    case 1: // Emotional
      title = "EMOTIONAL CYCLE";
      color = 0xFFE0; // Yellow
      description1 = "28-day cycle";
      description2 = "Mood, feelings";
      description3 = "and creativity";
      break;
    case 2: // Intellectual
      title = "INTELLECTUAL";
      color = 0x07FF; // Cyan
      description1 = "33-day cycle";
      description2 = "Logic, memory";
      description3 = "and concentration";
      break;
  }

  Disp.setTextColor(color);
  Disp.setTextSize(1);
  Disp.drawString(title, 10, 35, 1);
  Disp.drawString(description1, 15, 80, 1);
  
  Disp.setTextColor(WHITE);
  Disp.setTextSize(1);
  Disp.drawString(description2, 15, 110, 1);
  Disp.drawString(description3, 15, 130, 1);
  
  // Brief explanation
  Disp.setTextColor(0x8410); // Gray
  Disp.drawString("Higher values = better", 10, 165, 1);
  Disp.drawString("Lower values = rest time", 10, 185, 1);
  
  // Controls
  Disp.setTextColor(WHITE);
  Disp.drawString("M5: Next  B: Back", 15, 220, 1);
}

void drawBirthDateInput() {
  Disp.fillScreen(BLACK);
  
  // Title
  Disp.setTextColor(0xFFE0); // Yellow
  Disp.setTextSize(2);
  Disp.drawString("BIRTH DATE", 10, 30, 1);
  
  // Current date for reference
  Disp.setTextColor(WHITE);
  Disp.setTextSize(1);
  Disp.drawString("Today:", 10, 60, 1);
  String todayStr = String(currentDate.month) + "/" + String(currentDate.day) + "/" + String(currentDate.year);
  Disp.drawString(todayStr, 50, 60, 1);
  
  // Date input with field highlighting (USA format: MM/DD on line 1, YYYY on line 2)
  Disp.setTextSize(1);
  
  // Month (FIELD_MONTH)
  if (currentBirthField == FIELD_MONTH) Disp.setTextColor(0x07E0); // Green when selected
  else Disp.setTextColor(WHITE);
  String monthStr = (birthDate.month < 10 ? "0" : "") + String(birthDate.month);
  Disp.drawString(monthStr, 20, 90, 1);
  
  // Separator
  Disp.setTextColor(WHITE);
  Disp.drawString("/", 40, 90, 1);
  
  // Day (FIELD_DAY)
  if (currentBirthField == FIELD_DAY) Disp.setTextColor(0x07E0);
  else Disp.setTextColor(WHITE);
  String dayStr = (birthDate.day < 10 ? "0" : "") + String(birthDate.day);
  Disp.drawString(dayStr, 50, 90, 1);
  
  // Year on new line (FIELD_YEAR) - Full 4 digits with plenty of room
  if (currentBirthField == FIELD_YEAR) Disp.setTextColor(0x07E0);
  else Disp.setTextColor(WHITE);
  Disp.drawString(String(birthDate.year), 35, 110, 1);
  
  // Instructions
  Disp.setTextColor(0x07E0);
  Disp.setTextSize(1);
  Disp.drawString("M5: Increment", 25, 140, 1);
  Disp.drawString("PWR: Change field", 20, 160, 1);
  Disp.drawString("B: Save & Continue", 20, 180, 1);
}


void drawBiorhythmScreen() {
  Disp.fillScreen(BLACK);
  
  int daysAlive = daysBetween(birthDate, currentDate);
  
  // Title
  Disp.setTextColor(0x07E0);
  Disp.setTextSize(1);
  Disp.drawString("BIORHYTHM", 30, 10, 1);
  
  // Current date
  Disp.setTextColor(WHITE);
  String dateStr = String(String(currentDate.month) + "/" + String(currentDate.day) + "/" + String(currentDate.year));
  Disp.drawString(dateStr, 25, 25, 1);
  
  // Days alive
  Disp.drawString("Days alive: " + String(daysAlive), 5, 40, 1);
  
  // Calculate and display biorhythms
  // NOTE: Percentages show TODAY's biorhythm level for each cycle
  // +100% = Peak positive, 0% = Neutral, -100% = Peak negative
  float physical = calculateBiorhythm(daysAlive, PHYSICAL_CYCLE);
  float emotional = calculateBiorhythm(daysAlive, EMOTIONAL_CYCLE);
  float intellectual = calculateBiorhythm(daysAlive, INTELLECTUAL_CYCLE);
  
  // Physical
  Disp.setTextColor(getBiorhythmColor(physical));
  Disp.drawString("Physical:", 5, 65, 1);
  Disp.drawString(String(biorhythmToPercent(physical)) + "%", 70, 65, 1);
  
  // Emotional  
  Disp.setTextColor(getBiorhythmColor(emotional));
  Disp.drawString("Emotional:", 5, 85, 1);
  Disp.drawString(String(biorhythmToPercent(emotional)) + "%", 70, 85, 1);
  
  // Intellectual
  Disp.setTextColor(getBiorhythmColor(intellectual));
  Disp.drawString("Intellect:", 5, 105, 1);
  Disp.drawString(String(biorhythmToPercent(intellectual)) + "%", 70, 105, 1);
  
  // Mini graphs
  drawMiniBiorhythmGraph(5, 130, 125, 30);
  
  // Controls
  Disp.setTextColor(WHITE);
  Disp.setTextSize(1);
  Disp.drawString("M5:Graph B:Next day", 5, 200, 1);
}

// Replace the old drawGraphScreen with individual graph screens
void drawSingleGraphScreen() {
  Disp.fillScreen(BLACK);
  
  int daysAlive = daysBetween(birthDate, currentDate);
  
  // Graph configuration
  int graphWidth = 125;
  int graphHeight = 160;  // Much taller for portrait mode
  int startX = 5;
  int startY = 40;
  
  // Determine which graph to show
  String cycleName;
  int cycleDays;
  uint16_t cycleColor;
  
  switch(currentGraph) {
    case 0: // Physical
      cycleName = "PHYSICAL";
      cycleDays = PHYSICAL_CYCLE;
      cycleColor = 0xF800; // Red
      break;
    case 1: // Emotional  
      cycleName = "EMOTIONAL";
      cycleDays = EMOTIONAL_CYCLE;
      cycleColor = 0xFFE0; // Yellow
      break;
    case 2: // Intellectual
      cycleName = "INTELLECT";
      cycleDays = INTELLECTUAL_CYCLE;
      cycleColor = 0x07FF; // Cyan
      break;
  }
  
  // Title
  Disp.setTextColor(cycleColor);
  Disp.setTextSize(1);
  Disp.drawString(cycleName, 15, 10, 1);
  Disp.setTextColor(WHITE);
  Disp.drawString("7-DAY CYCLE", 25, 25, 1);  // Changed from 30-day to 7-day
  
  // Current value
  float currentValue = calculateBiorhythm(daysAlive, cycleDays);
  int percent = biorhythmToPercent(currentValue);
  String valueStr = String(percent) + "%";
  Disp.setTextColor(getBiorhythmColor(currentValue));
  Disp.setTextSize(2);
  Disp.drawString(valueStr, 40, 210, 1);
  
  // Draw large portrait graph
  drawLargeBiorhythmCurve(startX, startY, graphWidth, graphHeight, daysAlive, cycleDays, cycleColor);
  
  // Controls
  Disp.setTextColor(WHITE);
  Disp.setTextSize(1);
  String nextGraph = "";
  if (currentGraph == 0) nextGraph = "Emotional";
  else if (currentGraph == 1) nextGraph = "Intellect";
  else nextGraph = "Main Menu";
  
  Disp.drawString("A: " + nextGraph, 5, 230, 1);
  
  // Show current date
  String dateStr = String(String(currentDate.month) + "/" + String(currentDate.day) + "/" + String(currentDate.year));
  Disp.drawString(dateStr, 70, 230, 1);
}

void drawLargeBiorhythmCurve(int x, int y, int width, int height, int centerDay, int cycle, uint16_t color) {
  // Draw background box
  Disp.drawRect(x, y, width, height, WHITE);
  
  // Draw center line (0%)
  int centerY = y + height/2;
  Disp.drawLine(x, centerY, x + width, centerY, 0x8410);
  
  // Draw quarter lines for reference
  int quarterY1 = y + height/4;
  int quarterY2 = y + 3*height/4;
  for(int i = x; i < x + width; i += 5) {
    Disp.drawPixel(i, quarterY1, 0x4208);
    Disp.drawPixel(i, quarterY2, 0x4208);
  }
  
  // Draw connected line curve for 7 days (changed from dots to lines)
  int prevX = -1, prevY = -1;
  int daysToShow = 7;  // Changed from 30 to 7 days
  
  for (int i = 0; i < width; i += width/daysToShow) {  // Sample every day over 7 days
    int day = centerDay - daysToShow/2 + (i * daysToShow / width);
    float value = calculateBiorhythm(day, cycle);
    int plotY = centerY - (value * height/2);
    
    if (plotY >= y && plotY < y + height) {
      int currentX = x + i;
      
      // Draw thicker point
      Disp.drawPixel(currentX, plotY, color);
      if (plotY > y) Disp.drawPixel(currentX, plotY - 1, color);
      if (plotY < y + height - 1) Disp.drawPixel(currentX, plotY + 1, color);
      
      // Connect with line to previous point
      if (prevX != -1 && prevY != -1) {
        Disp.drawLine(prevX, prevY, currentX, plotY, color);
      }
      
      prevX = currentX;
      prevY = plotY;
    }
  }
  
  // Mark today with vertical line
  int todayX = x + width/2;
  Disp.drawLine(todayX, y, todayX, y + height, WHITE);
  
  // Add percentage labels
  Disp.setTextSize(1);
  Disp.setTextColor(0x8410);
  Disp.drawString("100", x - 20, y, 1);
  Disp.drawString("50", x - 15, quarterY1 - 4, 1);
  Disp.drawString("0", x - 10, centerY - 4, 1);
  Disp.drawString("-50", x - 18, quarterY2 - 4, 1);
  Disp.drawString("-100", x - 22, y + height - 8, 1);
}

void drawMiniBiorhythmGraph(int x, int y, int width, int height) {
  int daysAlive = daysBetween(birthDate, currentDate);
  
  // Draw background
  Disp.drawRect(x, y, width, height, WHITE);
  
  // Draw center line
  Disp.drawLine(x, y + height/2, x + width, y + height/2, 0x8410);
  
  // Draw curves (simplified)
  for (int i = 0; i < width; i += 2) {
    int day = daysAlive - width/2 + i;
    
    float physical = calculateBiorhythm(day, PHYSICAL_CYCLE);
    float emotional = calculateBiorhythm(day, EMOTIONAL_CYCLE);
    float intellectual = calculateBiorhythm(day, INTELLECTUAL_CYCLE);
    
    int py = y + height/2 - (physical * height/4);
    int ey = y + height/2 - (emotional * height/4);
    int iy = y + height/2 - (intellectual * height/4);
    
    Disp.drawPixel(x + i, py, 0xF800);      // Red - Physical
    Disp.drawPixel(x + i, ey, 0xFFE0);     // Yellow - Emotional
    Disp.drawPixel(x + i, iy, 0x07FF);     // Cyan - Intellectual
  }
}

void drawBiorhythmCurve(int x, int y, int width, int height, int centerDay, int cycle, uint16_t color) {
  // Draw background box
  Disp.drawRect(x, y, width, height, WHITE);
  
  // Draw center line (0%)
  Disp.drawLine(x, y + height/2, x + width, y + height/2, 0x8410);
  
  // Draw curve
  for (int i = 0; i < width; i++) {
    int day = centerDay - width/2 + i;
    float value = calculateBiorhythm(day, cycle);
    int plotY = y + height/2 - (value * height/2);
    
    if (plotY >= y && plotY < y + height) {
      Disp.drawPixel(x + i, plotY, color);
    }
  }
  
  // Mark today
  int todayX = x + width/2;
  Disp.drawLine(todayX, y, todayX, y + height, WHITE);
}

// Example of drawing colored borders (from Halloween Tetris)
void drawColoredBorder() {
  // Halloween border colors we used
  uint16_t halloweenColors[7] = {
    0xF800, // Red/Orange
    0x7800, // Dark Orange  
    0xF81F, // Magenta/Purple
    0x07E0, // Green
    0xFFE0, // Yellow
    0x8010, // Dark Purple
    0xFC00  // Orange-Red
  };
  
  // Left border
  for(int y = 0; y < 240; y++) {
    int colorIndex = (y / 10) % 7;
    Disp.drawPixel(0, y, halloweenColors[colorIndex]);
    Disp.drawPixel(1, y, halloweenColors[colorIndex]);
  }
  
  // Right border  
  for(int y = 0; y < 240; y++) {
    int colorIndex = (y / 10) % 7;
    Disp.drawPixel(133, y, halloweenColors[colorIndex]);
    Disp.drawPixel(134, y, halloweenColors[colorIndex]);
  }
}

// Example of drawing graphs/charts for biorhythm waves
void drawSineWave(int startX, int startY, int width, int height, float frequency, uint16_t color) {
  // This will be useful for biorhythm sine waves!
  for(int x = 0; x < width; x++) {
    float angle = (x * frequency * 2.0 * PI) / width;
    int y = startY + (height / 2) + (sin(angle) * (height / 2));
    
    if(y >= 0 && y < 240 && (startX + x) < 135) {
      Disp.drawPixel(startX + x, y, color);
    }
  }
}

// Text display helpers (from Tetris UI)
void drawCenteredText(String text, int y, uint16_t color, int size = 1) {
  Disp.setTextColor(color);
  Disp.setTextSize(size);
  int textWidth = text.length() * 6 * size; // Approximate
  int x = (135 - textWidth) / 2;
  Disp.drawString(text, x, y, 1);
}

// Button handling (from Tetris controls)
bool isButtonPressed(int pin) {
  return digitalRead(pin) == 0;
}

void waitForButtonPress() {
  while(true) {
    M5.update();
    if(isButtonPressed(37)) break; // M5 button
    delay(50);
  }
  delay(200); // Debounce
}
