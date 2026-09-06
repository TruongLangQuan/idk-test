#include <M5StickCPlus2.h>
#include <WiFi.h>
#include "esp_wifi.h"

// Bruce-Style Beacon Spammer for M5StickC Plus2
// Full beacon spam functionality extracted from Bruce

// Use weak symbol to avoid linker conflicts
extern "C" int __attribute__((weak)) ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    if (arg == 31337) return 1;
    else return 0;
}

// Beacon frame template (exact copy from Bruce)
uint8_t beaconPacket[109] = {
    0x80, 0x00, 0x00, 0x00,             // Type/Subtype: management beacon frame
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast  
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID
    0x00, 0x00,                         // Fragment & sequence number
    
    // Fixed parameters (timestamp + beacon interval + capability)
    0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // timestamp
    0x64, 0x00,                                       // beacon interval
    0x01, 0x04,                                       // capability info
    
    // Tagged parameters - SSID element
    0x00, 0x20,  // Element ID: SSID, Length: 32 bytes
    // SSID (32 bytes, filled dynamically)
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    
    // Supported rates element
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
    
    // DS Parameter set element  
    0x03, 0x01, 0x06
};

// Beacon Categories (Bruce style organization)
const char* governmentSSIDs[] = {
    "🕵️ NSA Mobile Monitoring 🕵️", 
    "🎯 CIA Field Office WiFi 🎯",
    "💊 DEA Surveillance Node 💊",
    "🚔 Police_Van_#3 🚔", 
    "📡 Mobile Command Center 📡",
    "⚔️ SWAT Team Alpha ⚔️",
    "🛡️ DHS Mobile Unit 🛡️",
    "💣 ATF Tactical Van 💣",
    "🕴️ Secret Service Mobile 🕴️",
    "🛂 Border Patrol Unit 🛂",
    "💰 IRS Investigation Van 💰",
    "🏛️ Homeland Security Net 🏛️",
    "⭐ Federal Marshal Unit ⭐",
    "🏦 US Treasury Mobile 🏦",
    "🛃 Customs Border WiFi 🛃",
    "🔍 FBI Evidence Unit 🔍",
    "⚖️ DOJ Investigation ⚖️",
    "✈️ TSA Security Check ✈️",
    "🧊 ICE Mobile Command 🧊",
    "🏛️ Federal Court WiFi 🏛️",
    "🏛️ Capitol Police Unit 🏛️",
    "🛡️ Pentagon Secure Net 🛡️"
};

const char* funnySSIDs[] = {
    "🤵 Abraham Linksys 🤵",
    "🛩️ Pretty Fly for a WiFi 🛩️", 
    "👶 Hide Your Kids, Hide Your WiFi 👶",
    "🥋 Wu Tang LAN 🥋",
    "😏 Router? I Barely Know Here! 😏",
    "🦕 The LAN Before Time 🦕",
    "🐑 Silence of the LANs 🐑",
    "🏰 House LANnister 🏰",
    "🎵 This LAN Is My LAN 🎵",
    "🚫 No More Mister WiFi 🚫",
    "👨‍🚀 LAN Solo 👨‍🚀", 
    "🚶‍♂️ Get Off My LAN 🚶‍♂️",
    "🔬 Bill Wi The Science Fi 🔬",
    "🌐 I Am The Internet 🌐",
    "🔥 Drop It Like Its Hotspot 🔥",
    "👑 Martin Router King 👑",
    "❄️ Winternet Is Coming ❄️",
    "💕 Tell My WiFi Love Her 💕",
    "💔 It Hurts When IP 💔",
    "🌮 Nacho WiFi 🌮",
    "👩‍💻 Mom Use This One 👩‍💻"
};

const char* suspiciousSSIDs[] = {
    "🦠 Virus Distribution Center 🦠",
    "🆓 Free WiFi (Totally Safe) 🆓",
    "🍯 Definitely Not A Honeypot 🍯",
    "⚠️ Your Network Is Compromised ⚠️", 
    "🔑 Password Is 123456 🔑",
    "😇 Totally_Not_Malware 😇",
    "👆 Click Here For Viruses 👆",
    "🏴‍☠️ Hacker's Paradise 🏴‍☠️",
    "📊 Data Harvesting Station 📊",
    "💳 Identity Theft Network 💳",
    "🚪 Backdoor Access Point 🚪",
    "🕹️ Malware Command Center 🕹️",
    "🎣 Phishing Network Hub 🎣",
    "💰 Ransomware Distribution 💰",
    "🐴 Trojan Horse WiFi 🐴",
    "👁️ Spyware Collection Point 👁️",
    "🤖 Botnet Control Node 🤖",
    "⌨️ Keylogger Network ⌨️",
    "🔐 Password Stealer WiFi 🔐",
    "💳 Credit Card Skimmer 💳",
    "🆔 Social Security Harvester 🆔",
    "🏦 Bank Account Drainer 🏦"
};

const char* techSSIDs[] = {
    "Loading...",
    "404 Network Not Found",
    "CONNECTING...",
    "Access Denied",
    "Connection Timeout",
    "Network Error 404",
    "SSID Not Found",
    "Please Wait...",
    "Buffering...",
    "System Overload",
    "Fatal Error",
    "Blue Screen WiFi",
    "Kernel Panic",
    "Segmentation Fault",
    "Memory Leak Detected",
    "Stack Overflow",
    "Null Pointer Exception",
    "Runtime Error",
    "Compilation Failed",
    "Syntax Error",
    "Database Connection Lost",
    "Server Not Responding"
};

const char* entertainmentSSIDs[] = {
    "Stark Industries WiFi",
    "Wayne Manor Network",
    "Umbrella Corporation",
    "Cyberdyne Systems",
    "Black Mesa Research",
    "Aperture Science Labs",
    "SHIELD Secure Network",
    "Oscorp Tower WiFi",
    "LexCorp Mobile",
    "Daily Planet WiFi",
    "Stark Tower Network",
    "Avengers Assembly",
    "X-Men School WiFi",
    "Gotham City Network",
    "Metropolis WiFi",
    "S.T.A.R. Labs",
    "Fortress of Solitude",
    "Hall of Justice",
    "Xavier Institute",
    "Baxter Building WiFi",
    "Fantastic Four HQ",
    "Doom Castle Network"
};

const char* offensiveSSIDs[] = {
    "YellAtYourKidsNotMe",
    "Your WiFi Sucks",
    "Stop Using My WiFi",
    "Password Is Go F Yourself",
    "I Hate My Neighbors",
    "Get Your Own Internet",
    "Router Of Holding",
    "WiFi So Hard",
    "Bandwidth Thief Detected",
    "Mom Click Here For WiFi",
    "Stop Stealing My WiFi",
    "Pay For Your Own Internet",
    "Neighbor WiFi Thief",
    "Buy Your Own Router",
    "WiFi Leech Detected",
    "Get A Job Buy WiFi",
    "Stop Being Cheap",
    "WiFi Moocher Alert",
    "Freeloading Neighbor",
    "Password Changed Again",
    "Nice Try Neighbor",
    "Keep Out Cheapskate"
};

const char* corporateSSIDs[] = {
    "Initech WiFi Network",
    "Dunder Mifflin WiFi",
    "Stark Enterprises",
    "Globodyne Corporation", 
    "Acme Corporation",
    "Massive Dynamic",
    "Weyland Industries",
    "Tyrell Corporation",
    "Buy N Large WiFi",
    "Soylent Corp Network",
    "Aperture Science",
    "Black Mesa Corp",
    "OCP Corporation",
    "Mishima Zaibatsu",
    "Shinra Electric",
    "Umbrella Corp WiFi",
    "InGen Corporation",
    "Cyberdyne Systems",
    "Oscorp Industries",
    "LexCorp Enterprises",
    "Queen Industries",
    "Palmer Technologies"
};

