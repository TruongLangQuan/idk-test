#include "motion_alarm.h"

MotionAlarm::MotionAlarm() {
    currentState = ALARM_OFF;
    baselineSet = false;
    sensitivity = MOTION_THRESHOLD;
    soundEnabled = true;
    visualAlarmEnabled = true;
    armingDelay = 10000; // 10 seconds default arming delay
    isArming = false;
    
    baselineAccelX = 0;
    baselineAccelY = 0;
    baselineAccelZ = 0;
}

void MotionAlarm::begin() {
    // Initialize preferences
    preferences.begin("motion_alarm", false);
    
    // Load saved settings
    loadSettings();
    
    // Initialize IMU
    M5.Imu.init();
    
    // Calibrate baseline after a short delay
    delay(1000);
    calibrateBaseline();
}

void MotionAlarm::update() {
    unsigned long currentTime = millis();
    
    switch (currentState) {
        case ALARM_OFF:
            // Do nothing when alarm is off
            break;
            
        case ALARM_ARMED:
            // Check for motion when armed
            if (detectMotion()) {
                triggerAlarm();
            }
            break;
            
        case ALARM_TRIGGERED:
            // Handle triggered alarm
            if (currentTime - alarmStartTime > ALARM_DURATION) {
                currentState = ALARM_COOLDOWN;
                lastMotionTime = currentTime;
            }
            break;
            
        case ALARM_COOLDOWN:
            // Cooldown period before re-arming
            if (currentTime - lastMotionTime > MOTION_COOLDOWN) {
                currentState = ALARM_ARMED;
            }
            break;
    }
    
    // Handle arming process
    if (isArming && currentTime - armingTime > armingDelay) {
        currentState = ALARM_ARMED;
        isArming = false;
        calibrateBaseline(); // Recalibrate when armed
    }
}

void MotionAlarm::armAlarm() {
    if (currentState == ALARM_OFF) {
        isArming = true;
        armingTime = millis();
        
        // Optional: beep to indicate arming started
        if (soundEnabled) {
            M5.Speaker.tone(1000, 200);
            delay(250);
            M5.Speaker.tone(1000, 200);
        }
    }
}

void MotionAlarm::disarmAlarm() {
    currentState = ALARM_OFF;
    isArming = false;
    
    // Optional: beep to indicate disarmed
    if (soundEnabled) {
        M5.Speaker.tone(500, 500);
    }
}

void MotionAlarm::triggerAlarm() {
    currentState = ALARM_TRIGGERED;
    alarmStartTime = millis();
    
    // Sound alarm
    if (soundEnabled) {
        M5.Speaker.tone(2000, 1000);
    }
    
    // Visual alarm
    if (visualAlarmEnabled) {
        M5.Lcd.fillScreen(0xF800); // Red screen
        M5.Lcd.setTextColor(0xFFFF);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("ALARM!", 20, 60);
        M5.Lcd.setTextSize(1);
        M5.Lcd.drawString("MOTION DETECTED", 10, 100);
    }
}

void MotionAlarm::stopAlarm() {
    if (currentState == ALARM_TRIGGERED) {
        disarmAlarm();
    }
}

bool MotionAlarm::detectMotion() {
    if (!baselineSet) return false;
    
    float accelX, accelY, accelZ;
    M5.Imu.getAccel(&accelX, &accelY, &accelZ);
    
    // Calculate motion magnitude
    float deltaX = abs(accelX - baselineAccelX);
    float deltaY = abs(accelY - baselineAccelY);
    float deltaZ = abs(accelZ - baselineAccelZ);
    
    float motionMagnitude = sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
    
    return motionMagnitude > sensitivity;
}

void MotionAlarm::calibrateBaseline() {
    float totalX = 0, totalY = 0, totalZ = 0;
    int samples = 50;
    
    // Take multiple samples for baseline
    for (int i = 0; i < samples; i++) {
        float accelX, accelY, accelZ;
        M5.Imu.getAccel(&accelX, &accelY, &accelZ);
        
        totalX += accelX;
        totalY += accelY;
        totalZ += accelZ;
        
        delay(20);
    }
    
    baselineAccelX = totalX / samples;
    baselineAccelY = totalY / samples;
    baselineAccelZ = totalZ / samples;
    baselineSet = true;
}

float MotionAlarm::getMotionMagnitude() {
    if (!baselineSet) return 0;
    
    float accelX, accelY, accelZ;
    M5.Imu.getAccel(&accelX, &accelY, &accelZ);
    
    float deltaX = abs(accelX - baselineAccelX);
    float deltaY = abs(accelY - baselineAccelY);
    float deltaZ = abs(accelZ - baselineAccelZ);
    
    return sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
}

