#include "wifi_beacon.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include <M5StickCPlus2.h>

// WiFi channel configuration
const uint8_t channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
uint8_t channelIndex = 0;
uint8_t wifi_channel = 1;

// Custom HALLOWEEN TETRIS WiFi beacon names - 2025 Edition
const char Beacons[] PROGMEM = {"🎃_PUMPKIN_TETRIS_👻\n"
                                "SPOOKY_BLOCKS_🧙‍♀️\n"
                                "WITCH_STACK_MAGIC_⚡\n"
                                "GHOST_TETRIS_BOO_👻\n"
                                "TETЯIS_🎃_PUMPKIN_PATCH\n"
                                "HALLOWEEN_BЯICKS_🦇\n"
                                "CREEPY_TETЯIS_TOWER_💀\n"
                                "TETЯIS_👻_PHANTOM_ZONE\n"
                                "SPIDERWEB_BLOCKS_🕷️\n"
                                "TETЯIS_🧟‍♂️_ZOMBIE_HORDE\n"
                                "VAMPIRE_TETЯIS_🧛‍♂️\n"
                                "TETЯIS_🕸️_HAUNTED_HOUSE\n"
                                "SKELETON_BLOCKS_💀\n"
                                "TETЯIS_🦇_BAT_SWARM\n"
                                "FRANKENSTEIN_STACK_⚡\n"
                                "TETЯIS_👹_DEMON_666\n"
                                "MUMMY_BLOCKS_🏺\n"
                                "TETЯIS_🌙_MIDNIGHT_HOUR\n"
                                "WEREWOLF_TETЯIS_🐺\n"
                                "TETЯIS_⚰️_COFFIN_DROP\n"
                                "TRICK_OR_TETЯIS_🍬\n"
                                "TETЯIS_🔮_CRYSTAL_BALL\n"
                                "HAUNTED_ARCADE_WiFi_🏚️\n"
                                "TETЯIS_🧪_POTION_LAB\n"
                                "GRAVEYARD_BLOCKS_⚰️\n"
                                "TETЯIS_🕯️_CANDLE_MAGIC\n"
                                "BANSHEE_TETЯIS_WAIL_👻\n"
                                "TETЯIS_🎭_MASK_OF_DOOM\n"
                                "CAULDRON_BLOCKS_🧙‍♀️\n"
                                "TETЯIS_🌕_FULL_MOON\n"
                                "GHOUL_STACK_RAMPAGE_👺\n"
                                "TETЯIS_🕷️_SPIDER_QUEEN\n"
                                "POLTERGEIST_BLOCKS_👻\n"
                                "TETЯIS_🧛‍♀️_BLOOD_MOON\n"
                                "NECROMANCER_TETЯIS_💀\n"
                                "TETЯIS_🎃_JACK_O_STACK\n"
                                "PHANTOM_ARCADE_ZONE_👻\n"
                                "TETЯIS_🦉_NIGHT_OWL\n"
                                "CURSED_BLOCKS_💀\n"
                                "TETЯIS_🌟_WITCH_STAR\n"
                                "Mom Use This One\n"
                                "Abraham Linksys\n"
                                "Benjamin FrankLAN\n"
                                "Martin Router King\n"
                                "John Wilkes Bluetooth\n"
                                "Pretty Fly for a Wi-Fi\n"
                                "Bill Wi the Science Fi\n"
                                "I Believe Wi Can Fi\n"
                                "Tell My Wi-Fi Love Her\n"
                                "No More Mister Wi-Fi\n"
                                "LAN Solo\n"
                                "The LAN Before Time\n"
                                "Silence of the LANs\n"
                                "House LANister\n"
                                "Winternet Is Coming\n"
                                "Ping's Landing\n"
                                "The Ping in the North\n"
                                "This LAN Is My LAN\n"
                                "Get Off My LAN\n"
                                "The Promised LAN\n"
                                "The LAN Down Under\n"
                                "FBI Surveillance Van 4\n"
                                "Area 51 Test Site\n"
                                "Drive-By Wi-Fi\n"
                                "Planet Express\n"
                                "Wu Tang LAN\n"
                                "Darude LANstorm\n"
                                "VIRUS.EXE\n"
                                "Virus-Infected Wi-Fi\n"
                                "Starbucks Wi-Fi\n"
                                "Text 64ALL for Password\n"
                                "Yell BRUCE for Password\n"
                                "The Password Is 1234\n"
                                "Free Public Wi-Fi\n"
                                "No Free Wi-Fi Here\n"
                                "Get Your Own Damn Wi-Fi\n"
                                "It Hurts When IP\n"
                                "Dora the Internet Explorer\n"
                                "404 Wi-Fi Unavailable\n"
                                "Porque-Fi\n"
                                "Titanic Syncing\n"
                                "Test Wi-Fi Please Ignore\n"
                                "Drop It Like It's Hotspot\n"
                                "Life in the Fast LAN\n"
                                "The Creep Next Door\n"
                                "Ye Olde Internet\n"};

