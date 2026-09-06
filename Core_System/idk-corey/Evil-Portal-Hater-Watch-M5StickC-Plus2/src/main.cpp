// Portal Killer Watch - Discrete Portal Defense with Savage Insults
// A watch that roasts data thieves and attacks evil portals
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>

Preferences preferences;

// Software clock (since M5Core has no hardware RTC)
unsigned long clockStartMillis = 0;
int setHours = 7;     // User-set time: hours (7:10 AM)
int setMinutes = 10;  // User-set time: minutes
int setSeconds = 0;   // User-set time: seconds

// Device modes
enum DeviceMode {
    MODE_PORTAL_HATER,
    MODE_JOKE_SCROLLER,
    MODE_JOKES,
    MODE_SETTINGS
};
DeviceMode currentMode = MODE_PORTAL_HATER;

// Main menu
bool inMainMenu = false;
int mainMenuSelection = 0;
const int MAIN_MENU_ITEMS = 6;
const char* MAIN_MENU_OPTIONS[] = {
    "Portal Hater",
    "Joke Scroller",
    "Jokes",
    "Set Clock",
    "Clock Color",
    "Brightness"
};

// Clock display settings
bool use24HourFormat = true;
int brightnessLevel = 8;
const int BRIGHTNESS_VALUES[] = {25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 255};
int clockColorIndex = 2; // Default GREEN
const uint16_t CLOCK_COLORS[] = {RED, ORANGE, GREEN, CYAN, BLUE, MAGENTA, YELLOW, WHITE};
const char* CLOCK_COLOR_NAMES[] = {"Red", "Orange", "Green", "Cyan", "Blue", "Magenta", "Yellow", "White"};
const int NUM_CLOCK_COLORS = 8;

// Screen timeout settings
bool screenTimeoutEnabled = false;
int screenTimeoutSeconds = 30;
unsigned long lastActivityTime = 0;
bool screenOff = false;

// Shake to wake settings
bool shakeToWakeEnabled = false;
int shakeSensitivity = 2; // 0=Light, 1=Medium, 2=Heavy
const float SHAKE_THRESHOLDS[] = {0.8, 1.2, 1.5}; // Acceleration thresholds

// Button handling
unsigned long btnBPressTime = 0;
const unsigned long LONG_PRESS_TIME = 1000; // 1 second hold

// Portal killer state
bool portalKillerActive = false;
bool scanningForPortals = false;
int portalKillCount = 0;
unsigned long lastScanTime = 0;
const unsigned long SCAN_INTERVAL = 30000; // Scan every 30 seconds when active
String lastAttackedPortal = ""; // Track last attacked portal