const char* gamerSSIDs[] = {
    "🏆 Achievement Unlocked 🏆",
    "🎮 Press Start To Connect 🎮",
    "💀 Game Over Man Game Over 💀",
    "🎯 All Your Base 🎯",
    "🍰 The Cake Is A LIE 🍰",
    "🥇 Xbox Live Gold Required 🥇",
    "🎮 PlayerOne Has Joined 🎮",
    "🔄 Respawn Point Alpha 🔄",
    "👹 Final Boss WiFi 👹",
    "🪙 Insert Coin To Continue 🪙",
    "⚔️ Critical Hit Network ⚔️",
    "🗡️ Legendary Loot WiFi 🗡️",
    "💥 Noob Tube Network 💥",
    "🎯 Headshot WiFi 🎯",
    "💥 Epic Fail Network 💥",
    "🏴‍☠️ Pwned Network 🏴‍☠️",
    "⏰ Lag Spike Central ⏰",
    "⛺ Camping Spot WiFi ⛺",
    "💀 Spawn Kill Network 💀",
    "😡 Rage Quit WiFi 😡",
    "🏆 MLG Pro Network 🏆",
    "🎮 GG Easy WiFi 🎮"
};

const char* foodSSIDs[] = {
    "Pizza Palace WiFi",
    "Taco Bell Supreme",
    "McDonalds WiFi",
    "Starbucks Guest",
    "KFC Secret Recipe",
    "Subway Sandwich WiFi",
    "Burger King Crown",
    "Dominos Delivery",
    "Dennys Open 24/7",
    "Waffle House Emergency",
    "In-N-Out Secret Menu",
    "Chipotle Extra Guac",
    "Five Guys Burgers",
    "Wendy's Fresh Never Frozen",
    "Chick-fil-A My Pleasure",
    "Popeyes Chicken Sandwich",
    "Arby's We Have The Meats",
    "White Castle Sliders",
    "Krispy Kreme Hot Now",
    "Dunkin Donuts WiFi",
    "Tim Hortons Eh",
    "IHOP Pancake Stack"
};

const char* adultSSIDs[] = {
    "Girls Gone WiFi",
    "Pretty Fly For A WiFi", 
    "No Clothes Required",
    "Adults Only Network",
    "After Dark WiFi",
    "XXX Entertainment",
    "Red Light District",
    "Mature Content Warning",
    "18+ Verification Required",
    "Parental Controls Off",
    "Adults Only Lounge",
    "Mature Audiences Only",
    "NSFW Network",
    "Adult Swim WiFi",
    "R Rated Network",
    "X Rated Connection",
    "Adults Only Club",
    "Mature Content Zone",
    "18+ Only WiFi",
    "Adults Only Area",
    "Restricted Access",
    "Age Verification Required"
};

const char* hackingSSIDs[] = {
    "Anonymous Network",
    "Hackerman WiFi",
    "Elite Hacker Club",
    "Zero Day Exploit",
    "Metasploit Framework",
    "Kali Linux Network",
    "Penetration Testing",
    "Social Engineering",
    "SQL Injection Here",
    "Buffer Overflow Net",
    "BlackHat Convention",
    "DefCon Official",
    "2600 Meeting Point",
    "Exploit Database",
    "Packet Sniffing Here",
    "Wireshark Capture",
    "Nmap Scan Network",
    "John The Ripper",
    "Aircrack-ng Ready",
    "Burp Suite Pro",
    "OWASP Testing Lab",
    "Red Team Network"
};

// Holiday Categories
const char* christmasSSIDs[] = {
    "Santa's Workshop WiFi",
    "North Pole Network",
    "Rudolph's Red Nose",
    "Christmas Tree Farm",
    "Gingerbread House Net",
    "Mistletoe Manor",
    "Jingle Bell Rock",
    "Silent Night Network",
    "Deck The Halls WiFi",
    "Frosty's Snowman Net",
    "Candy Cane Connection",
    "Christmas Carol WiFi",
    "Elf Workshop Network",
    "Santa's Sleigh WiFi",
    "Winter Wonderland",
    "Holy Night Network",
    "Christmas Morning",
    "Reindeer Games WiFi",
    "Christmas Miracle",
    "Fa La La La LAN"
};

const char* halloweenSSIDs[] = {
    "Haunted House WiFi",
    "Pumpkin Patch Network",
    "Spooky Skeleton Net",
    "Trick Or Treat WiFi",
    "Witch's Brew Network",
    "Zombie Apocalypse",
    "Ghost Buster WiFi",
    "Vampire's Lair",
    "Frankenstein's Lab",
    "Mummy's Tomb WiFi",
    "Creepy Crawlers Net",
    "Boo! Scared You",
    "Horror Movie Marathon",
    "Monster Mash WiFi",
    "Graveyard Shift",
    "Nightmare Network",
    "Salem Witch Trial",
    "Full Moon Rising",
    "Midnight Horror",
    "Scream Factory WiFi"
};

const char* valentinesSSIDs[] = {
    "Love Is In The Air",
    "Cupid's Arrow WiFi",
    "Be My Valentine",
    "Hearts And Flowers",
    "Romeo And Juliet",
    "Sweet Heart Network",
    "XOXO Connection",
    "Love Potion WiFi",
    "Date Night Network",
    "Wedding Bells WiFi",
    "Honeymoon Suite",
    "True Love Network",
    "Kiss Me WiFi",
    "Romantic Dinner",
    "Love Birds Network",
    "Forever Yours WiFi",
    "Soul Mate Connection",
    "Heart Throb Network",
    "Love Letters WiFi",
    "Passionate Connection"
};

const char* easterSSIDs[] = {
    "Easter Bunny WiFi",
    "Chocolate Egg Hunt",
    "Spring Has Sprung",
    "Bunny Hop Network",
    "Easter Sunday WiFi",
    "Peeps Marshmallow",
    "Carrot Cake Network",
    "Jelly Bean WiFi",
    "Easter Parade",
    "Resurrection Sunday",
    "Spring Flowers",
    "Easter Basket WiFi",
    "Hop To It Network",
    "Some Bunny Loves You",
    "Easter Miracle",
    "Painted Eggs WiFi",
    "Spring Break Network",
    "Bunny Trail WiFi",
    "Easter Morning",
    "Springtime Network"
};

// Seasonal Categories
const char* summerSSIDs[] = {
    "Beach Party WiFi",
    "Summer Vacation",
    "Pool Side Network",
    "Ice Cream Truck",
    "BBQ Grill Master",
    "Sunshine Network",
    "Flip Flop WiFi",
    "Surf's Up Network",
    "Beach Umbrella WiFi",
    "Summer Breeze",
    "Sandcastle Network",
    "Watermelon WiFi",
    "Lemonade Stand",
    "Sun Tan Lotion",
    "Beach Volleyball",
    "Summer Camp WiFi",
    "Hot Dog Stand",
    "Picnic Basket",
    "Camping Adventure",
    "Fireworks Display"
};

const char* winterSSIDs[] = {
    "Winter Wonderland",
    "Snowflake Network",
    "Ice Skating Rink",
    "Hot Chocolate WiFi",
    "Cozy Fireplace",
    "Snow Day Network",
    "Frozen Pond WiFi",
    "Winter Storm",
    "Icicle Formation",
    "Snowman Building",
    "Ski Lodge WiFi",
    "Blizzard Warning",
    "Frost Bite Network",
    "Snow Angel WiFi",
    "Winter Solstice",
    "Cold Front Network",
    "Snowball Fight",
    "Cabin Fever WiFi",
    "Arctic Blast",
    "Winter Olympics"
};

const char* springSSIDs[] = {
    "Spring Cleaning",
    "Flower Garden WiFi",
    "Birds Chirping",
    "Rain Shower Network",
    "Cherry Blossom",
    "Green Grass WiFi",
    "April Showers",
    "May Flowers Network",
    "Robin Red Breast",
    "Spring Forward",
    "Tulip Garden WiFi",
    "Morning Dew",
    "Fresh Air Network",
    "Sunshine Breaks",
    "New Growth WiFi",
    "Spring Rain",
    "Baby Animals",
    "Daylight Savings",
    "Warm Weather",
    "Nature Awakens"
};

const char* fallSSIDs[] = {
    "Autumn Leaves",
    "Pumpkin Spice WiFi",
    "Apple Orchard",
    "Harvest Moon",
    "Fall Colors Network",
    "Sweater Weather",
    "Corn Maze WiFi",
    "Thanksgiving Feast",
    "Hayride Network",
    "Apple Cider WiFi",
    "Football Season",
    "Back To School",
    "Falling Leaves",
    "Crisp Air Network",
    "Acorn Collection",
    "Maple Syrup WiFi",
    "Bonfire Night",
    "Cider Mill Network",
    "Pumpkin Patch",
    "October Fest WiFi"
};

