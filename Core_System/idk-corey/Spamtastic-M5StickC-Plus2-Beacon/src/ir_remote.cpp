#include "ir_remote.h"

IRRemote::IRRemote() : irSend(IR_SEND_PIN), irRecv(IR_RECV_PIN) {
    commandCount = 0;
    selectedCommand = 0;
}

void IRRemote::begin() {
    // Initialize IR send and receive
    irSend.begin();
    irRecv.enableIRIn();
    
    // Initialize preferences
    preferences.begin("ir_remote", false);
    
    // Load saved commands
    loadCommands();
}

void IRRemote::update() {
    // Check for received IR signals
    decode_results results;
    if (irRecv.decode(&results)) {
        // Handle received IR data
        irRecv.resume();
    }
}

void IRRemote::sendCommand(IRCommand cmd) {
    switch (cmd.protocol) {
        case IR_PROTOCOL_NEC:
            sendNEC(cmd.address, cmd.command);
            break;
        case IR_PROTOCOL_SONY:
            sendSony(cmd.address, cmd.command, cmd.bits);
            break;
        case IR_PROTOCOL_RC5:
            sendRC5(cmd.address, cmd.command);
            break;
        case IR_PROTOCOL_RC6:
            sendRC6(cmd.address, cmd.command);
            break;
        case IR_PROTOCOL_SAMSUNG:
            sendSamsung(cmd.address, cmd.command);
            break;
    }
}

void IRRemote::sendNEC(uint32_t address, uint32_t command) {
    uint32_t data = (address << 16) | command;
    irSend.sendNEC(data, 32);
    delay(50);
}

void IRRemote::sendSony(uint32_t address, uint32_t command, uint8_t bits) {
    uint32_t data = (address << 7) | command;
    irSend.sendSony(data, bits);
    delay(50);
}

void IRRemote::sendRC5(uint32_t address, uint32_t command) {
    uint32_t data = (address << 6) | command;
    irSend.sendRC5(data, 13);
    delay(50);
}

void IRRemote::sendRC6(uint32_t address, uint32_t command) {
    uint32_t data = (address << 8) | command;
    irSend.sendRC6(data, 20);
    delay(50);
}

void IRRemote::sendSamsung(uint32_t address, uint32_t command) {
    uint32_t data = (address << 16) | command;
    irSend.sendSAMSUNG(data, 32);
    delay(50);
}

bool IRRemote::learnCommand(int slot) {
    if (slot >= 10) return false;
    
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Learning IR...", 10, 50);
    M5.Lcd.drawString("Point remote and", 10, 70);
    M5.Lcd.drawString("press button", 10, 90);
    M5.Lcd.drawString("Press B to cancel", 10, 130);
    
    unsigned long startTime = millis();
    decode_results results;
    
    while (millis() - startTime < 30000) { // 30 second timeout
        M5.update();
        
        if (M5.BtnB.wasClicked()) {
            return false; // Cancelled
        }
        
        if (irRecv.decode(&results)) {
            // Successfully received IR signal
            IRCommand newCmd;
            newCmd.name = "Custom " + String(slot + 1);
            
            // Determine protocol
            switch (results.decode_type) {
                case NEC:
                    newCmd.protocol = IR_PROTOCOL_NEC;
                    break;
                case SONY:
                    newCmd.protocol = IR_PROTOCOL_SONY;
                    break;
                case RC5:
                    newCmd.protocol = IR_PROTOCOL_RC5;
                    break;
                case RC6:
                    newCmd.protocol = IR_PROTOCOL_RC6;
                    break;
                case SAMSUNG:
                    newCmd.protocol = IR_PROTOCOL_SAMSUNG;
                    break;
                default:
                    newCmd.protocol = IR_PROTOCOL_NEC; // Default
            }
            
            newCmd.address = (results.value >> 16) & 0xFFFF;
            newCmd.command = results.value & 0xFFFF;
            newCmd.bits = results.bits;
            
            savedCommands[slot] = newCmd;
            if (slot >= commandCount) {
                commandCount = slot + 1;
            }
            
            irRecv.resume();
            saveCommands();
            return true;
        }
        
        delay(50);
    }
    
    return false; // Timeout
}

IRCommand IRRemote::receiveCommand() {
    IRCommand cmd;
    decode_results results;
    
    if (irRecv.decode(&results)) {
        cmd.name = "Received";
        
        switch (results.decode_type) {
            case NEC:
                cmd.protocol = IR_PROTOCOL_NEC;
                break;
            case SONY:
                cmd.protocol = IR_PROTOCOL_SONY;
                break;
            case RC5:
                cmd.protocol = IR_PROTOCOL_RC5;
                break;
            case RC6:
                cmd.protocol = IR_PROTOCOL_RC6;
                break;
            case SAMSUNG:
                cmd.protocol = IR_PROTOCOL_SAMSUNG;
                break;
            default:
                cmd.protocol = IR_PROTOCOL_NEC;
        }
        
        cmd.address = (results.value >> 16) & 0xFFFF;
        cmd.command = results.value & 0xFFFF;
        cmd.bits = results.bits;
        
        irRecv.resume();
    }
    
    return cmd;
}