// Insult messages targeting data thieves and portal hackers
const String dataThiefInsults[] = {
    "Caught ya slippin, fool!",
    "Your portal is garbage!",
    "Stealing data? Pathetic.",
    "Your encryption is weak!",
    "Portal hacker detected!",
    "You're a script kiddie!",
    "Your phishing sucks!",
    "Amateur hour detected!",
    "Your portal is toast!",
    "Your evil portal died!",
    "Packet sniffer noob!",
    "Your honeypot stinks!",
    "Man-in-middle fail!",
    "DNS spoofing amateur!",
    "Your RAT got caught!",
    "Keylogger detected!",
    "Your botnet is dead!",
    "Credential thief fail!",
    "Your rootkit is weak!",
    "Session hijack fail!",
    "Your malware is old!",
    "Zero-day? More like zero-brain!",
    "SQL injection noob!",
    "XSS attack? Pathetic!",
    "Your DDoS is weak!",
    "Brute force amateur!",
    "Your payload failed!",
    "Ransomware wannabe!",
    "Your trojan is obvious!",
    "Spyware fail detected!",
    "Your backdoor is closed!",
    "Exploit kit trash!",
    "Your crypto is broken!",
    "Password stealer fail!",
    "Your phish got caught!",
    "Evil twin AP detected!",
    "Your MITM died!",
    "Rogue AP shutdown!",
    "Your capture portal failed!",
    "Fake hotspot destroyed!",
    "Your portal got wrecked!",
    "Data exfil blocked!",
    "Your C2 server died!",
    "Packet injection fail!",
    "Your sniffer is blind!",
    "ARP poisoning fail!",
    "Your beacon spam failed!",
    "Deauth attack blocked!",
    "Your WiFi pineapple died!",
    "Portal credentials fake!",
    "Your harvest failed!",
    "Social engineering fail!",
    "Your portal is burning!",
    "Credential harvesting blocked!",
    "Your captive portal died!",
    "Login stealer detected!",
    "Your fake AP is dead!",
    "Network infiltration fail!",
    "Your data grab failed!",
    "Information thief blocked!",
    "Your portal got nuked!",
    "Access point imposter!",
    "Your WiFi trap sprung!",
    "Man-in-browser fail!",
    "Your session steal failed!",
    "Cookie theft blocked!",
    "Your portal is toast!",
    "Authentication bypass fail!",
    "Your evil portal burned!",
    "Credential logger dead!",
    "Your WiFi attack failed!",
    "Portal defense active!",
    "Your network trap closed!",
    "Data theft prevented!",
    "Your phishing died!",
    "Portal spam incoming!",
    "Your harvest is over!",
    "Fake login destroyed!",
    "Your portal got flooded!",
    "Credential theft blocked!",
    "Your evil WiFi died!",
    "Portal attack launched!",
    "Your trap got trapped!",
    "Data stealer defeated!",
    "Your portal is spam!",
    "Network attacker blocked!",
    "Your WiFi scam failed!",
    "Portal defender active!",
    "Your data grab died!",
    "Evil portal eliminated!",
    "Your network hack failed!",
    "Portal killer active!",
    "Your WiFi trick failed!",
    "Data protection active!",
    "Your portal is history!",
    "Caught you snooping — congrats on failing at crime and life simultaneously.",
    "Wow, you pick locks about as well as you pick hobbies.",
    "You're not a hacker — you're a walking tutorial on bad decisions.",
    "Your skills are so weak even auto-fill feels sorry for you.",
    "If this is your career path, it's time to turn around. Fast.",
    "You're about three mistakes away from starring in a cautionary tale.",
    "Stealing data? Buddy, you can't even steal a nap.",
    "Please uninstall yourself from the internet.",
    "You break into systems like a toddler breaks into cookies — messy and obvious.",
    "Your hacking attempt just got outsmarted by a toaster.",
    "You're the reason cybersecurity professionals sleep well.",
    "If incompetence were a felony, you'd finally succeed at something.",
    "This attempt was so weak, it made my antivirus yawn.",
    "You're basically a screensaver pretending to be dangerous.",
    "Find a new hobby — this one requires skill.",
    "Your whole technique feels like a last-minute science project.",
    "You break more rules than you break encryption.",
    "Your keyboard begged me to stop you.",
    "You're the bicycle thief of cyberspace.",
    "You couldn't brute-force your way out of a paper bag.",
    "Your approach screams 'I watched one YouTube tutorial.'",
    "You're a bootleg villain with discount skills.",
    "Your digital footprint has 'amateur hour' written all over it.",
    "I've met CAPTCHA bots with more finesse.",
    "You're why cybersecurity budgets keep going up — thanks for the job security.",
    "This breach attempt came pre-failed.",
    "Your skillset is the reason training wheels exist.",
    "Please go touch grass before touching another login form.",
    "Even malware would be embarrassed to be seen with you.",
    "This attempt had all the grace of a dropped sandwich.",
    "You hack like you live: without a plan.",
    "You're the NPC of cybercrime — predictable and useless.",
    "Even the bugs in this system are laughing at you.",
    "Your best exploit is wasting my time.",
    "You're the participation trophy of cybercriminals.",
    "This intrusion attempt was sponsored by Failure™.",
    "If brains were bandwidth, you'd be offline.",
    "Try knitting. Literally anything else would suit you better.",
    "You're the reason tutorials start with 'Don't do this.'",
    "Your attempt was denied due to lack of effort and dignity.",
    "I've seen fortune cookies with better strategies.",
    "You're a threat to no one but yourself.",
    "Breaking news: local wannabe hacker defeated by basic security.",
    "You're the human equivalent of a weak password.",
    "Your entire skillset should come with a warning label.",
    "You're one wrong click away from infecting yourself.",
    "This attempt had all the stealth of a marching band.",
    "You can't even spell 'breach' without autocorrect.",
    "You're the knock-knock joke of cybercrime.",
    "If stupidity was encrypted, you'd finally crack something.",
    "Your technique is older than your excuses.",
    "Your best hack was convincing yourself you can hack.",
    "You're not a threat — you're a training dummy.",
    "If failure had a user ID, it would be yours.",
    "The only thing you've breached is your own dignity.",
    "You're a glitch in the tutorial level.",
    "Your persistence is impressive; your results are not.",
    "You're proof that copy-pasting commands is not a personality.",
    "My logs are laughing at you.",
    "Your effort-to-competence ratio is tragic.",
    "You're the junk folder of cybercriminals.",
    "Breaking into systems isn't for everyone. Especially you.",
    "You're the CAPTCHA equivalent of 'try again later.'",
    "Your digital fingerprints smell like desperation.",
    "You navigate cyberspace like a Roomba in a hurricane.",
    "This attempt is being saved for the blooper reel.",
    "You're the practice round, not the boss fight.",
    "Your ambition far exceeds your ability.",
    "You're about as stealthy as a car alarm.",
    "You're the weakest link — and the chain knows it.",
    "Your tools are outdated, much like your judgment.",
    "You're the melted ice cream cone of cybercrime.",
    "You're one Google search away from discovering this isn't for you.",
    "Thanks for the entertainment, failure levels like yours are rare.",
    "Final verdict: find a new hobby, champ — this one is beating you.",
    "Your intrusion attempt was so weak the firewall didn't even stretch first.",
    "You're about as threatening as a rubber spoon.",
    "This hack attempt belongs in a museum labeled 'Why It Failed.'",
    "My logs are keeping this for comedy night.",
    "You type like you're wearing oven mitts.",
    "Your idea of stealth is shouting 'I'M SNEAKING!'",
    "Congratulations, you've been defeated by basic math and common sense.",
    "You couldn't bypass a revolving door.",
    "This was so predictable even fortune cookies saw it coming.",
    "Your best exploit is giving cybersecurity analysts something to laugh at.",
    "You're the kiddie-pool version of a cyber threat.",
    "Even a broken clock hacks better than you.",
    "You're a minor inconvenience pretending to be a menace.",
    "Your code smells like fear and stack overflow threads.",
    "If effort was competence, you'd still be failing.",
    "You're the tutorial boss that everyone beats by accident.",
    "Breaking news: local amateur defeated by login screen.",
    "You fumble through cyberspace like spilled spaghetti.",
    "I've seen toddlers with more coherent inputs.",
    "Your hack attempt expired before it even launched.",
    "You're a discount villain with bargain-bin tactics.",
    "You couldn't break encryption even if it handed you the key.",
    "Your persistence is admirable; your results are tragic.",
    "Even your malware ghosted you.",
    "You're the bootleg version of a cybercriminal.",
    "You're a 404 error wearing a hoodie.",
    "If cluelessness was currency, you'd be rich.",
    "You're not a threat — you're a screensaver with delusions of grandeur.",
    "Your technique is so outdated it runs on nostalgia.",
    "You approach security like it's a vending machine.",
    "This attempt has been auto-filed under 'LOL.'",
    "Your keyboard deserves a better owner.",
    "You hack like you cook: burnt, sloppy, and barely edible.",
    "Your threat level: lukewarm soup.",
    "You couldn't exploit a wet paper bag.",
    "You're a participation trophy of cybercrime.",
    "Your entire strategy was a polite request to fail.",
    "Even password123 would reject you.",
    "You're the cyber equivalent of a tricycle with one wheel.",
    "This is the kind of attempt antivirus software uses for warmups.",
    "You're a walking, talking 'access denied.'",
    "Your best vector is disappointing people.",
    "You're a loading bar stuck at 2%.",
    "This intrusion was powered by hope and zero skill.",
    "You're a glitch in human form.",
    "Cybersecurity didn't notice you — the janitor did.",
    "You're a knockoff hacker with expired motivation.",
    "You hack like a pigeon plays chess.",
    "I've seen spam emails with more finesse.",
    "Your effort was high; your execution was subterranean.",
    "You fumbled the bag, the room, the whole building.",
    "You're the digital equivalent of stepping on a LEGO.",
    "If failure had a logo, it would be you.",
    "Your strategy is the cybersecurity version of 'winging it.'",
    "You missed the mark so hard the mark sent condolences.",
    "You couldn't compromise a cardboard box.",
    "Your plan was so transparent it didn't require decryption.",
    "You're the NPC who thinks they're the final boss.",
    "Your intrusion attempt tripped over its own shoelaces.",
    "You're the practice dummy with fragile confidence.",
    "Your execution was a crime. Against efficiency.",
    "You hack like you're afraid of success.",
    "Your entire playbook is a sticky note that says 'try?'",
    "You're a mild inconvenience at best.",
    "This hack attempt had the structural integrity of wet tissue.",
    "You are the lag spike of human potential.",
    "Your technique was so obvious even the toaster saw it coming.",
    "Trying to hack this was your third mistake today.",
    "You're a floppy disk pretending to be a threat.",
    "Your digital fingerprint looks like a cry for help.",
    "Even brute-force attacks want nothing to do with you.",
    "You hack like you're being timed — and losing.",
    "Your incompetence has reached legendary status.",
    "You're the final boss of disappointment.",
    "Result: You lose. Again. Maybe try sudoku?",
    "Your entire personality feels like a software bug they never bothered to patch.",
    "You're the reason people double-check if the door is locked — not from fear, from embarrassment.",
    "Your existence is a long-running inside joke the universe forgot to explain.",
    "You function like a broken captcha — annoying and unsolvable.",
    "Even your shadow distances itself from your decisions.",
    "You have the energy of a wet firework fired upside down.",
    "If confidence was based on results, you'd be invisible.",
    "Your competence is in beta. Permanent beta.",
    "You are the final boss of wasted potential.",
    "Your thought process requires a parental advisory warning.",
    "You are proof that not every experiment needs to be repeated.",
    "Your brain runs on trial versions with watermark restrictions.",
    "Your ambition and your ability filed for divorce.",
    "You're a motivational poster in reverse.",
    "Your problem-solving skills come pre-broken.",
    "Your life is a software update that keeps failing at 99%.",
    "You bring the same energy as an unplugged router.",
    "Your best ideas sound like rejected spam subject lines.",
    "You are a loading screen that never finishes.",
    "Your presence feels like a pop-up ad nobody asked for.",
    "Even autocorrect refuses to fix you.",
    "Your initiative left the chat years ago.",
    "You could trip over wireless internet.",
    "Your brain has buffering issues.",
    "You couldn't navigate a straight line with GPS, a map, and a guide dog.",
    "Your decisions are what happens when common sense calls in sick.",
    "You are the human equivalent of a corrupted file.",
    "Even your excuses need excuses.",
    "You have the internal stability of a folding lawn chair in a hurricane.",
    "Your problem-solving skills file for bankruptcy daily.",
    "You're a participation trophy with self-esteem issues.",
    "Your logic is the reason patience was invented.",
    "You could lose a staring contest with a wall.",
    "You are the plot twist no one wanted and everyone saw coming.",
    "Your ideas belong in time-out forever.",
    "You are a glitch pretending to be a feature.",
    "Your determination has the lifespan of a soap bubble.",
    "You're the tutorial mode for bad choices.",
    "Your common sense evaporates under light questioning.",
    "You bring the same intensity as a lukewarm handshake.",
    "You are the reason warning labels exist.",
    "Your skill level is the digital equivalent of dial-up.",
    "You are a user error given walking privileges.",
    "Your confidence is wildly out of sync with your output.",
    "You generate disappointment at factory-setting efficiency.",
    "You fail with such style it should be taught in art class.",
    "Your brain runs on whatever is left after the battery icon turns red.",
    "You are the aftertaste of a bad idea.",
    "Your intelligence pings the server and gets 'no response.'",
    "You're the final draft of a mistake.",
    "Your planning skills are a public safety concern.",
    "You are an unskippable ad in human form.",
    "Your thinking is the spiritual successor to static noise.",
    "You're the boss battle players beat accidentally.",
    "Your best argument has a plot hole the size of your confidence.",
    "You are the patch note that says 'miscellaneous fixes' — meaningless and forgettable.",
    "Your efforts collapse under the weight of their own mediocrity.",
    "You contribute to conversations like a dropped call.",
    "Your judgment is powered by outdated firmware.",
    "You are the moral equivalent of clicking the wrong link.",
    "Your success rate is what statisticians call 'statistically amusing.'",
    "You break down faster than cheap headphones.",
    "Your entire vibe feels auto-generated on low settings.",
    "You are the practice round nobody takes seriously.",
    "Your decision-making is a spectator sport.",
    "You are the slowest-loading part of the human experience.",
    "You're the Easter egg no one bothered hiding.",
    "Your aspirations need tech support.",
    "You bring the chaos of a missing semicolon.",
    "Your confidence is built on fictional backstory.",
    "You are the human version of a typo that somehow made it to print.",
    "Your thought process takes detours into dead ends.",
    "You have the intensity of a sleeping goldfish.",
    "You are the spiritual embodiment of 'not great, not terrible.'",
    "You are a cosmic reminder that not every process concludes successfully.",
    "Your existence feels like a loading error the universe forgot to fix.",
    "You have the unstoppable momentum of a paper airplane thrown in a hurricane.",
    "Your IQ called — it's filing for emancipation.",
    "You radiate the confidence of someone who doesn't realize the toaster is unplugged.",
    "You are the final stage of a failed experiment nobody documented.",
    "Your ideas hit the ground faster than bad Wi-Fi.",
    "You bring the emotional depth of a voicemail beep.",
    "You're the reason the progress bar stays at 0%.",
    "Your brain activity is the spiritual cousin of airplane mode.",
    "You generate confusion like it's renewable energy.",
    "You're a pop-up ad with legs and misplaced optimism.",
    "Your thought process is a maze with all the walls missing.",
    "You're the sequel nobody asked for and everyone regrets.",
    "Your best decisions are on backorder indefinitely.",
    "Your initiative is powered by a dead AA battery.",
    "You're a system crash pretending to be a reboot.",
    "Your reasoning is sponsored by random number generators.",
    "You think like the warning label on a cheap blender.",
    "You're a motivational speech in reverse.",
    "Your ideas should come with a 'return to sender' label.",
    "You're a smoke alarm with no battery and too much confidence.",
    "Your potential is trapped in a permanent maintenance window.",
    "If common sense was a currency, you'd be in debt.",
    "Your logic evaporates faster than spilled vodka.",
    "You're a corrupted save file with delusional ambitions.",
    "Your presence feels like lag on a boss fight.",
    "You're the blooper reel of human decision-making.",
    "Your brain takes coffee breaks during emergencies.",
    "You're a storm cloud that forgot how to rain.",
    "Your ideas crash harder than a bargain-bin drone.",
    "You're a low-resolution disappointment.",
    "Your confidence is wildly unlicensed.",
    "Your life is a tutorial level you keep failing.",
    "You bring the intensity of a paperclip in a thunderstorm.",
    "Your choices need a legal disclaimer.",
    "Your strategy is what happens when chaos gets tired.",
    "You are a cosmic hiccup in the timeline.",
    "Your competence fell off the map and took your dignity with it.",
    "You're a push notification nobody enables.",
    "Your plan was outdated before you thought of it.",
    "You're a factory-reset version of mediocrity.",
    "Your effort and your results haven't met yet.",
    "Your thinking is a speedrun of confusion.",
    "You're a spilled drink on the keyboard of life.",
    "Your goals need an exorcism.",
    "You're the cold fries of humanity.",
    "Your mind is running a background process called 'failure.'",
    "You bring the impact of a rubber hammer.",
    "Your decisions were handcrafted by poor judgment.",
    "You're the unplugged extension cord of ambition.",
    "Your attempts are so weak even gravity ignores them.",
    "You have the mental Wi-Fi strength of a basement from 1992.",
    "You're a glitch hoping to be a feature someday.",
    "Your skills are archived in a folder labeled 'why bother.'",
    "You couldn't shine even in a blackout.",
    "Your problem-solving skills were discontinued.",
    "You're the lag spike that ruins the raid.",
    "Your competence is a myth told around campfires.",
    "You are the error message that appears even when nothing is wrong.",
    "Your ambition is running on fumes and excuses.",
    "You're the boot sequence that freezes at startup.",
    "Your ideas should come with a hazard warning.",
    "You're a motivational poster rewritten by despair.",
    "Your intelligence is on a coffee break permanently.",
    "You're a rerun of a show that got canceled mid-season.",
    "Your presence is the human equivalent of low battery mode.",
    "Your failures have unionized.",
    "You are the distant cousin of bad decisions.",
    "Your purpose crashed and never restarted.",
    "You're a broken compass in a fog machine.",
    "Your thinking needs a software rollback.",
    "You bring the thrill of a muted alarm.",
    "Your existence is the universe's debugging session.",
    "You're the lost instruction manual for a product no one wanted.",
    "You are the cliffhanger that leads to cancellation.",
    "You're the ransomware nobody bothered to patch.",
    "Your ambition is a phishing email written in crayon.",
    "You're the human embodiment of a corrupted ZIP file.",
    "Your brain is running malware disguised as common sense.",
    "You're the Trojan horse that forgot to hide.",
    "Your logic is a denial-of-service attack on yourself.",
    "You're the lag spike that ruins every raid.",
    "Your ideas buffer forever and never load.",
    "You're the spinning wheel of disappointment.",
    "Your thought process is stuck in safe mode.",
    "You're the reboot nobody asked for.",
    "Your competence times out after 30 seconds.",
    "You're a password written on a sticky note.",
    "Your strategy is weaker than '1234.'",
    "You're the CAPTCHA nobody can solve because you make no sense.",
    "Your brain is a firewall with all the ports open.",
    "You're the expired SSL certificate of humanity.",
    "Your ideas leak faster than unencrypted data.",
    "You're the patch note that only adds bugs.",
    "Your existence is a failed penetration test.",
    "You're the exploit that crashes the tutorial level.",
    "Your confidence is a phishing scam with typos.",
    "You're the brute-force attempt that locks everyone out.",
    "Your presence is the human equivalent of spam mail.",
};
const int numInsults = sizeof(dataThiefInsults) / sizeof(dataThiefInsults[0]);