// New Categories - Aliens
const char* alienSSIDs[] = {
    "Area 51 Guest Network",
    "UFO Landing Zone WiFi",
    "Alien Abduction Center",
    "Roswell Incident WiFi",
    "ET Phone Home",
    "Greys Communication Hub",
    "Mars Base Alpha",
    "Galactic Federation Net",
    "Intergalactic WiFi",
    "Beam Me Up Scotty",
    "Close Encounters Net",
    "X-Files Investigation",
    "Flying Saucer WiFi",
    "Crop Circle Network",
    "Alien Autopsy Lab",
    "Space Probe WiFi",
    "Extraterrestrial Net",
    "UFO Sighting Report",
    "Alien Embassy WiFi",
    "Mothership Command"
};

// Symbols & Special Characters
const char* symbolSSIDs[] = {
    "★☆✦✧WiFi✧✦☆★",
    "♠♣♥♦Network♦♥♣♠",
    "▲▼◄►Net◄►▲▼",
    "●○◐◑WiFi◑◐○●",
    "♪♫♬Music♬♫♪",
    "✓✗✓✗Check✗✓✗✓",
    "←↑→↓Arrow↓→↑←",
    "◊◇◈◉WiFi◉◈◇◊",
    "※※※Warning※※※",
    "☼☽☾Sun&Moon☾☽☼",
    "▓▒░Blocks░▒▓",
    "◄◄Rewind►►",
    "♀♂Gender♂♀",
    "∞∞Infinity∞∞",
    "▲△▼▽Triangles▽▼△▲",
    "◊♦◇♢Diamonds♢◇♦◊",
    "⚡⚡Lightning⚡⚡",
    "✈✈Airplane✈✈",
    "⚠⚠Caution⚠⚠",
    "♨♨Hot Springs♨♨"
};

// Dogs
const char* dogSSIDs[] = {
    "Good Boy WiFi",
    "Woof Woof Network",
    "Doggo Internet",
    "Puppers Paradise",
    "Bork Bork WiFi",
    "Golden Retriever Net",
    "Labrador Lab Network",
    "German Shepherd WiFi",
    "Bulldog Broadband",
    "Husky Howl Network",
    "Beagle Bay WiFi",
    "Poodle Paradise Net",
    "Rottweiler Router",
    "Chihuahua Connection",
    "Border Collie Net",
    "Dalmatian Dots WiFi",
    "Boxer Broadcast",
    "Dachshund Data Net",
    "Pitbull Power WiFi",
    "Corgi Castle Network",
    "Shiba Inu Internet",
    "Great Dane Data"
};

// Cats
const char* catSSIDs[] = {
    "Meow Meow Network",
    "Purr Purr WiFi",
    "Kitty Cat Connection",
    "Feline Fine Network",
    "Cat Nap WiFi",
    "Whiskers WiFi",
    "Tabby Cat Network",
    "Persian Palace WiFi",
    "Siamese Signal",
    "Maine Coon Network",
    "Ragdoll Router",
    "British Shorthair Net",
    "Russian Blue WiFi",
    "Bengal Broadband",
    "Sphynx Sphinx Net",
    "Norwegian Forest WiFi",
    "Scottish Fold Net",
    "Abyssinian Access",
    "Birman Broadband",
    "Himalayan Heights",
    "Manx Manor WiFi",
    "Oriental Shorthair"
};

// Astrology
const char* astrologySSIDs[] = {
    "Aries Rising WiFi",
    "Taurus Bull Network",
    "Gemini Twins Net",
    "Cancer Crab WiFi",
    "Leo Lion Network",
    "Virgo Virgin WiFi",
    "Libra Balance Net",
    "Scorpio Sting WiFi",
    "Sagittarius Archer",
    "Capricorn Goat Net",
    "Aquarius Water WiFi",
    "Pisces Fish Network",
    "Mercury Retrograde",
    "Venus In Love WiFi",
    "Mars Attack Network",
    "Jupiter Expansion",
    "Saturn Return WiFi",
    "Uranus Surprise Net",
    "Neptune Dreams WiFi",
    "Pluto Power Network",
    "Full Moon Energy",
    "New Moon Wishes"
};

// Astronomy
const char* astronomySSIDs[] = {
    "Milky Way Galaxy",
    "Solar System WiFi",
    "Black Hole Network",
    "Supernova Explosion",
    "Hubble Telescope Net",
    "International Space",
    "Mars Rover WiFi",
    "Saturn Ring Network",
    "Jupiter Storm WiFi",
    "Venus Hot Spot",
    "Mercury Speed Net",
    "Pluto Dwarf WiFi",
    "Asteroid Belt Net",
    "Comet Tail WiFi",
    "Neutron Star Net",
    "Quasar Signal WiFi",
    "Galaxy Cluster Net",
    "Constellation WiFi",
    "Nebula Cloud Network",
    "Pulsar Beam WiFi",
    "Big Bang Theory",
    "Dark Matter Network"
};

// Sports
const char* sportsSSIDs[] = {
    "Game Day Network",
    "Super Bowl WiFi",
    "World Cup Network",
    "Olympics 2024 WiFi",
    "NBA Championship",
    "NFL Playoffs WiFi",
    "MLB World Series",
    "NHL Stanley Cup",
    "Tennis Grand Slam",
    "Golf Masters WiFi",
    "Marathon Finish Line",
    "Boxing Ring Network",
    "MMA Octagon WiFi",
    "Formula One Racing",
    "NASCAR Speed WiFi",
    "Soccer Goal Network",
    "Basketball Court",
    "Football Stadium",
    "Baseball Diamond",
    "Hockey Rink WiFi",
    "Track And Field",
    "Swimming Pool Net"
};

// Variety / Miscellaneous
const char* varietySSIDs[] = {
    "🎲 Random WiFi Name 🎲",
    "🥯 Everything Bagel Net",
    "🎒 Mixed Bag Network",
    "🌺 Potpourri WiFi 🌺", 
    "📦 Variety Pack Net 📦",
    "🔀 Miscellaneous WiFi",
    "🧩 Odds And Ends Net",
    "🎁 Grab Bag Network 🎁",
    "📦 Surprise WiFi Box",
    "🎰 Lucky Dip Network",
    "❓ Mystery Box WiFi ❓",
    "🗂️ Catch All Network",
    "🍭 Assorted WiFi Mix",
    "⚙️ General Purpose Net",
    "🔧 All Purpose WiFi 🔧",
    "🔄 Multi Use Network",
    "🗡️ Swiss Army WiFi",
    "🃏 Jack Of All Trades",
    "📏 One Size Fits All",
    "🌐 Universal WiFi 🌐",
    "🌍 Global Network Hub",
    "🕸️ Worldwide WiFi Web"
};

// Hunter S. Thompson Quotes
const char* hunterThompsonSSIDs[] = {
    "🦇 Fear And Loathing 🦇",
    "🍺 Buy The Ticket Take Ride",
    "🎪 Circus-Circus WiFi 🎪", 
    "💊 We Had Two Bags Of Grass",
    "🏜️ Bat Country Network 🏜️",
    "🚗 Red Shark WiFi 🚗",
    "🎰 Vegas Baby Vegas 🎰",
    "🦅 American Dream WiFi 🦅",
    "⚡ Weird Vibes Network ⚡",
    "🍸 Mescaline Connection 🍸",
    "🏁 Racing Never Stops 🏁",
    "📰 Rolling Stone WiFi 📰",
    "🔫 Hell's Angels Network 🔫",
    "🌵 Desert Madness WiFi 🌵",
    "🎭 Gonzo Journalism Net 🎭",
    "💀 Death Of American Dream",
    "🏍️ Hells Angels Oakland 🏍️",
    "🍻 Rum Diary Network 🍻",
    "⚰️ Fear Death By Boredom",
    "🎪 Freak Power WiFi 🎪",
    "💥 Too Weird To Live 💥",
    "🌪️ Savage Journey WiFi 🌪️"
};

