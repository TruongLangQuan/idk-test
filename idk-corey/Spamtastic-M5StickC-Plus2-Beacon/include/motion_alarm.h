#ifndef MOTION_ALARM_H
#define MOTION_ALARM_H

#include <M5StickCPlus2.h>
#include <Preferences.h>

// Motion detection using IMU
#define MOTION_THRESHOLD 2.0      // Accelerometer threshold for motion detection
#define MOTION_COOLDOWN 5000      // 5 seconds cooldown between alarms
#define ALARM_DURATION 10000      // 10 seconds alarm duration

// Alarm states
enum AlarmState {
    ALARM_OFF,
    ALARM_ARMED,
    ALARM_TRIGGERED,
    ALARM_COOLDOWN
};

class MotionAlarm {
private:
    Preferences preferences;
    
    AlarmState currentState;
    unsigned long alarmStartTime;
    unsigned long lastMotionTime;
    unsigned long armingTime;
    
    // IMU baseline values
    float baselineAccelX, baselineAccelY, baselineAccelZ;
    bool baselineSet;
    
    // Sensitivity settings
    float sensitivity;
    bool soundEnabled;
    bool visualAlarmEnabled;
    
    // Arming delay
    unsigned long armingDelay;
    bool isArming;
    
public:
    MotionAlarm();
    
    // Core functions
    void begin();
    void update();
    
    // Alarm control
    void armAlarm();
    void disarmAlarm();
    void triggerAlarm();
    void stopAlarm();
    
    // Motion detection
    bool detectMotion();
    void calibrateBaseline();
    float getMotionMagnitude();
    
    // Display functions
    void displayStatus();
    void displayArming();
    void displayTriggered();
    void displaySettings();
    
    // Settings
    void setSensitivity(float sens);
    void setSoundEnabled(bool enabled);
    void setVisualAlarmEnabled(bool enabled);
    void setArmingDelay(unsigned long delay);
    
    // Status
    AlarmState getState() { return currentState; }
    bool isArmed() { return currentState == ALARM_ARMED; }
    bool isTriggered() { return currentState == ALARM_TRIGGERED; }
    
    // Storage
    void saveSettings();
    void loadSettings();
    
    // Menu functions
    void displayMenu();
    void handleMenuNavigation();
};

#endif