// Function prototypes
void drawClock();
void scanForEvilPortals();
void attackPortal(String ssid);
void displayMessage(String msg, uint16_t color = WHITE);
void drawMainMenu();
void handleMainMenuInput();
void setTimeMenu();
void brightnessMenu();
void setTimeMenu();
void screenTimeoutMenu();
void shakeToWakeMenu();
void clockColorMenu();
void updateActivity();
void loadSettings();
void saveSettings();

void setup() {
    M5.begin();
    Serial.begin(115200);
    
    // Load saved settings from flash
    loadSettings();
    
    // Seed random number generator for true randomness
    randomSeed(esp_random());
    
    // Initialize software clock (M5Core has no hardware RTC)
    clockStartMillis = millis();
    Serial.println("Software clock initialized");
    
    M5.Display.setRotation(1);
    M5.Display.setBrightness(BRIGHTNESS_VALUES[brightnessLevel]);
    M5.Display.fillScreen(BLACK);
    
    // Startup message
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(RED);
    M5.Display.setCursor(10, 20);
    M5.Display.println("PORTAL");
    M5.Display.setCursor(10, 40);
    M5.Display.println("KILLER");
    M5.Display.setCursor(10, 60);
    M5.Display.println("WATCH");
    
    delay(2000);
    M5.Display.fillScreen(BLACK);
    
    Serial.println("Portal Killer Watch Started");
    Serial.println("Button A: Toggle Portal Killer Mode");
    Serial.println("Button B: MENU");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

void loop() {
    M5.update();
    
    // Check for screen timeout
    if (screenTimeoutEnabled && !screenOff) {
        if (millis() - lastActivityTime > (screenTimeoutSeconds * 1000)) {
            M5.Display.setBrightness(0);
            screenOff = true;
        }
    }
    
    // Wake screen on any button press or shake
    if (screenOff) {
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
            M5.Display.setBrightness(BRIGHTNESS_VALUES[brightnessLevel]);
            screenOff = false;
            updateActivity();
        } else if (shakeToWakeEnabled) {
            M5.Imu.update();
            auto data = M5.Imu.getImuData();
            if (abs(data.accel.y) > SHAKE_THRESHOLDS[shakeSensitivity]) {
                M5.Display.setBrightness(BRIGHTNESS_VALUES[brightnessLevel]);
                screenOff = false;
                updateActivity();
            }
        }
        
        // Portal killer continues running even when screen is off
        if (portalKillerActive) {
            unsigned long now = millis();
            if (now - lastScanTime >= SCAN_INTERVAL) {
                scanForEvilPortals();
                lastScanTime = now;
            }
        }
        
        delay(100);
        return; // Don't process other inputs when screen is off
    }
    
    // Button B: Short press = open main menu
    if (M5.BtnB.wasPressed() && !inMainMenu) {
        inMainMenu = true;
        mainMenuSelection = 0;
        M5.Display.fillScreen(BLACK);
        drawMainMenu();
        updateActivity();
        // Wait for button B to be released before entering menu handler
        while (M5.BtnB.isPressed()) {
            M5.update();
            delay(10);
        }
        delay(50); // Debounce
    }
    
    // Handle main menu mode
    if (inMainMenu) {
        handleMainMenuInput();
        delay(100);
        return;
    }
    
    // Button A: Toggle Portal Killer Mode (only in Portal Hater mode)
    if (M5.BtnA.wasPressed() && currentMode == MODE_PORTAL_HATER) {
        portalKillerActive = !portalKillerActive;
        M5.Display.fillScreen(BLACK);
        updateActivity();
        
        if (portalKillerActive) {
            displayMessage("PORTAL KILLER\nACTIVE!", RED);
            Serial.println("Portal Killer Mode: ACTIVE");
            lastScanTime = 0; // Force immediate scan
            portalKillCount = 0;
        } else {
            displayMessage("PORTAL KILLER\nOFF", GREEN);
            Serial.println("Portal Killer Mode: OFF");
        }
        delay(1500);
        M5.Display.fillScreen(BLACK);
    }
    
    // ANY BUTTON stops portal killer if it's active
    if (portalKillerActive && (M5.BtnB.wasPressed() || M5.BtnC.wasPressed())) {
        portalKillerActive = false;
        M5.Display.fillScreen(BLACK);
        displayMessage("PORTAL KILLER\nSTOPPED", YELLOW);
        Serial.println("Portal Killer Mode: STOPPED");
        delay(1000);
        M5.Display.fillScreen(BLACK);
    }
    
    // Portal scanning and attack logic
    if (portalKillerActive) {
        unsigned long now = millis();
        if (now - lastScanTime >= SCAN_INTERVAL) {
            scanForEvilPortals();
            lastScanTime = now;
        }
    }
    
    // Always draw clock (with portal status below if active)
    drawClock();
    
    delay(100);
}

