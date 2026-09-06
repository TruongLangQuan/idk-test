#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include <M5StickCPlus2.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Preferences.h>

// IR Pin Configuration (M5StickC Plus2)
#define IR_SEND_PIN 9    // IR LED output pin
#define IR_RECV_PIN 33   // IR receiver input pin

// IR Protocols
#define IR_PROTOCOL_NEC     1
#define IR_PROTOCOL_SONY    2
#define IR_PROTOCOL_RC5     3
#define IR_PROTOCOL_RC6     4
#define IR_PROTOCOL_SAMSUNG 5

// Common IR Commands
struct IRCommand {
    String name;
    uint8_t protocol;
    uint32_t address;
    uint32_t command;
    uint8_t bits;
};

class IRRemote {
private:
    IRsend irSend;
    IRrecv irRecv;
    Preferences preferences;
    
    IRCommand savedCommands[10];  // Store up to 10 commands
    int commandCount;
    int selectedCommand;
    
    // Common TV/Device commands
    IRCommand commonCommands[20] = {
        {"TV Power",     IR_PROTOCOL_NEC, 0x00FF, 0x00FF, 32},
        {"TV Vol+",      IR_PROTOCOL_NEC, 0x00FF, 0x01FE, 32},
        {"TV Vol-",      IR_PROTOCOL_NEC, 0x00FF, 0x02FD, 32},
        {"TV Ch+",       IR_PROTOCOL_NEC, 0x00FF, 0x03FC, 32},
        {"TV Ch-",       IR_PROTOCOL_NEC, 0x00FF, 0x04FB, 32},
        {"TV Mute",      IR_PROTOCOL_NEC, 0x00FF, 0x05FA, 32},
        {"AC Power",     IR_PROTOCOL_NEC, 0x20DF, 0x10EF, 32},
        {"AC Temp+",     IR_PROTOCOL_NEC, 0x20DF, 0x11EE, 32},
        {"AC Temp-",     IR_PROTOCOL_NEC, 0x20DF, 0x12ED, 32},
        {"Fan Power",    IR_PROTOCOL_NEC, 0x40BF, 0x807F, 32},
        {"Fan Speed",    IR_PROTOCOL_NEC, 0x40BF, 0x817E, 32},
        {"Light On",     IR_PROTOCOL_NEC, 0x60AF, 0x609F, 32},
        {"Light Off",    IR_PROTOCOL_NEC, 0x60AF, 0x619E, 32},
        {"DVD Power",    IR_PROTOCOL_SONY, 0x1, 0x15, 12},
        {"DVD Play",     IR_PROTOCOL_SONY, 0x1, 0x32, 12},
        {"DVD Stop",     IR_PROTOCOL_SONY, 0x1, 0x38, 12},
        {"Cable Power",  IR_PROTOCOL_RC5, 0x0, 0x0C, 13},
        {"Cable Info",   IR_PROTOCOL_RC5, 0x0, 0x0F, 13},
        {"STB Power",    IR_PROTOCOL_RC6, 0x0, 0x0C, 20},
        {"STB Guide",    IR_PROTOCOL_RC6, 0x0, 0x84, 20}
    };
    
public:
    IRRemote();
    
    // Core functions
    void begin();
    void update();
    
    // Send functions
    void sendCommand(IRCommand cmd);
    void sendNEC(uint32_t address, uint32_t command);
    void sendSony(uint32_t address, uint32_t command, uint8_t bits = 12);
    void sendRC5(uint32_t address, uint32_t command);
    void sendRC6(uint32_t address, uint32_t command);
    void sendSamsung(uint32_t address, uint32_t command);
    
    // Learning functions
    bool learnCommand(int slot);
    IRCommand receiveCommand();
    
    // Menu functions
    void displayMenu();
    void displayCommandList();
    void handleMenuNavigation();
    
    // Storage functions
    void saveCommands();
    void loadCommands();
    
    // Utility functions
    IRCommand getCommonCommand(int index);
    int getCommonCommandCount() { return 20; }
    IRCommand getSavedCommand(int index);
    int getSavedCommandCount() { return commandCount; }
    
    // TV-B-Gone functionality
    void runTVBGone();
};

#endif