// Dead Kennedys Quotes
const char* deadKennedysSSIDs[] = {
    "🏴 Holiday In Cambodia 🏴",
    "💀 Dead Kennedys WiFi 💀",
    "🎸 California Uber Alles 🎸",
    "⚡ Nazi Punks F Off ⚡",
    "🏭 Chemical Warfare Net 🏭",
    "💣 Bleed For Me WiFi 💣",
    "🚫 Police Truck Network 🚫",
    "⭐ Kill The Poor WiFi ⭐",
    "🔥 Fresh Fruit Rotting 🔥",
    "💊 Drug Me Network 💊",
    "🎭 Plastic Surgery Disasters",
    "🌊 Surf City Network 🌊",
    "💀 Frankenchrist WiFi 💀",
    "⚡ Bedtime For Democracy ⚡",
    "🎪 Carnival Of Excess 🎪",
    "💥 Punk Rock Explosion 💥",
    "🏴‍☠️ Anarchy Burger WiFi",
    "🎵 Hardcore Punk Network 🎵",
    "💀 Jello Biafra WiFi 💀",
    "⚡ East Bay Ray Network ⚡",
    "🔊 Maximum Rocknroll 🔊",
    "🎸 Punk Not Dead WiFi 🎸"
};

// Crass Band Quotes  
const char* crassSSIDs[] = {
    "🏴 Crass Punk WiFi 🏴",
    "⚫ Feeding Of 5000 ⚫",
    "💀 Death Is Not The End 💀",
    "⚡ Anarchy Peace WiFi ⚡",
    "🔥 Stations Of Crass 🔥",
    "✊ Fight War Not Wars ✊",
    "🎭 Penis Envy Network 🎭",
    "💣 Bloody Revolutions 💣",
    "🚫 No Authority WiFi 🚫",
    "⚫ Christ The Album ⚫",
    "✊ There Is No Authority ✊",
    "🏴 Anarcho-Punk WiFi 🏴",
    "💀 Do They Owe Us Living 💀",
    "⚡ Reality Asylum WiFi ⚡",
    "🔥 Burning Britain Net 🔥",
    "✊ Fight The Power WiFi ✊",
    "🎪 Circus Of Deceit 🎪",
    "💣 Revolution Now WiFi 💣",
    "🚫 Reject Authority Net 🚫",
    "⚫ Black Flag Rising ⚫",
    "✊ Punk Rock Rebellion ✊",
    "🏴 Anarchy In The UK 🏴"
};

// Insane/Crazy Categories
const char* insaneSSIDs[] = {
    "🤪 Totally Insane WiFi 🤪",
    "😵‍💫 Madness Network 😵‍💫",
    "🌀 Dizzy Spiral WiFi 🌀",
    "💥 Mind Blown Network 💥",
    "🎭 Crazy Circus WiFi 🎭",
    "🌪️ Tornado Of Madness 🌪️",
    "🤯 Brain Explosion Net 🤯",
    "🎢 Roller Coaster WiFi 🎢",
    "🌈 Rainbow Madness Net 🌈",
    "⚡ Lightning Crazy WiFi ⚡",
    "🎪 Circus Freakshow 🎪",
    "💀 Skeleton Dance WiFi 💀",
    "🌙 Midnight Madness Net 🌙",
    "🔥 Burning Crazy WiFi 🔥",
    "⭐ Stellar Insanity Net ⭐",
    "💫 Cosmic Chaos WiFi 💫",
    "🌊 Tidal Wave Madness 🌊",
    "🎯 Target Practice Crazy 🎯",
    "🎲 Dice Roll Insanity 🎲",
    "🎊 Party Animal WiFi 🎊",
    "🎈 Balloon Pop Crazy 🎈",
    "🎆 Firework Explosion 🎆"
};

// German Language 
const char* germanSSIDs[] = {
    "🇩🇪 Deutscher WiFi 🇩🇪",
    "🍺 Oktoberfest Network 🍺",
    "🏰 Neuschwanstein WiFi 🏰",
    "🥨 Pretzel Connection 🥨",
    "⚡ Blitzkrieg Network ⚡",
    "🍻 Biergarten WiFi 🍻",
    "🚗 Autobahn Network 🚗",
    "🏭 Deutsche Qualität 🏭",
    "🎼 Beethoven WiFi 🎼",
    "⚽ Fußball Network ⚽",
    "🍖 Bratwurst WiFi 🍖",
    "🏔️ Schwarzwald Network 🏔️",
    "🎪 Karneval WiFi 🎪",
    "💎 BMW Network 💎",
    "🍺 Löwenbräu WiFi 🍺",
    "⚙️ Maschinenbau Net ⚙️",
    "🏰 Bayern München WiFi 🏰",
    "🥨 Weißwurst Network 🥨",
    "🌲 Black Forest WiFi 🌲",
    "⚡ Mercedes Network ⚡",
    "🍻 Hofbräuhaus WiFi 🍻",
    "🎵 Wagner Network 🎵"
};

// Russian Language
const char* russianSSIDs[] = {
    "🇷🇺 Russian WiFi 🇷🇺",
    "❄️ Siberian Network ❄️",
    "🏰 Red Square WiFi 🏰", 
    "🍶 Vodka Connection 🍶",
    "⚡ Soviet Network ⚡",
    "🐻 Bear Country WiFi 🐻",
    "🎭 Bolshoi Network 🎭",
    "🚀 Sputnik WiFi 🚀",
    "❄️ Moscow Winter Net ❄️",
    "⚔️ Kremlin Network ⚔️",
    "🎼 Tchaikovsky WiFi 🎼",
    "🏔️ Ural Mountains Net 🏔️",
    "⚡ Stalin Network ⚡",
    "🍖 Borscht WiFi 🍖",
    "❄️ Frozen Tundra Net ❄️",
    "🚂 Trans-Siberian WiFi 🚂",
    "⭐ Communist Star Net ⭐",
    "🎪 Russian Circus WiFi 🎪",
    "💎 Diamond Mine Network 💎",
    "⚡ KGB Surveillance ⚡",
    "🏰 St Petersburg WiFi 🏰",
    "❄️ Ice Palace Network ❄️"
};

// Pure Symbols Only
const char* pureSymbolSSIDs[] = {
    "★☆✦✧✨✩✪✫✬✭✮✯",
    "♠♣♥♦♤♧♡♢",
    "▲▼◀▶◄►▲▼",
    "●○◐◑◒◓◔◕◖◗",
    "♪♫♬♩♭♮♯𝄞",
    "✓✗✘✚✙✛✜✝✞✟",
    "←↑→↓↔↕↖↗↘↙",
    "◊◇◈◉◎●○◐◑◒",
    "※※※※※※※※",
    "☼☽☾☀☁☂☃☄",
    "▓▒░░▒▓█▉▊▋▌▍▎▏",
    "◄◄◄◄►►►►",
    "♀♂⚢⚣⚤⚥⚦⚧⚨⚩",
    "∞∝∴∵∶∷∸∹∺∻",
    "▲△▼▽◀◁▶▷",
    "◊♦◇♢◈◉◎●○",
    "⚡⚡⚡⚡⚡⚡⚡⚡",
    "✈✉✊✋✌✍✎✏✐✑",
    "⚠⚡⚢⚣⚤⚥⚦⚧⚨⚩",
    "♨♩♪♫♬♭♮♯"
};

// International Characters & Accents
const char* internationalSSIDs[] = {
    "Café WiFi Naïve Résumé",
    "Björk Mötley Crüe Ñoño",
    "Rößler München Zürich",
    "Åse Øystein Æther",
    "François Château Médoc",
    "José María Peña Señor",
    "Škoda Český Dřevo",
    "Włochy Łódź Gdańsk",
    "Москва Санкт-Петербург",
    "Ελληνικά Αθήνα Θεσσαλονίκη",
    "العربية الرياض جدة",
    "中文 北京 上海 广州",
    "日本語 東京 大阪 京都",
    "한국어 서울 부산 대구",
    "ไทย กรุงเทพฯ เชียงใหม่",
    "Tiếng Việt Hà Nội Sài Gòn",
    "हिंदी दिल्ली मुंबई",
    "Türkçe İstanbul Ankara",
    "پارسی تهران اصفهان",
    "עברית תל אביב ירושלים",
    "Português São Paulo Rio",
    "Italiano Roma Milano"
};