void drawClock() {
    static int lastMinute = -1;
    
    // Calculate current time from millis() and user-set time
    unsigned long elapsedMillis = millis() - clockStartMillis;
    unsigned long totalSeconds = (setHours * 3600UL + setMinutes * 60UL + setSeconds) + (elapsedMillis / 1000);
    
    int currentHours = (totalSeconds / 3600) % 24;
    int currentMinutes = (totalSeconds / 60) % 60;
    int currentSeconds = totalSeconds % 60;
    
    // Only clear screen on minute change when portal killer is OFF
    if (currentMinutes != lastMinute && !portalKillerActive) {
        M5.Display.fillScreen(BLACK);
        lastMinute = currentMinutes;
    }
    
    char timeBuf[16];
    if (use24HourFormat) {
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", 
                 currentHours, currentMinutes, currentSeconds);
    } else {
        int displayHour = currentHours % 12;
        if (displayHour == 0) displayHour = 12;
        snprintf(timeBuf, sizeof(timeBuf), "%2d:%02d:%02d", 
                 displayHour, currentMinutes, currentSeconds);
    }
    
    // Clock at top 1/3 of screen
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(CLOCK_COLORS[clockColorIndex], BLACK);
    M5.Display.setCursor(20, 10);
    M5.Display.println(timeBuf);
    
    // Show Portal Killer status below clock
    M5.Display.setTextSize(2);  // Double size for portal status
    if (portalKillerActive) {
        M5.Display.setTextColor(RED, BLACK);
        M5.Display.setCursor(5, 50);
        M5.Display.println("PORTAL KILLER:");
        M5.Display.setCursor(5, 70);
        M5.Display.println("ACTIVE");
        
        M5.Display.setTextColor(YELLOW, BLACK);
        M5.Display.setCursor(5, 100);
        M5.Display.printf("Kills: %d", portalKillCount);
        
        M5.Display.setTextColor(GREEN, BLACK);
        M5.Display.setCursor(5, 125);
        M5.Display.println("Scanning...");
        
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(DARKGREY, BLACK);
        M5.Display.setCursor(5, 220);
        M5.Display.print("A:Off  B:Menu");
    } else {
        M5.Display.setTextColor(DARKGREY, BLACK);
        M5.Display.setCursor(5, 50);
        M5.Display.print("Portal Killer:");
        M5.Display.setCursor(5, 70);
        M5.Display.print("OFF");
        M5.Display.setTextSize(1);
        M5.Display.setCursor(5, 220);
        M5.Display.print("A:Activate  B:Menu");
    }
}