void MotionAlarm::displayStatus() {
    M5.Lcd.setTextSize(1);
    
    switch (currentState) {
        case ALARM_OFF:
            M5.Lcd.setTextColor(0x8410); // Gray
            M5.Lcd.drawString("Alarm: OFF", 10, 180);
            break;
            
        case ALARM_ARMED: {
            M5.Lcd.setTextColor(0x07E0); // Green
            M5.Lcd.drawString("Alarm: ARMED", 10, 180);
            
            // Show motion level
            float motion = getMotionMagnitude();
            String motionStr = "Motion: " + String(motion, 2);
            M5.Lcd.setTextColor(0xFFFF);
            M5.Lcd.drawString(motionStr, 10, 200);
            break;
        }
            
        case ALARM_TRIGGERED:
            M5.Lcd.setTextColor(0xF800); // Red
            M5.Lcd.drawString("ALARM TRIGGERED!", 10, 180);
            break;
            
        case ALARM_COOLDOWN:
            M5.Lcd.setTextColor(0xFFE0); // Yellow
            M5.Lcd.drawString("Alarm: Cooldown", 10, 180);
            break;
    }
    
    if (isArming) {
        displayArming();
    }
}

void MotionAlarm::displayArming() {
    unsigned long remaining = armingDelay - (millis() - armingTime);
    int seconds = remaining / 1000;
    
    M5.Lcd.setTextColor(0xFFE0); // Yellow
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Arming in: " + String(seconds), 10, 200);
}

void MotionAlarm::displayTriggered() {
    M5.Lcd.fillScreen(0xF800); // Red background
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("MOTION", 25, 50);
    M5.Lcd.drawString("ALARM!", 25, 80);
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Press any button", 10, 130);
    M5.Lcd.drawString("to stop alarm", 10, 150);
}

void MotionAlarm::displaySettings() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF); // Cyan
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("MOTION ALARM", 20, 10);
    
    M5.Lcd.setTextColor(0xFFFF); // White
    M5.Lcd.drawString("Sensitivity:", 10, 40);
    M5.Lcd.drawString(String(sensitivity, 1), 80, 40);
    
    M5.Lcd.drawString("Sound:", 10, 60);
    M5.Lcd.drawString(soundEnabled ? "ON" : "OFF", 60, 60);
    
    M5.Lcd.drawString("Visual:", 10, 80);
    M5.Lcd.drawString(visualAlarmEnabled ? "ON" : "OFF", 60, 80);
    
    M5.Lcd.drawString("Arm Delay:", 10, 100);
    M5.Lcd.drawString(String(armingDelay / 1000) + "s", 80, 100);
    
    M5.Lcd.setTextColor(0x8410); // Gray
    M5.Lcd.drawString("A: Arm/Disarm", 10, 140);
    M5.Lcd.drawString("B: Settings", 10, 160);
    M5.Lcd.drawString("PWR: Back", 10, 180);
}

void MotionAlarm::displayMenu() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF); // Cyan
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("MOTION ALARM", 20, 10);
    
    M5.Lcd.setTextColor(0xFFFF); // White
    
    String stateStr;
    uint16_t stateColor;
    
    switch (currentState) {
        case ALARM_OFF:
            stateStr = "OFF";
            stateColor = 0x8410; // Gray
            break;
        case ALARM_ARMED:
            stateStr = "ARMED";
            stateColor = 0x07E0; // Green
            break;
        case ALARM_TRIGGERED:
            stateStr = "TRIGGERED";
            stateColor = 0xF800; // Red
            break;
        case ALARM_COOLDOWN:
            stateStr = "COOLDOWN";
            stateColor = 0xFFE0; // Yellow
            break;
    }
    
    M5.Lcd.drawString("Status:", 10, 40);
    M5.Lcd.setTextColor(stateColor);
    M5.Lcd.drawString(stateStr, 60, 40);
    
    M5.Lcd.setTextColor(0xFFFF);
    if (baselineSet) {
        float motion = getMotionMagnitude();
        M5.Lcd.drawString("Motion:", 10, 60);
        M5.Lcd.drawString(String(motion, 2), 60, 60);
    }
    
    M5.Lcd.drawString("Threshold:", 10, 80);
    M5.Lcd.drawString(String(sensitivity, 1), 80, 80);
    
    M5.Lcd.setTextColor(0x8410); // Gray
    M5.Lcd.drawString("A: Toggle Alarm", 10, 120);
    M5.Lcd.drawString("B: Calibrate", 10, 140);
    M5.Lcd.drawString("PWR: Settings", 10, 160);
}

void MotionAlarm::setSensitivity(float sens) {
    sensitivity = sens;
    saveSettings();
}

void MotionAlarm::setSoundEnabled(bool enabled) {
    soundEnabled = enabled;
    saveSettings();
}

void MotionAlarm::setVisualAlarmEnabled(bool enabled) {
    visualAlarmEnabled = enabled;
    saveSettings();
}

void MotionAlarm::setArmingDelay(unsigned long delay) {
    armingDelay = delay;
    saveSettings();
}

void MotionAlarm::saveSettings() {
    preferences.putFloat("sensitivity", sensitivity);
    preferences.putBool("sound", soundEnabled);
    preferences.putBool("visual", visualAlarmEnabled);
    preferences.putULong("arm_delay", armingDelay);
}

void MotionAlarm::loadSettings() {
    sensitivity = preferences.getFloat("sensitivity", MOTION_THRESHOLD);
    soundEnabled = preferences.getBool("sound", true);
    visualAlarmEnabled = preferences.getBool("visual", true);
    armingDelay = preferences.getULong("arm_delay", 10000);
}

void MotionAlarm::handleMenuNavigation() {
    // This will be implemented in the main application
    // to handle joystick navigation through settings
}