// Emoji Only Networks
const char* emojiOnlySSIDs[] = {
    "🎉🎊🎈🎁🎂🍰🧁",
    "🚀🛸🌟⭐✨💫🌙",
    "🦄🌈🦋🌸🌺🌻🌷",
    "🔥💥⚡🌟✨💫⭐",
    "🎮🎲🃏🎯🎪🎭🎨",
    "🍕🍔🍟🌭🥨🥯🍿",
    "🐶🐱🐭🐹🐰🦊🐻",
    "🌊🏖️🏄‍♂️🐚🐠🦈🐙",
    "❤️💛💚💙💜🖤🤍",
    "👑💎💍👸🤴👰🤵",
    "🌍🌎🌏🌐🗺️📍📌",
    "⚽🏀🏈⚾🎾🏐🏓",
    "🎵🎶🎼🎤🎧🎷🎸",
    "📱💻⌨️🖥️📺📷📹",
    "🚗🚕🚙🚌🚎🏎️🚓",
    "✈️🚁🚀🛸🚂🚇🚝",
    "🏠🏡🏢🏣🏤🏥🏦",
    "🌮🌯🥙🥗🍜🍝🍛",
    "⛰️🏔️🗻🌋🏕️⛺🏞️",
    "🎯🎪🎨🖼️🎭🎪🎢",
    "🔮🪄✨💫🌟⭐🌙",
    "🦸‍♂️🦸‍♀️🦹‍♂️🦹‍♀️🧙‍♂️🧙‍♀️🧚‍♂️"
};

// Mixed Everything - Symbols, Letters, Emojis
const char* mixedEverythingSSIDs[] = {
    "★Café★ 🌟Naïve🌟 ◊WiFi◊",
    "🚀München♦Zürich⚡",
    "♪François🎵Château♪",
    "▲José▲ 🇪🇸María🇪🇸 ▼Peña▼",
    "◈Москва◈ 💫Wifi💫 ●Net●",
    "🦄Škoda🦄 ♠Český♠ 🌈WiFi🌈",
    "⚡中文⚡ 🐉WiFi🐉 ◊网络◊",
    "♦日本語♦ 🗾Tokyo🗾 ★Net★",
    "🎌한국어🎌 ◄Seoul► 💎WiFi💎",
    "♨ไทย♨ 🇹🇭Bangkok🇹🇭 ▲Net▲",
    "🌸Việt🌸 ◉Hà Nội◉ ★WiFi★",
    "♠हिंदी♠ 🇮🇳Delhi🇮🇳 ◆Net◆",
    "🌙Türkçe🌙 ◇Istanbul◇ ▼WiFi▼",
    "♪پارسی♪ 🇮🇷Tehran🇮🇷 ●Net●",
    "✡עברית✡ 🇮🇱Tel Aviv🇮🇱 ◊WiFi◊",
    "🌊Português🌊 ◈São Paulo◈",
    "🍝Italiano🍝 ♦Roma♦ 🎭Milano🎭",
    "⚔️Русский⚔️ 🐻Moscow🐻 ❄️WiFi❄️",
    "🏰Deutsch🏰 ◉Berlin◉ 🍺WiFi🍺",
    "🥖Français🥖 ♠Paris♠ 🗼WiFi🗼",
    "💃Español💃 ◇Madrid◇ 🌞WiFi🌞",
    "🎪العربية🎪 ♦Riyadh♦ 🕌WiFi🕌"
};

// Cyberpunk/Tech Aesthetic
const char* cyberpunkSSIDs[] = {
    "◢█◣ CYBER_NEXUS ◢█◣",
    "▓▒░ MATRIX_NODE ░▒▓",
    "║▌│█║▌│█║▌│█",
    "◤◢◣◥ NEO_NET ◤◢◣◥",
    "▀▄▀▄ GHOST_SHELL ▀▄▀▄",
    "●◐○ QUANTUM_LINK ○◐●",
    "▲▼▲ NEURAL_NET ▲▼▲",
    "░▒▓ BLADE_RUNNER ▓▒░",
    "◄►◄ AKIRA_GRID ►◄►",
    "██▓ CYBER_PUNK ▓██",
    "▬▬ι═══════ﺤ",
    "┌─┐┌─┐┌─┐ HACKER ┌─┐┌─┐┌─┐",
    "░░░▒▒▒▓▓▓███",
    "◥◤◢◣ TRON_NET ◢◣◥◤",
    "├──┤ DATA_STREAM ├──┤",
    "▓█▇▆▅▄▃▂▁",
    "┠─┨ CYBER_SPACE ┠─┨",
    "♦♢◇◆ NET_RUNNER ◆◇♢♦",
    "▶◀▼▲ DIGITAL_RAIN ▲▼◀▶",
    "⚡⚡⚡ NEON_GRID ⚡⚡⚡",
    "●○◐◑ CYBER_TOKYO ◑◐○●",
    "▬▬▬► GHOST_NET ◄▬▬▬"
};

// Hellraiser Horror
const char* hellraiserSSIDs[] = {
    "⛓️ Cenobites WiFi Network ⛓️",
    "📦 Lament Configuration 📦",
    "🔧 Pinhead's Puzzle Box 🔧",
    "⛓️ Chains Of Hell Network ⛓️",
    "💀 We Have Such Sights 💀",
    "🎭 Pain And Pleasure WiFi 🎭",
    "⚫ Leviathan Network ⚫",
    "🔥 Labyrinth Of Suffering 🔥",
    "📦 Hellbound Heart WiFi 📦",
    "⛓️ Chatterer's Network ⛓️",
    "💉 Tears Are Gifts WiFi 💉",
    "🎪 Circus Of Damnation 🎪",
    "⚫ Engineer's Design ⚫",
    "🔧 Puzzle Box Portal 🔧",
    "💀 Butterball Network 💀",
    "⛓️ Female Cenobite WiFi ⛓️",
    "🔥 Hell's Geometry Network 🔥",
    "📦 Configuration Portal 📦",
    "💉 Angelique's Network 💉",
    "⚫ Channard's Hospital ⚫",
    "🎭 Doctor Hell WiFi 🎭",
    "⛓️ Torment Dimension ⛓️"
};

// Batman Universe
const char* batmanSSIDs[] = {
    "🦇 Wayne Manor WiFi 🦇",
    "🏢 Gotham City Network 🏢",
    "🚗 Batmobile Connection 🚗",
    "🦇 Dark Knight Network 🦇",
    "🃏 Joker's Wild Card WiFi 🃏",
    "❄️ Mr. Freeze Network ❄️",
    "🌿 Poison Ivy's Garden 🌿",
    "🐧 Penguin's Iceberg WiFi 🐧",
    "💀 Two-Face Coin Flip 💀",
    "🦇 Batcave Command Center 🦇",
    "🌃 Commissioner Gordon 🌃",
    "🎪 Arkham Asylum WiFi 🎪",
    "⚡ Robin Network ⚡",
    "🖤 Catwoman's Lair 🖤",
    "🏢 LexCorp Tower WiFi 🏢",
    "💎 Batman Beyond Net 💎",
    "🦅 Nightwing Network 🦅",
    "⚡ Oracle Database WiFi ⚡",
    "🌙 Batgirl Network 🌙",
    "💀 Red Hood WiFi 💀",
    "⚫ Justice League Net ⚫",
    "🦇 Gotham Knights WiFi 🦇"
};

// Zombie Apocalypse
const char* zombieSSIDs[] = {
    "🧟‍♂️ Zombie Outbreak WiFi 🧟‍♂️",
    "💀 Walking Dead Network 💀",
    "🧟‍♀️ Undead Connection 🧟‍♀️",
    "🏥 CDC Quarantine Zone 🏥",
    "⚠️ ZOMBIE ALERT WiFi ⚠️",
    "🧟‍♂️ Brain Eaters Network 🧟‍♂️",
    "💉 Infection Control WiFi 💉",
    "🏚️ Safe House Network 🏚️",
    "🔫 Zombie Hunters WiFi 🔫",
    "💀 Resident Evil Net 💀",
    "🧟‍♀️ Shambling Horde WiFi 🧟‍♀️",
    "⚠️ Biohazard Network ⚠️",
    "🏥 T-Virus Outbreak 🏥",
    "💀 Left 4 Dead WiFi 💀",
    "🧟‍♂️ Dead Rising Network 🧟‍♂️",
    "⚠️ Zombie Survival WiFi ⚠️",
    "💉 Patient Zero Network 💉",
    "🔥 Burn The Infected 🔥",
    "🏚️ Barricade Network 🏚️",
    "💀 Day Of The Dead WiFi 💀",
    "🧟‍♀️ World War Z Network 🧟‍♀️",
    "⚠️ Last Stand WiFi ⚠️"
};