char randomName[32];
char emptySSID[32];

// Override function for raw frame transmission (only if needed)
// extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
//     if (arg == 31337) return 1;
//     else return 0;
// }

void generateRandomWiFiMac(uint8_t *mac) {
    mac[0] = 0x02; // Set locally administered bit
    for (int i = 1; i < 6; i++) { 
        mac[i] = random(0, 255); 
    }
}

char* randomSSID() {
    const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = random(7, 22); // Generate a random length between 7 and 21
    for (int i = 0; i < len; ++i) {
        randomName[i] = charset[random(0, strlen(charset))];
    }
    randomName[len] = '\0';
    return randomName;
}

void nextChannel() {
    if (sizeof(channels) > 1) {
        uint8_t ch = channels[channelIndex];
        channelIndex++;
        if (channelIndex >= sizeof(channels)) channelIndex = 0;

        if (ch != wifi_channel && ch >= 1 && ch <= 14) {
            wifi_channel = ch;
            esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
        }
    }
}

void beaconSpamList(const char list[]) {
    // Beacon frame definition
    uint8_t beaconPacket[109] = {
        /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: management beacon frame
        /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
        /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
        /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
        
        // Fixed parameters
        /* 22 - 23 */ 0x00, 0x00, // Fragment & sequence number
        /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
        /* 32 - 33 */ 0xe8, 0x03, // Interval: every 1s
        /* 34 - 35 */ 0x31, 0x00, // Capabilities Information
        
        // Tagged parameters
        // SSID parameters
        /* 36 - 37 */ 0x00, 0x20, // Tag: Set SSID length, Tag length: 32
        /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // SSID
        
        // Supported Rates
        /* 70 - 71 */ 0x01, 0x08, // Tag: Supported Rates, Tag length: 8
        /* 72 */ 0x82, // 1(B)
        /* 73 */ 0x84, // 2(B)
        /* 74 */ 0x8b, // 5.5(B)
        /* 75 */ 0x96, // 11(B)
        /* 76 */ 0x24, // 18
        /* 77 */ 0x30, // 24
        /* 78 */ 0x48, // 36
        /* 79 */ 0x6c, // 54
        
        // Current Channel
        /* 80 - 81 */ 0x03, 0x01, // Channel set, length
        /* 82 */ 0x01, // Current Channel
        
        // RSN information
        /*  83 -  84 */ 0x30, 0x18,
        /*  85 -  86 */ 0x01, 0x00,
        /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
        /*  91 -  92 */ 0x02, 0x00,
        /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
        /* 101 - 102 */ 0x01, 0x00,
        /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
        /* 107 - 108 */ 0x00, 0x00
    };

    int i = 0;
    int j = 0;
    char tmp;
    uint8_t macAddr[6];
    int ssidsLen = strlen_P(list);

    // Go to next channel
    nextChannel();

    while (i < ssidsLen) {
        // Read out next SSID
        j = 0;
        do {
            tmp = pgm_read_byte(list + i + j);
            j++;
        } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

        uint8_t ssidLen = j - 1;

        // Set MAC address
        generateRandomWiFiMac(macAddr);

        // Write MAC address into beacon frame
        memcpy(&beaconPacket[10], macAddr, 6);
        memcpy(&beaconPacket[16], macAddr, 6);

        // Reset SSID
        memcpy(&beaconPacket[38], emptySSID, 32);

        // Write new SSID into beacon frame
        memcpy_P(&beaconPacket[38], &list[i], ssidLen);
        
        // Set channel for beacon frame
        beaconPacket[82] = wifi_channel;
        beaconPacket[34] = 0x31; // WPA

        // Send packet
        for (int k = 0; k < 3; k++) {
            esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, sizeof(beaconPacket), 0);
            delay(1);
        }
        
        i += j;
        
        // Check for exit condition - you'll need to implement this based on your button system
        // For now, just continue
    }
}

void beaconSpam() {
    // Initialize WiFi
    WiFi.mode(WIFI_MODE_STA);
    
    // Create empty SSID
    for (int i = 0; i < 32; i++) emptySSID[i] = ' ';
    
    // Initialize random generator
    randomSeed(esp_random());
    
    // Show status on screen
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("🎃Halloween WiFi👻", 5, 20);
    M5.Lcd.drawString("Spooky Beacons...", 10, 40);
    M5.Lcd.drawString("Press M5 to stop", 10, 60);
    
    // Main beacon spam loop
    while (true) {
        beaconSpamList(Beacons);
        
        // Check for exit (M5 button)
        if (digitalRead(37) == 0) {
            delay(200); // Debounce
            break;
        }
        
        delay(10); // Small delay between cycles
    }
    
    // Cleanup
    WiFi.mode(WIFI_OFF);
    M5.Lcd.fillScreen(BLACK);
}