void scanForEvilPortals() {
    Serial.println("Scanning for evil portals...");
    
    // Ensure WiFi is in proper scanning mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Clear only the status area below clock (not the whole screen)
    M5.Display.fillRect(0, 45, 240, 90, BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(YELLOW, BLACK);
    M5.Display.setCursor(5, 60);
    M5.Display.println("SCANNING NETWORKS...");
    
    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        M5.Display.fillRect(0, 60, 240, 15, BLACK);
        M5.Display.setCursor(5, 60);
        M5.Display.println("No networks found");
        delay(1000);
        return;
    }
    
    // Look for open networks (potential evil portals)
    bool foundLastPortal = false;
    for (int i = 0; i < n; i++) {
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
            String ssid = WiFi.SSID(i);
            
            // Check if this is the portal we just attacked
            if (ssid == lastAttackedPortal) {
                foundLastPortal = true;
            }
            
            Serial.printf("Evil portal detected: %s\n", ssid.c_str());
            
            // Attack the portal with random insult
            lastAttackedPortal = ssid;
            attackPortal(ssid);
            portalKillCount++;
            
            delay(1000);
        }
    }
    
    // If we attacked a portal before and it's now gone, show nuked message
    if (lastAttackedPortal != "" && !foundLastPortal) {
        displayMessage("PORTAL\nNUKED!", GREEN);
        lastAttackedPortal = "";
        delay(1500);
    }
    
    WiFi.scanDelete();
}