// Nuclear Warning
const char* nuclearSSIDs[] = {
    "☢️ NUCLEAR WARNING ☢️",
    "⚠️ RADIATION HAZARD ⚠️",
    "☢️ Chernobyl Network ☢️",
    "⚠️ ATOMIC ENERGY WiFi ⚠️",
    "☢️ Fallout Shelter Net ☢️",
    "⚠️ MELTDOWN IMMINENT ⚠️",
    "☢️ Reactor Core WiFi ☢️",
    "⚠️ GEIGER COUNTER HIGH ⚠️",
    "☢️ Nuclear Waste Site ☢️",
    "⚠️ URANIUM ENRICHMENT ⚠️",
    "☢️ Manhattan Project ☢️",
    "⚠️ ATOMIC BOMB WiFi ⚠️",
    "☢️ Nuclear Winter Net ☢️",
    "⚠️ PLUTONIUM STORAGE ⚠️",
    "☢️ Three Mile Island ☢️",
    "⚠️ FUKUSHIMA ALERT ⚠️",
    "☢️ Nuclear Submarine ☢️",
    "⚠️ CONTAMINATION ZONE ⚠️",
    "☢️ Atomic Testing WiFi ☢️",
    "⚠️ NUCLEAR SILO NET ⚠️",
    "☢️ Radioactive Decay ☢️",
    "⚠️ CRITICAL MASS WiFi ⚠️"
};

// Mental Asylum
const char* godSSIDs[] = {
    "🙏 Divine Connection WiFi 🙏",
    "✨ Heavenly Network ✨",
    "👼 Angels Internet Café 👼",
    "⛪ Sacred Heart WiFi ⛪",
    "🕊️ Holy Spirit Network 🕊️",
    "✝️ Blessed Connection ✝️",
    "🌟 Star of Bethlehem 🌟",
    "🎺 Gabriel's Trumpet 🎺",
    "👑 Kingdom of Heaven 👑",
    "💒 Cathedral WiFi 💒",
    "🔔 Church Bells Network 🔔",
    "📿 Rosary Connection 📿",
    "🕯️ Eternal Light WiFi 🕯️",
    "🌈 Noah's Covenant Net 🌈",
    "⚡ Burning Bush WiFi ⚡",
    "📖 Gospel Network 📖",
    "🎭 Miracle Connection 🎭",
    "🌅 Resurrection WiFi 🌅",
    "🏔️ Mount Sinai Network 🏔️",
    "💫 Divine Providence 💫"
};

const char* satanSSIDs[] = {
    "😈 Infernal WiFi Network 😈",
    "🔥 Hellfire Broadband 🔥",
    "👹 Demon Spawn Internet 👹",
    "⚡ Lucifer's Connection ⚡",
    "🖤 Dark Lord Network 🖤",
    "🔱 Pitchfork WiFi 🔱",
    "💀 Skull & Brimstone 💀",
    "🌋 Lake of Fire WiFi 🌋",
    "👺 Beelzebub's Net 👺",
    "⛪ Blasphemy Connection ⛪",
    "🐍 Serpent's Network 🐍",
    "🗡️ Fallen Angel WiFi 🗡️",
    "🌑 Eclipse of Evil 🌑",
    "🦇 Bat Cave Network 🦇",
    "⚰️ Coffin WiFi ⚰️",
    "🕷️ Web of Sin 🕷️",
    "👻 Phantom Network 👻",
    "🔮 Crystal Ball WiFi 🔮",
    "🎭 Mask of Deception 🎭",
    "⚡ Wrath Connection ⚡"
};

const char* mentalAsylumSSIDs[] = {
    "🏥 Arkham Asylum WiFi 🏥",
    "💊 Psychiatric Ward Net 💊",
    "🔒 Mental Institution 🔒",
    "💉 Electroshock Therapy 💉",
    "🏥 Bedlam Hospital WiFi 🏥",
    "💊 Strait Jacket Network 💊",
    "🔒 Padded Cell WiFi 🔒",
    "💉 Lobotomy Ward Net 💉",
    "🏥 Shutter Island WiFi 🏥",
    "💊 Cuckoo's Nest Network 💊",
    "🔒 Sanatorium WiFi 🔒",
    "💉 Session 9 Network 💉",
    "🏥 Danvers State WiFi 🏥",
    "💊 Tranquilizer Network 💊",
    "🔒 Isolation Ward WiFi 🔒",
    "💉 Shock Treatment Net 💉",
    "🏥 Pennhurst Asylum 🏥",
    "💊 Antipsychotic WiFi 💊",
    "🔒 Maximum Security 🔒",
    "💉 Experimental Ward 💉",
    "🏥 Disturbed Unit WiFi 🏥",
    "💊 Psychotic Break Net 💊"
};

// Beacon category structure
struct BeaconCategory {
    const char* name;
    const char** ssids;
    int count;
};

BeaconCategory categories[] = {
    {"Government/LEO", governmentSSIDs, sizeof(governmentSSIDs)/sizeof(governmentSSIDs[0])},
    {"Funny Names", funnySSIDs, sizeof(funnySSIDs)/sizeof(funnySSIDs[0])},
    {"Suspicious", suspiciousSSIDs, sizeof(suspiciousSSIDs)/sizeof(suspiciousSSIDs[0])},
    {"Tech/Error", techSSIDs, sizeof(techSSIDs)/sizeof(techSSIDs[0])},
    {"Entertainment", entertainmentSSIDs, sizeof(entertainmentSSIDs)/sizeof(entertainmentSSIDs[0])},
    {"Offensive", offensiveSSIDs, sizeof(offensiveSSIDs)/sizeof(offensiveSSIDs[0])},
    {"Corporate", corporateSSIDs, sizeof(corporateSSIDs)/sizeof(corporateSSIDs[0])},
    {"Gaming", gamerSSIDs, sizeof(gamerSSIDs)/sizeof(gamerSSIDs[0])},
    {"Food/Restaurants", foodSSIDs, sizeof(foodSSIDs)/sizeof(foodSSIDs[0])},
    {"Adult Content", adultSSIDs, sizeof(adultSSIDs)/sizeof(adultSSIDs[0])},
    {"Hacking/Security", hackingSSIDs, sizeof(hackingSSIDs)/sizeof(hackingSSIDs[0])},
    {"Aliens & UFOs", alienSSIDs, sizeof(alienSSIDs)/sizeof(alienSSIDs[0])},
    {"Symbols & Icons", symbolSSIDs, sizeof(symbolSSIDs)/sizeof(symbolSSIDs[0])},
    {"Dogs & Puppies", dogSSIDs, sizeof(dogSSIDs)/sizeof(dogSSIDs[0])},
    {"Cats & Kittens", catSSIDs, sizeof(catSSIDs)/sizeof(catSSIDs[0])},
    {"Astrology", astrologySSIDs, sizeof(astrologySSIDs)/sizeof(astrologySSIDs[0])},
    {"Astronomy", astronomySSIDs, sizeof(astronomySSIDs)/sizeof(astronomySSIDs[0])},
    {"Sports", sportsSSIDs, sizeof(sportsSSIDs)/sizeof(sportsSSIDs[0])},
    {"Variety Mix", varietySSIDs, sizeof(varietySSIDs)/sizeof(varietySSIDs[0])},
    {"Hunter S. Thompson", hunterThompsonSSIDs, sizeof(hunterThompsonSSIDs)/sizeof(hunterThompsonSSIDs[0])},
    {"Dead Kennedys", deadKennedysSSIDs, sizeof(deadKennedysSSIDs)/sizeof(deadKennedysSSIDs[0])},
    {"Crass Punk", crassSSIDs, sizeof(crassSSIDs)/sizeof(crassSSIDs[0])},
    {"Insane & Crazy", insaneSSIDs, sizeof(insaneSSIDs)/sizeof(insaneSSIDs[0])},
    {"German Language", germanSSIDs, sizeof(germanSSIDs)/sizeof(germanSSIDs[0])},
    {"Russian Language", russianSSIDs, sizeof(russianSSIDs)/sizeof(russianSSIDs[0])},
    {"Pure Symbols", pureSymbolSSIDs, sizeof(pureSymbolSSIDs)/sizeof(pureSymbolSSIDs[0])},
    {"International", internationalSSIDs, sizeof(internationalSSIDs)/sizeof(internationalSSIDs[0])},
    {"Emoji Only", emojiOnlySSIDs, sizeof(emojiOnlySSIDs)/sizeof(emojiOnlySSIDs[0])},
    {"Mixed Everything", mixedEverythingSSIDs, sizeof(mixedEverythingSSIDs)/sizeof(mixedEverythingSSIDs[0])},
    {"Cyberpunk Tech", cyberpunkSSIDs, sizeof(cyberpunkSSIDs)/sizeof(cyberpunkSSIDs[0])},
    {"Hellraiser Horror", hellraiserSSIDs, sizeof(hellraiserSSIDs)/sizeof(hellraiserSSIDs[0])},
    {"Batman Universe", batmanSSIDs, sizeof(batmanSSIDs)/sizeof(batmanSSIDs[0])},
    {"Zombie Apocalypse", zombieSSIDs, sizeof(zombieSSIDs)/sizeof(zombieSSIDs[0])},
    {"Nuclear Warning", nuclearSSIDs, sizeof(nuclearSSIDs)/sizeof(nuclearSSIDs[0])},
    {"God & Divine", godSSIDs, sizeof(godSSIDs)/sizeof(godSSIDs[0])},
    {"Satan & Dark", satanSSIDs, sizeof(satanSSIDs)/sizeof(satanSSIDs[0])},
    {"Mental Asylum", mentalAsylumSSIDs, sizeof(mentalAsylumSSIDs)/sizeof(mentalAsylumSSIDs[0])},
    {"Christmas", christmasSSIDs, sizeof(christmasSSIDs)/sizeof(christmasSSIDs[0])},
    {"Halloween", halloweenSSIDs, sizeof(halloweenSSIDs)/sizeof(halloweenSSIDs[0])},
    {"Valentine's Day", valentinesSSIDs, sizeof(valentinesSSIDs)/sizeof(valentinesSSIDs[0])},
    {"Easter", easterSSIDs, sizeof(easterSSIDs)/sizeof(easterSSIDs[0])},
    {"Summer", summerSSIDs, sizeof(summerSSIDs)/sizeof(summerSSIDs[0])},
    {"Winter", winterSSIDs, sizeof(winterSSIDs)/sizeof(winterSSIDs[0])},
    {"Spring", springSSIDs, sizeof(springSSIDs)/sizeof(springSSIDs[0])},
    {"Fall/Autumn", fallSSIDs, sizeof(fallSSIDs)/sizeof(fallSSIDs[0])}
};