void IRRemote::displayMenu() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF); // Cyan
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("IR REMOTE", 35, 10);
    
    M5.Lcd.setTextColor(0xFFFF); // White
    M5.Lcd.drawString("A: Send Command", 10, 40);
    M5.Lcd.drawString("B: Learn Command", 10, 60);
    M5.Lcd.drawString("PWR: TV-B-Gone", 10, 80);
    M5.Lcd.drawString("Joy: Navigate", 10, 100);
    
    // Show current selection
    String selStr = "Selected: " + String(selectedCommand + 1);
    M5.Lcd.setTextColor(0xFFE0); // Yellow
    M5.Lcd.drawString(selStr, 10, 130);
    
    // Show command name if available
    if (selectedCommand < commandCount) {
        M5.Lcd.setTextColor(0x07E0); // Green
        M5.Lcd.drawString(savedCommands[selectedCommand].name, 10, 150);
    } else if (selectedCommand < 20) {
        M5.Lcd.setTextColor(0x07E0); // Green
        M5.Lcd.drawString(commonCommands[selectedCommand].name, 10, 150);
    }
}

void IRRemote::displayCommandList() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF); // Cyan
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("COMMAND LIST", 25, 10);
    
    M5.Lcd.setTextColor(0xFFFF); // White
    
    int startY = 30;
    int displayCount = min(8, max(commandCount, 8)); // Show at least common commands
    
    for (int i = 0; i < displayCount && i < 20; i++) {
        String cmdName;
        if (i < commandCount) {
            cmdName = savedCommands[i].name;
        } else {
            cmdName = commonCommands[i].name;
        }
        
        if (i == selectedCommand) {
            M5.Lcd.setTextColor(0xFFE0); // Yellow for selection
            M5.Lcd.drawString("> " + cmdName, 10, startY + i * 15);
        } else {
            M5.Lcd.setTextColor(0xFFFF); // White
            M5.Lcd.drawString("  " + cmdName, 10, startY + i * 15);
        }
    }
}

void IRRemote::handleMenuNavigation() {
    // This will be called from main loop to handle joystick navigation
    // Implementation depends on joystick integration
}

void IRRemote::saveCommands() {
    preferences.putInt("cmd_count", commandCount);
    
    for (int i = 0; i < commandCount; i++) {
        String prefix = "cmd" + String(i) + "_";
        preferences.putString((prefix + "name").c_str(), savedCommands[i].name);
        preferences.putUChar((prefix + "proto").c_str(), savedCommands[i].protocol);
        preferences.putULong((prefix + "addr").c_str(), savedCommands[i].address);
        preferences.putULong((prefix + "cmd").c_str(), savedCommands[i].command);
        preferences.putUChar((prefix + "bits").c_str(), savedCommands[i].bits);
    }
}

void IRRemote::loadCommands() {
    commandCount = preferences.getInt("cmd_count", 0);
    
    for (int i = 0; i < commandCount && i < 10; i++) {
        String prefix = "cmd" + String(i) + "_";
        savedCommands[i].name = preferences.getString((prefix + "name").c_str(), "Command " + String(i + 1));
        savedCommands[i].protocol = preferences.getUChar((prefix + "proto").c_str(), IR_PROTOCOL_NEC);
        savedCommands[i].address = preferences.getULong((prefix + "addr").c_str(), 0);
        savedCommands[i].command = preferences.getULong((prefix + "cmd").c_str(), 0);
        savedCommands[i].bits = preferences.getUChar((prefix + "bits").c_str(), 32);
    }
}

IRCommand IRRemote::getCommonCommand(int index) {
    if (index >= 0 && index < 20) {
        return commonCommands[index];
    }
    return IRCommand();
}

IRCommand IRRemote::getSavedCommand(int index) {
    if (index >= 0 && index < commandCount) {
        return savedCommands[index];
    }
    return IRCommand();
}

void IRRemote::runTVBGone() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0xF800); // Red
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("TV-B-GONE", 30, 50);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString("Sending all TV", 10, 80);
    M5.Lcd.drawString("power codes...", 10, 100);
    
    // Send common TV power codes
    for (int i = 0; i < 20; i++) {
        if (commonCommands[i].name.indexOf("Power") != -1) {
            sendCommand(commonCommands[i]);
            delay(100);
        }
    }
    
    // Add more TV power codes here
    uint32_t tvPowerCodes[] = {
        0x00FF00FF, 0x20DF10EF, 0x40BF00FF, 0x807F00FF,
        0x01FE00FF, 0x21DE10EF, 0x41BE00FF, 0x817E00FF
    };
    
    for (int i = 0; i < 8; i++) {
        irSend.sendNEC(tvPowerCodes[i], 32);
        delay(100);
    }
    
    M5.Lcd.setTextColor(0x07E0); // Green
    M5.Lcd.drawString("Complete!", 30, 130);
    delay(2000);
}