void attackPortal(String ssid) {
    // Clear only status area below clock
    M5.Display.fillRect(0, 45, 320, 195, BLACK);  // Use full M5Core width
    M5.Display.setTextSize(2);  // Double size
    M5.Display.setTextColor(RED, BLACK);
    M5.Display.setCursor(10, 50);
    M5.Display.println("ATTACKING:");
    
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(10, 75);
    M5.Display.println(ssid.substring(0, 18)); // Show SSID (shorter for size 2)
    
    // Pick random insult for display
    int insultIndex = random(0, numInsults);
    String insult = dataThiefInsults[insultIndex];
    
    M5.Display.setTextSize(2);  // Keep size 2 for insult
    M5.Display.setTextColor(YELLOW, BLACK);
    
    // Word wrap the insult (proper word boundaries)
    int maxWidth = 18; // Characters per line at size 2
    int yPos = 105;
    int startPos = 0;
    
    while (startPos < insult.length() && yPos < 220) {
        int endPos = startPos + maxWidth;
        
        // If we're not at the end of the string
        if (endPos < insult.length()) {
            // Look backwards for a space to break at
            int spacePos = insult.lastIndexOf(' ', endPos);
            
            // If we found a space after our start position, break there
            if (spacePos > startPos) {
                endPos = spacePos;
            }
        } else {
            endPos = insult.length();
        }
        
        String line = insult.substring(startPos, endPos);
        M5.Display.setCursor(10, yPos);
        M5.Display.println(line);
        
        // Skip the space for next line
        startPos = endPos;
        if (startPos < insult.length() && insult.charAt(startPos) == ' ') {
            startPos++;
        }
        
        yPos += 18;  // Bigger spacing for size 2
    }
    
    Serial.printf("Attacking %s with: %s\n", ssid.c_str(), insult.c_str());
    
    // Try to connect and spam the portal
    WiFi.begin(ssid.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) {
        delay(100);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected! Launching counterattack...");
        
        // Get the gateway IP (portal's actual IP)
        IPAddress gateway = WiFi.gatewayIP();
        String gatewayStr = gateway.toString();
        Serial.printf("Gateway IP: %s\n", gatewayStr.c_str());
        
        // Common portal endpoints (WiFi Pineapple, generic portals)
        String commonEndpoints[] = {"/login", "/auth", "/signin", "/portal", "/admin"};
        
        // NEMO portal endpoints (172.0.0.1)
        String nemoEndpoints[] = {"/post", "/post", "/post"}; // Hit /post multiple times
        
        // Attack both common portals and NEMO-style portals
        for (int i = 0; i < 3; i++) {
            // Pick NEW random insults for each attack
            int msgInsultIndex = random(0, numInsults);
            String msgInsult = dataThiefInsults[msgInsultIndex];
            int msgInsultIndex2 = random(0, numInsults);
            String msgInsult2 = dataThiefInsults[msgInsultIndex2];
            
            // Attack common portal endpoints (192.168.4.1 style)
            for (String endpoint : commonEndpoints) {
                HTTPClient http;
                String url = "http://" + gatewayStr + endpoint;
                http.begin(url);
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                
                String fakeData = "username=hacker" + String(random(1000, 9999)) + 
                                 "&password=" + msgInsult.substring(0, min(16, (int)msgInsult.length())) +
                                 "&email=trash" + String(random(1000, 9999)) + "@dump.com";
                
                http.POST(fakeData);
                http.end();
                delay(30);
            }
            
            // Attack NEMO portal style (email + password parameters)
            for (String endpoint : nemoEndpoints) {
                HTTPClient http;
                String url = "http://" + gatewayStr + endpoint;
                http.begin(url);
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                
                // NEMO expects "email" and "password" parameters
                String fakeEmail = "trash" + String(random(1000, 9999)) + "@" + msgInsult.substring(0, min(8, (int)msgInsult.length())) + ".com";
                String fakePassword = msgInsult2.substring(0, min(20, (int)msgInsult2.length()));
                String fakeData = "email=" + fakeEmail + "&password=" + fakePassword;
                
                http.POST(fakeData);
                http.end();
                delay(30);
            }
        }
        
        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_STA);
        Serial.println("Counterattack complete!");
        
        displayMessage("PORTAL\nATTACKED!", YELLOW);
    } else {
        Serial.println("Couldn't connect to portal");
        displayMessage("PORTAL\nDODGED!", BLUE);
    }
    
    delay(1500);
}

void displayMessage(String msg, uint16_t color) {
    // Clear only status area below clock
    M5.Display.fillRect(0, 45, 240, 90, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(color, BLACK);
    
    // Simple word wrap in status area
    int yPos = 60;
    int startPos = 0;
    while (startPos < msg.length()) {
        int newlinePos = msg.indexOf('\n', startPos);
        if (newlinePos == -1) newlinePos = msg.length();
        
        String line = msg.substring(startPos, newlinePos);
        M5.Display.setCursor(10, yPos);
        M5.Display.println(line);
        
        startPos = newlinePos + 1;
        yPos += 20;
    }
}

void updateActivity() {
    lastActivityTime = millis();
}

void drawMainMenu() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(YELLOW, BLACK);
    M5.Display.setCursor(50, 10);
    M5.Display.println("MAIN MENU");
    
    M5.Display.setTextSize(2);
    
    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
        if (i == mainMenuSelection) {
            M5.Display.setTextColor(BLACK, GREEN);
        } else {
            M5.Display.setTextColor(WHITE, BLACK);
        }
        M5.Display.setCursor(20, 50 + (i * 25));
        M5.Display.println(MAIN_MENU_OPTIONS[i]);
    }
    
    // Button help at bottom
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, BLACK);
    M5.Display.setCursor(10, 220);
    M5.Display.print("A:Up  B:Select  C:Down");
}