// WiFi channels to cycle through
const uint8_t channels[] = {1, 6, 11, 3, 8, 13};
int currentChannel = 0;

// Interface states
enum AppState {
    STATE_CATEGORY_MENU,
    STATE_SPAMMING  
};

AppState currentState = STATE_CATEGORY_MENU;
int selectedCategory = 0;
int currentSSIDIndex = 0; // For cycling through SSIDs in category
int totalCategories = sizeof(categories) / sizeof(categories[0]);
bool spamming = false;
unsigned long lastSpam = 0;
unsigned long beaconCount = 0;
BeaconCategory* currentCategory = nullptr;
int menuStartIndex = 0; // For scrolling menu

// Beacon display scrolling
String currentlySpamming = "";
unsigned long lastScrollUpdate = 0;
int scrollPosition = 0;
const int SCROLL_SPEED = 300; // ms between scroll steps
const int SCROLL_AREA_WIDTH = 20; // characters that fit on screen

// Function declarations
void showCategoryMenu();
void showSpamming(); 
void sendBeacon();
void spamAllBeaconsInCategory();
void handleCategoryInput();
void handleSpammingInput();
void updateScrollingBeaconDisplay();
void drawScrollingText(String text, int y, uint16_t color);

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Lcd.setRotation(0);
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setTextSize(1);
    
    // COMPLETE WiFi reset to ensure clean state
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    // Initialize WiFi in AP mode for beacon injection
    WiFi.mode(WIFI_AP);
    esp_wifi_start();
    delay(100);
    
    // Set to a clean channel
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
    
    showCategoryMenu();
}

void loop() {
    M5.update();
    
    // Update scrolling display for spamming beacons
    updateScrollingBeaconDisplay();
    
    switch (currentState) {
        case STATE_CATEGORY_MENU:
            handleCategoryInput();
            break;
        case STATE_SPAMMING:
            handleSpammingInput();
            // Handle beacon spamming - Bruce style: spam ALL beacons in category rapidly
            if (spamming && millis() - lastSpam > 50) {  // Every 50ms, spam entire category
                spamAllBeaconsInCategory();
                lastSpam = millis();
                beaconCount++;
                
                // Change channel after each full cycle through all beacons
                currentChannel++;
                if (currentChannel >= sizeof(channels) / sizeof(channels[0])) {
                    currentChannel = 0;
                }
                
                // Update display every cycle to show progress
                showSpamming();
            }
            break;
    }
    
    delay(10);
}

void handleCategoryInput() {
    // Button A (left button): Navigate up in category menu
    if (M5.BtnA.wasClicked()) {
        selectedCategory--;
        if (selectedCategory < 0) {
            selectedCategory = totalCategories - 1;
        }
        showCategoryMenu();
    }
    
    // Button B (right button): Start spamming entire selected category
    if (M5.BtnB.wasClicked()) {
        currentCategory = &categories[selectedCategory];
        currentSSIDIndex = 0;
        spamming = true;
        beaconCount = 0;
        currentState = STATE_SPAMMING;
        
        // Change to a different channel immediately to avoid conflicts
        currentChannel = random(0, sizeof(channels)/sizeof(channels[0]));
        esp_wifi_set_channel(channels[currentChannel], WIFI_SECOND_CHAN_NONE);
        
        showSpamming();
    }
    
    // Power button: Navigate down in category menu
    if (M5.BtnPWR.wasClicked()) {
        selectedCategory++;
        if (selectedCategory >= totalCategories) {
            selectedCategory = 0;
        }
        showCategoryMenu();
    }
}

void handleSpammingInput() {
    // Any button: Stop spamming and go back to category menu
    if (M5.BtnA.wasClicked() || M5.BtnB.wasClicked() || M5.BtnPWR.wasClicked()) {
        spamming = false;
        currentlySpamming = "";  // Clear scrolling text
        scrollPosition = 0;      // Reset scroll position
        currentState = STATE_CATEGORY_MENU;
        showCategoryMenu();
    }
}

void showCategoryMenu() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextSize(1);
    
    // Title
    M5.Lcd.setTextColor(0x07FF);
    M5.Lcd.drawString("Beacon Categories", 5, 5);
    
    M5.Lcd.setTextColor(0xFFE0);
    M5.Lcd.drawString("Bruce Style!", 35, 20);
    
    // Ensure selected category is visible in the window
    if (selectedCategory < menuStartIndex) {
        menuStartIndex = selectedCategory;
    } else if (selectedCategory >= menuStartIndex + 5) {
        menuStartIndex = selectedCategory - 4;
    }
    
    // Show visible categories (5 at a time for better display)
    int y = 45;
    int maxVisible = min(5, totalCategories - menuStartIndex);
    
    for (int i = 0; i < maxVisible; i++) {
        int categoryIndex = menuStartIndex + i;
        
        if (categoryIndex == selectedCategory) {
            // Highlight selected item with bigger highlight box
            M5.Lcd.fillRect(0, y-3, 135, 24, 0x0410);
            M5.Lcd.setTextColor(0xFFFF);
            M5.Lcd.drawString(">", 2, y);
        } else {
            M5.Lcd.setTextColor(0xC618);
        }
        
        // Truncate long category names better
        String catName = String(categories[categoryIndex].name);
        if (catName.length() > 14) {
            catName = catName.substring(0, 11) + "...";
        }
        
        M5.Lcd.drawString(catName, 12, y);
        M5.Lcd.setTextColor(0x8410);
        M5.Lcd.drawString("(" + String(categories[categoryIndex].count) + ")", 95, y);
        y += 26;  // More spacing between items
    }
    
    // Show scroll indicators
    if (menuStartIndex > 0) {
        M5.Lcd.setTextColor(0xFFE0);
        M5.Lcd.drawString("^ More above", 30, 30);
    }
    if (menuStartIndex + 5 < totalCategories) {
        M5.Lcd.setTextColor(0xFFE0);
        M5.Lcd.drawString("v More below", 30, 175);
    }
    
    // Show current selection info
    M5.Lcd.setTextColor(0x07E0);
    M5.Lcd.drawString("Selected: " + String(selectedCategory + 1) + "/" + String(totalCategories), 5, 190);
    
    // Button instructions
    M5.Lcd.setTextColor(0x07E0);
    M5.Lcd.drawString("A:Up PWR:Down B:SPAM!", 5, 205);
    
    // Show scrolling beacon display at bottom if spamming
    if (spamming && !currentlySpamming.isEmpty()) {
        drawScrollingText("SPAMMING: " + currentlySpamming, 220, 0xF800);
    }
}