void handleMainMenuInput() {
    static int lastSelection = -1;
    bool selectionChanged = false;
    
    if (M5.BtnA.wasPressed()) {
        // Button A navigates UP in menu
        mainMenuSelection = (mainMenuSelection - 1 + MAIN_MENU_ITEMS) % MAIN_MENU_ITEMS;
        updateActivity();
        selectionChanged = true;
    }
    
    if (M5.BtnC.wasPressed()) {
        // Button C navigates DOWN in menu
        mainMenuSelection = (mainMenuSelection + 1) % MAIN_MENU_ITEMS;
        updateActivity();
        selectionChanged = true;
    }
    
    // Only redraw if selection changed
    if (selectionChanged || lastSelection != mainMenuSelection) {
        lastSelection = mainMenuSelection;
        drawMainMenu();
    }
    
    if (M5.BtnB.wasPressed()) {
        updateActivity();
        switch (mainMenuSelection) {
            case 0: // Portal Hater
                currentMode = MODE_PORTAL_HATER;
                inMainMenu = false;
                M5.Display.fillScreen(BLACK);
                displayMessage("PORTAL HATER\nMODE", GREEN);
                delay(1000);
                M5.Display.fillScreen(BLACK);
                // Wait for button release
                while (M5.BtnB.isPressed()) {
                    M5.update();
                    delay(10);
                }
                delay(100); // Extra debounce
                break;
            case 1: // Joke Scroller
                currentMode = MODE_JOKE_SCROLLER;
                inMainMenu = false;
                M5.Display.fillScreen(BLACK);
                displayMessage("JOKE SCROLLER\nCOMING SOON", YELLOW);
                delay(1500);
                currentMode = MODE_PORTAL_HATER; // Revert for now
                M5.Display.fillScreen(BLACK);
                break;
            case 2: // Jokes
                currentMode = MODE_JOKES;
                inMainMenu = false;
                M5.Display.fillScreen(BLACK);
                displayMessage("JOKES MODE\nCOMING SOON", YELLOW);
                delay(1500);
                currentMode = MODE_PORTAL_HATER; // Revert for now
                M5.Display.fillScreen(BLACK);
                break;
            case 3: // Set Clock
                setTimeMenu();
                lastSelection = -1;
                break;
            case 4: // Clock Color
                clockColorMenu();
                lastSelection = -1;
                break;
            case 5: // Brightness
                brightnessMenu();
                lastSelection = -1;
                break;
        }
    }
}

void brightnessMenu() {
    while (true) {
        M5.update();
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(YELLOW, BLACK);
        M5.Display.setCursor(40, 10);
        M5.Display.println("BRIGHTNESS");
        
        M5.Display.setTextSize(3);
        M5.Display.setTextColor(WHITE, BLACK);
        M5.Display.setCursor(80, 60);
        M5.Display.printf("%d", BRIGHTNESS_VALUES[brightnessLevel]);
        
        // Draw brightness bar
        int barWidth = (BRIGHTNESS_VALUES[brightnessLevel] * 200) / 255;
        M5.Display.fillRect(60, 110, barWidth, 20, GREEN);
        M5.Display.drawRect(60, 110, 200, 20, WHITE);
        
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(DARKGREY, BLACK);
        M5.Display.setCursor(60, 150);
        M5.Display.print("A:Up C:Down B:Save");
        
        if (M5.BtnA.wasPressed()) {
            brightnessLevel = (brightnessLevel + 1) % 11;
            M5.Display.setBrightness(BRIGHTNESS_VALUES[brightnessLevel]);
        }
        
        if (M5.BtnC.wasPressed()) {
            brightnessLevel = (brightnessLevel - 1 + 11) % 11;
            M5.Display.setBrightness(BRIGHTNESS_VALUES[brightnessLevel]);
        }
        
        if (M5.BtnB.wasPressed()) {
            M5.Display.fillScreen(BLACK);
            return;
        }
        
        delay(100);
    }
}

void setTimeMenu() {
    int hour = 12;
    int minute = 0;
    bool settingHour = true;
    
    while (true) {
        M5.update();
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(YELLOW, BLACK);
        M5.Display.setCursor(20, 10);
        M5.Display.println("SET TIME");
        
        M5.Display.setTextSize(3);
        if (settingHour) {
            M5.Display.setTextColor(GREEN, BLACK);
        } else {
            M5.Display.setTextColor(WHITE, BLACK);
        }
        M5.Display.setCursor(40, 50);
        M5.Display.printf("%02d", hour);
        
        M5.Display.setTextColor(WHITE, BLACK);
        M5.Display.print(":");
        
        if (!settingHour) {
            M5.Display.setTextColor(GREEN, BLACK);
        } else {
            M5.Display.setTextColor(WHITE, BLACK);
        }
        M5.Display.printf("%02d", minute);
        
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(DARKGREY, BLACK);
        M5.Display.setCursor(10, 110);
        M5.Display.print("B:Change A:Next/Save");
        
        if (M5.BtnB.wasPressed()) {
            updateActivity();
            if (settingHour) {
                hour = (hour + 1) % 24;
            } else {
                minute = (minute + 1) % 60;
            }
        }
        
        if (M5.BtnA.wasPressed()) {
            updateActivity();
            if (settingHour) {
                settingHour = false;
            } else {
                // Save time to software clock
                setHours = hour;
                setMinutes = minute;
                setSeconds = 0;
                clockStartMillis = millis(); // Reset the clock base time
                M5.Display.fillScreen(BLACK);
                return;
            }
        }
        
        delay(100);
    }
}