void showSpamming() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextSize(1);
    
    M5.Lcd.setTextColor(0xF800);
    M5.Lcd.drawString("FULL CATEGORY SPAM!", 5, 10);
    
    // Show category being spammed
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString("Spamming Category:", 5, 30);
    
    M5.Lcd.setTextColor(0x07FF);
    M5.Lcd.drawString(currentCategory->name, 5, 45);
    
    // Show current SSID being transmitted (last one in cycle)
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString("Last SSID:", 5, 65);
    
    String ssid = String(currentCategory->ssids[currentSSIDIndex]);
    if (ssid.length() > 18) {
        M5.Lcd.setTextColor(0x07E0);
        M5.Lcd.drawString(ssid.substring(0, 18), 5, 80);
        if (ssid.length() > 18) {
            M5.Lcd.drawString(ssid.substring(18), 5, 95);
        }
    } else {
        M5.Lcd.setTextColor(0x07E0);
        M5.Lcd.drawString(ssid, 5, 80);
    }
    
    // Show spam stats
    M5.Lcd.setTextColor(0xFFE0);
    M5.Lcd.drawString("Full Cycles: " + String(beaconCount), 5, 110);
    
    M5.Lcd.setTextColor(0xF81F);
    M5.Lcd.drawString("Total SSIDs: " + String(currentCategory->count), 5, 125);
    
    M5.Lcd.setTextColor(0xF81F);
    M5.Lcd.drawString("Channel: " + String(channels[currentChannel]), 5, 140);
    
    M5.Lcd.setTextColor(0x07FF);
    M5.Lcd.drawString("Rate: Bruce Style Fast!", 5, 155);
    
    // Instructions
    M5.Lcd.setTextColor(0x8410);
    M5.Lcd.drawString("Any Button: Stop", 25, 180);
    
    // Show scrolling beacon display at bottom
    drawScrollingText("LIVE: " + currentlySpamming, 200, 0xF800);
}

void spamAllBeaconsInCategory() {
    // Ensure we have a valid category
    if (!currentCategory) {
        return;
    }
    
    // Set current channel
    esp_wifi_set_channel(channels[currentChannel], WIFI_SECOND_CHAN_NONE);
    
    // Bruce-style: Rapidly send ALL beacons in the category
    for (int ssidIndex = 0; ssidIndex < currentCategory->count; ssidIndex++) {
        String ssid = String(currentCategory->ssids[ssidIndex]);
        
        // Create a fresh beacon packet for each SSID
        uint8_t freshBeacon[109];
        memcpy(freshBeacon, beaconPacket, sizeof(beaconPacket));
        
        // Fill SSID into fresh beacon packet
        int ssidLen = min(32, (int)ssid.length());
        
        // COMPLETELY clear SSID area first - fill with spaces (0x20)
        memset(&freshBeacon[38], 0x20, 32);
        
        // Copy SSID characters directly 
        for (int i = 0; i < ssidLen; i++) {
            freshBeacon[38 + i] = ssid.c_str()[i];
        }
        
        // Set SSID length in the packet (CRITICAL!)
        freshBeacon[37] = (uint8_t)ssidLen;
        
        // Generate completely random MAC addresses for each beacon
        uint32_t macSeed = millis() + ssidIndex + random(1000);
        randomSeed(macSeed);
        
        for (int i = 10; i < 16; i++) {
            freshBeacon[i] = random(256);        // Source MAC
            freshBeacon[i + 6] = freshBeacon[i]; // Copy to BSSID
        }
        
        // Make MAC locally administered and unicast
        freshBeacon[10] = (freshBeacon[10] & 0xFC) | 0x02;
        freshBeacon[16] = (freshBeacon[16] & 0xFC) | 0x02;
        
        // Update timestamp to current time
        uint64_t timestamp = micros();
        memcpy(&freshBeacon[24], &timestamp, 8);
        
        // Update DS parameter with current channel
        freshBeacon[108] = channels[currentChannel];
        
        // Send the beacon with multiple bursts like Bruce (3 times with 1ms delay)
        for (int burst = 0; burst < 3; burst++) {
            esp_wifi_80211_tx(WIFI_IF_AP, freshBeacon, sizeof(freshBeacon), false);
            delayMicroseconds(1000); // 1ms delay like Bruce
        }
        
        // Update current SSID index for display and scrolling text
        currentSSIDIndex = ssidIndex;
        currentlySpamming = ssid;  // Update the scrolling display text
        
        // Small delay between SSIDs (like Bruce)
        delayMicroseconds(100);
    }
}

void sendBeacon() {
    // Ensure we have a valid category and SSID index
    if (!currentCategory || currentSSIDIndex >= currentCategory->count) {
        return;
    }
    
    String ssid = String(currentCategory->ssids[currentSSIDIndex]);
    
    // Create a fresh beacon packet for each transmission
    uint8_t freshBeacon[109];
    memcpy(freshBeacon, beaconPacket, sizeof(beaconPacket));
    
    // Fill SSID into fresh beacon packet
    int ssidLen = min(32, (int)ssid.length());
    
    // COMPLETELY clear SSID area first - fill with spaces (0x20)
    memset(&freshBeacon[38], 0x20, 32);
    
    // Copy SSID characters directly 
    for (int i = 0; i < ssidLen; i++) {
        freshBeacon[38 + i] = ssid.c_str()[i];
    }
    
    // Set SSID length in the packet (CRITICAL!)
    freshBeacon[37] = (uint8_t)ssidLen;
    
    // Generate completely random MAC addresses for each beacon
    uint32_t macSeed = millis() + currentSSIDIndex + random(1000);
    randomSeed(macSeed);
    
    for (int i = 10; i < 16; i++) {
        freshBeacon[i] = random(256);        // Source MAC
        freshBeacon[i + 6] = freshBeacon[i]; // Copy to BSSID
    }
    
    // Make MAC locally administered and unicast
    freshBeacon[10] = (freshBeacon[10] & 0xFC) | 0x02;
    freshBeacon[16] = (freshBeacon[16] & 0xFC) | 0x02;
    
    // Update timestamp to current time
    uint64_t timestamp = micros();
    memcpy(&freshBeacon[24], &timestamp, 8);
    
    // Set current channel and update DS parameter
    esp_wifi_set_channel(channels[currentChannel], WIFI_SECOND_CHAN_NONE);
    freshBeacon[108] = channels[currentChannel];
    
    // Send the beacon with multiple bursts for better visibility
    for (int burst = 0; burst < 5; burst++) {
        esp_wifi_80211_tx(WIFI_IF_AP, freshBeacon, sizeof(freshBeacon), false);
        delayMicroseconds(100);
    }
}

// Update scrolling beacon display
void updateScrollingBeaconDisplay() {
    if (millis() - lastScrollUpdate > SCROLL_SPEED) {
        scrollPosition++;
        lastScrollUpdate = millis();
    }
}

// Draw scrolling text at bottom of screen
void drawScrollingText(String text, int y, uint16_t color) {
    if (text.isEmpty()) return;
    
    // Clear the line first
    M5.Lcd.fillRect(0, y, 135, 16, 0x0000);
    
    // If text fits on screen, just display it normally
    if (text.length() <= SCROLL_AREA_WIDTH) {
        M5.Lcd.setTextColor(color);
        M5.Lcd.drawString(text, 5, y);
        return;
    }
    
    // Create scrolling effect by cycling through the text
    String displayText = text + "    "; // Add spacing between loops
    int totalLength = displayText.length();
    
    // Calculate scroll position within the text
    int startPos = scrollPosition % totalLength;
    
    // Build the display string
    String scrollingDisplay = "";
    for (int i = 0; i < SCROLL_AREA_WIDTH && i < totalLength; i++) {
        int charIndex = (startPos + i) % totalLength;
        scrollingDisplay += displayText.charAt(charIndex);
    }
    
    M5.Lcd.setTextColor(color);
    M5.Lcd.drawString(scrollingDisplay, 5, y);
}