void screenTimeoutMenu() {
    int timeoutOptions[] = {10, 30, 60, 120, 300}; // seconds
    int currentOption = 1; // default 30s
    
    // Find current setting
    for (int i = 0; i < 5; i++) {
        if (timeoutOptions[i] == screenTimeoutSeconds) {
            currentOption = i;
            break;
        }
    }
    
    bool needsRedraw = true;
    
    while (true) {
        M5.update();
        
        if (needsRedraw) {
            M5.Display.fillScreen(BLACK);
            M5.Display.setTextSize(2);
            M5.Display.setTextColor(YELLOW, BLACK);
            M5.Display.setCursor(10, 10);
            M5.Display.println("SCREEN TIMEOUT");
            
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(WHITE, BLACK);
            M5.Display.setCursor(10, 50);
            M5.Display.println("Enable:");
            M5.Display.setCursor(80, 50);
            M5.Display.setTextColor(screenTimeoutEnabled ? GREEN : RED, BLACK);
            M5.Display.println(screenTimeoutEnabled ? "ON" : "OFF");
            
            M5.Display.setTextColor(WHITE, BLACK);
            M5.Display.setCursor(10, 70);
            M5.Display.println("Time:");
            M5.Display.setCursor(80, 70);
            M5.Display.setTextColor(GREEN, BLACK);
            M5.Display.printf("%ds", timeoutOptions[currentOption]);
            
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(DARKGREY, BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.print("B:Change PWR:On/Off A:Save");
            
            needsRedraw = false;
        }
        
        if (M5.BtnB.wasPressed()) {
            updateActivity();
            if (!screenTimeoutEnabled) {
                screenTimeoutEnabled = true;
            } else {
                currentOption = (currentOption + 1) % 5;
            }
            needsRedraw = true;
        }
        
        // PWR button toggles on/off
        if (M5.BtnPWR.wasPressed()) {
            updateActivity();
            screenTimeoutEnabled = !screenTimeoutEnabled;
            needsRedraw = true;
        }
        
        if (M5.BtnA.wasPressed()) {
            updateActivity();
            screenTimeoutSeconds = timeoutOptions[currentOption];
            saveSettings(); // Save to flash
            M5.Display.fillScreen(BLACK);
            return;
        }
        
        delay(100);
    }
}

void shakeToWakeMenu() {
    const char* sensitivityNames[] = {"Light", "Medium", "Heavy"};
    bool needsRedraw = true;
    
    while (true) {
        M5.update();
        
        if (needsRedraw) {
            M5.Display.fillScreen(BLACK);
            M5.Display.setTextSize(2);
            M5.Display.setTextColor(YELLOW, BLACK);
            M5.Display.setCursor(10, 10);
            M5.Display.println("SHAKE TO WAKE");
            
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(WHITE, BLACK);
            M5.Display.setCursor(10, 50);
            M5.Display.println("Enable:");
            M5.Display.setCursor(80, 50);
            M5.Display.setTextColor(shakeToWakeEnabled ? GREEN : RED, BLACK);
            M5.Display.println(shakeToWakeEnabled ? "ON" : "OFF");
            
            M5.Display.setTextColor(WHITE, BLACK);
            M5.Display.setCursor(10, 70);
            M5.Display.println("Sensitivity:");
            M5.Display.setCursor(80, 70);
            M5.Display.setTextColor(GREEN, BLACK);
            M5.Display.println(sensitivityNames[shakeSensitivity]);
            
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(DARKGREY, BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.print("B:Change PWR:On/Off A:Save");
            
            needsRedraw = false;
        }
        
        if (M5.BtnB.wasPressed()) {
            updateActivity();
            if (!shakeToWakeEnabled) {
                shakeToWakeEnabled = true;
            } else {
                shakeSensitivity = (shakeSensitivity + 1) % 3;
            }
            needsRedraw = true;
        }
        
        // PWR button toggles on/off
        if (M5.BtnPWR.wasPressed()) {
            updateActivity();
            shakeToWakeEnabled = !shakeToWakeEnabled;
            needsRedraw = true;
        }
        
        if (M5.BtnA.wasPressed()) {
            updateActivity();
            saveSettings(); // Save to flash
            M5.Display.fillScreen(BLACK);
            return;
        }
        
        delay(100);
    }
}

void clockColorMenu() {
    bool needsRedraw = true;
    
    while (true) {
        M5.update();
        
        if (needsRedraw) {
            M5.Display.fillScreen(BLACK);
            M5.Display.setTextSize(2);
            M5.Display.setTextColor(YELLOW, BLACK);
            M5.Display.setCursor(20, 10);
            M5.Display.println("CLOCK COLOR");
            
            // Show current color name
            M5.Display.setTextSize(2);
            M5.Display.setTextColor(CLOCK_COLORS[clockColorIndex], BLACK);
            M5.Display.setCursor(40, 50);
            M5.Display.println(CLOCK_COLOR_NAMES[clockColorIndex]);
            
            // Show preview time
            M5.Display.setTextSize(3);
            M5.Display.setTextColor(CLOCK_COLORS[clockColorIndex], BLACK);
            M5.Display.setCursor(30, 80);
            M5.Display.println("12:34:56");
            
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(DARKGREY, BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.print("B:Change A:Save");
            
            needsRedraw = false;
        }
        
        if (M5.BtnB.wasPressed()) {
            updateActivity();
            clockColorIndex = (clockColorIndex + 1) % NUM_CLOCK_COLORS;
            needsRedraw = true;
        }
        
        if (M5.BtnA.wasPressed()) {
            updateActivity();
            saveSettings(); // Save to flash
            M5.Display.fillScreen(BLACK);
            return;
        }
        
        delay(100);
    }
}

void loadSettings() {
    preferences.begin("portalKiller", false);
    
    // Load all settings with defaults if not set
    use24HourFormat = preferences.getBool("use24Hour", true);
    brightnessLevel = preferences.getInt("brightness", 8);
    clockColorIndex = preferences.getInt("clockColor", 2);
    screenTimeoutEnabled = preferences.getBool("timeoutOn", false);
    screenTimeoutSeconds = preferences.getInt("timeoutSec", 30);
    shakeToWakeEnabled = preferences.getBool("shakeOn", false);
    shakeSensitivity = preferences.getInt("shakeSens", 2);
    
    preferences.end();
    
    Serial.println("Settings loaded from flash");
    Serial.printf("24hr: %d, Brightness: %d, Color: %d\n", use24HourFormat, brightnessLevel, clockColorIndex);
    Serial.printf("Timeout: %d (%ds), Shake: %d (sens: %d)\n", 
                  screenTimeoutEnabled, screenTimeoutSeconds, shakeToWakeEnabled, shakeSensitivity);
}

void saveSettings() {
    preferences.begin("portalKiller", false);
    
    // Save all settings
    preferences.putBool("use24Hour", use24HourFormat);
    preferences.putInt("brightness", brightnessLevel);
    preferences.putInt("clockColor", clockColorIndex);
    preferences.putBool("timeoutOn", screenTimeoutEnabled);
    preferences.putInt("timeoutSec", screenTimeoutSeconds);
    preferences.putBool("shakeOn", shakeToWakeEnabled);
    preferences.putInt("shakeSens", shakeSensitivity);
    
    preferences.end();
    
    Serial.println("Settings saved to flash");
}
