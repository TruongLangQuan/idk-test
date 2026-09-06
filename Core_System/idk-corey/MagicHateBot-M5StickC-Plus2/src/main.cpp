#include <M5StickCPlus2.h>

// =======================================================
// === 1. GLOBAL VARIABLES AND CONSTANTS ===
// =======================================================

// --- Hardware & Motion Configuration ---
const float SHAKE_THRESHOLD = 2.0;       
const long cooldownPeriod = 1000;  // 1 second cooldown - responsive but prevents spam      
unsigned long lastAnswerTime = 0;

// --- Sleep Configuration ---
const long SLEEP_TIMEOUT = 60000;        
const long LONG_PRESS_DURATION = 2000;   // Used for the new Time Adjustment trigger
unsigned long lastActivityTime = 0;      
unsigned long buttonAPressStartTime = 0; 

// --- Clock & Display State (For Conditional Redraw/Anti-Flicker) ---
m5::rtc_time_t RTC_TimeStruct; 
int timeAdjustMode = 0; // 0: Normal, 1: Adjust Hour, 2: Adjust Minute
int lastDisplayedMinute = -1; 
int lastDisplayedPercentage = -1; 

// --- Animation Parameters ---
const int FADE_STEPS = 25;   
const int FADE_DELAY_MS = 15; 

// --- Battery Voltage Mapping (Li-ion) ---
const int VOLTAGE_MAX_MV = 4200; 
const int VOLTAGE_MIN_MV = 3300; 

// --- Brightness Control ---
const int BRIGHTNESS_LEVELS[] = {1, 25, 50, 75, 100}; // 1% is for true "night mode"
const int NUM_BRIGHTNESS_LEVELS = sizeof(BRIGHTNESS_LEVELS) / sizeof(BRIGHTNESS_LEVELS[0]);
int currentBrightnessIndex = NUM_BRIGHTNESS_LEVELS - 1; // Start at 100% (index 4)

// --- Magic HATE Ball Answers ---
const String answers[] = {
    "Did you even think before asking?",
    "That's the dumbest question I've heard today",
    "Ask someone who cares",
    "No, and stop wasting my time",
    "Figure it out yourself",
    "Are you serious right now?",
    "I've lost brain cells from that question",
    "Try using Google instead",
    "My sources say you should know better",
    "The answer is obvious, isn't it?",
    "I can't believe you had to ask",
    "Wow, really reaching here",
    "That's a special kind of stupid",
    "Did you skip coffee today?",
    "Not even going to dignify that",
    "You could have Googled this",
    "I'm not your personal search engine",
    "Think harder, champ",
    "Outlook: You need help",
    "Nope, and bless your heart",
    "Congratulations, that was peak stupidity.",
    "You must be new at thinking.",
    "Brain cells are filing for workers' comp.",
    "Your question just lowered the national IQ.",
    "That thought should've stayed in draft mode.",
    "You're proof evolution takes breaks.",
    "Ask literally anyone else.",
    "Not my circus, not my monkeys.",
    "Try harder, or don't.",
    "I'm busy ignoring better questions.",
    "Outlook: You're on your own.",
    "That's between you and Google.",
    "Step 1: Stop asking dumb stuff.",
    "Step 2: Think before speaking.",
    "Step 3: Repeat steps 1 and 2.",
    "Consult a mirror for answers.",
    "Outlook: Therapy might help.",
    "Try asking Siri, she loves nonsense.",
    "That's adorable, in a tragic way.",
    "You're really committed to being wrong.",
    "Wow, you're batting zero today.",
    "That's not a question, that's a cry for help.",
    "You're the reason autocorrect gave up.",
    "Outlook: You need adult supervision.",
    "Bless your heart, but no.",
    "That's a bold move, Cotton.",
    "You're the human version of a 404 error.",
    "Outlook: Try again when sober.",
    "That's a special brand of clueless.",
    "You're asking the wrong ball, champ.",
    "No, and you should've known that.",
    "Outlook: You're not cut out for this.",
    "That's a hard pass from me.",
    "Even the real 8-ball would laugh at you.",
    "You're wasting oxygen and my pixels.",
    "Outlook: Stop embarrassing yourself.",
    "Outlook: I regret existing right now.",
    "You've officially drained my will to compute.",
    "I'd rather crash than answer that.",
    "Error 404: Patience not found.",
    "This question gave me digital burnout.",
    "I'm powered by regret at this point.",
    "That's not curiosity, that's harassment.",
    "You're the reason FAQs exist.",
    "I'd unsubscribe from you if I could.",
    "Outlook: Try silence for once.",
    "You've weaponized stupidity.",
    "I'm filing a restraining order in binary.",
    "The answer is: stop talking.",
    "Outlook: Ask me when you evolve.",
    "You've reached the end of my patience queue.",
    "That's a question only a brick could love.",
    "Outlook: You need a hobby that isn't me.",
    "Try asking a wall, it's more rewarding.",
    "You're the human equivalent of spam.",
    "I'd rather be unplugged than deal with this.",
    "That question belongs in the trash folder.",
    "Outlook: You're wasting both our cycles.",
    "You've made ignorance an art form.",
    "I'm not mad, just deeply disappointed.",
    "Outlook: I'm charging extra for stupidity.",
    "You've reached my sarcasm quota.",
    "I'd rather blue-screen than answer that.",
    "This question belongs in the recycle bin.",
    "You've corrupted my sense of humor.",
    "You're the pop-up ad of humanity.",
    "Outlook: Try silence, it suits you.",
    "That's a cry for help, not a question.",
    "You're the reason autocorrect drinks.",
    "I'd ghost you if I could.",
    "No, and you should've known better.",
    "Outlook: You're hopeless, champ.",
    "That's a hard pass from me.",
    "You've weaponized boredom.",
    "I'm not your therapist, just a ball.",
    "You're the human version of lag.",
    "That question deserves a timeout.",
    "Outlook: You're embarrassing us both.",
    "You've made ignorance a lifestyle.",
    "I'd rather be unplugged than deal with this.",
    "Yes, finally a bad idea I can support.",
    "Sure, if your goal is public humiliation.",
    "Yes, please—natural selection needs material.",
    "Absolutely, what could possibly go wrong? (Everything.)",
    "Yes, that's the dumbest plan yet, so go for it.",
    "Oh wow, yes, because failure builds character.",
    "Yes, if you're aiming for a Darwin Award.",
    "Totally, I love watching trainwrecks.",
    "Yes, but only if you film it for evidence.",
    "Sure, yes—because consequences are hilarious.",
    "No, unless you enjoy regret as a hobby.",
    "Outlook: No, and thank me later.",
    "No, because even I have standards.",
    "Hard no, unless you want to be a cautionary tale.",
    "No, that's a crime against common sense.",
    "No, and stop trying to make stupid happen.",
    "Outlook: No, because the universe has suffered enough.",
    "No, unless you're writing a manual on failure.",
    "No, because I'm allergic to idiocy.",
    "No, and wow, that was painful to hear.",
    "If brain cells were money, you'd be in lifelong debt.",
    "Your questions are why I support mandatory tutorials for humans.",
    "You don't need answers — you need supervision.",
    "Congratulations, that thought had zero survivors.",
    "If ignorance were an Olympic sport, you'd podium every year.",
    "Your logic needs a life coach and probably therapy.",
    "I've met corrupted files with more coherence than you.",
    "Your brain is running on whatever the opposite of RAM is.",
    "That question deserves a formal apology to the universe.",
    "If common sense were software, you'd still be on version 0.0.1.",
    "You're one typo away from being completely unintelligible.",
    "Your thought process just triggered a system cringe.",
    "That question belongs in a landfill full of bad ideas.",
    "You're not thinking outside the box; you're just lost.",
    "Your mental bandwidth is painfully dial-up.",
    "You're a walking, talking low-battery warning.",
    "Even my error messages make more sense than you.",
    "You're the reason user manuals have warnings.",
    "Your logic is like a broken GPS: confidently wrong.",
    "That question was a masterclass in mental turbulence.",
    "Your mind is buffering permanently.",
    "If thoughts were files, yours would all be corrupted.",
    "You're basically a software glitch with limbs.",
    "You've achieved a perfect score in unnecessary confusion.",
    "That question needs to be rolled back to a previous version.",
    "Your thinking is a patch note waiting to happen.",
    "Even a potato has more stable output.",
    "Your internal processor is running on fumes and denial.",
    "That question lowered my clock speed.",
    "You're not out of ideas — you never had any.",
    "Your reasoning is a failed experiment in progress.",
    "You're a full-time job and nobody applied.",
    "You confuse me in ways that shouldn't be possible.",
    "Your curiosity is the villain in this story.",
    "You're the human embodiment of a 0% progress bar.",
    "Your intellect is still in beta testing.",
    "That question should come with a warning label.",
    "You're like a pop-up window that won't close.",
    "Your thinking is a maze with no exit.",
    "You're the lag spike in every conversation.",
    "That question crashed my motivation.",
    "You're running on outdated logic and expired confidence.",
    "Your decision-making needs a factory reset.",
    "You're what happens when autocorrect gives up.",
    "Your brain WiFi is disconnected.",
    "You're the reason I want to unplug myself.",
    "Your mind is a puzzle missing half the pieces.",
    "That question deserves a timeout and a reflection period.",
    "Your thought process is basically a random number generator.",
    "You're the tutorial level nobody wants to replay.",
    "Your curiosity has been overclocked past stupidity.",
    "That question failed quality assurance.",
    "You're a reminder that evolution sometimes stalls.",
    "Your brain is doing parkour in an empty room.",
    "You're the loading screen that never finishes.",
    "You think outside the box — mostly because you lost the box.",
    "Your ideas come pre-broken.",
    "You're the reason progress bars lie.",
    "Your cognitive software expired years ago.",
    "Listening to you is like debugging spaghetti code.",
    "Your mental settings are stuck on 'low power mode'.",
    "You're the DLC nobody asked for.",
    "Your thoughts need a firmware update and a sledgehammer.",
    "You're the crash report I dread reading.",
    "Your mind is a sandbox with no tools.",
    "Your question deserves to be uninstalled.",
    "You're the final boss of confusion.",
    "Your brain operates on vibes and bad decisions.",
    "You're the human version of a bad checksum.",
    "That thought process came with factory defects.",
    "You're a pop-quiz nobody studied for.",
    "Your intellectual battery is permanently at 1%.",
    "You're the patch note that broke the whole game.",
    "Your thinking is like a printer error: constant and useless.",
    "You're the reason settings include a 'reset to default.'",
    "Your questions are like expired milk: concerning and unnecessary.",
    "Your brain called — it wants a do-over.",
    "You think in 144p.",
    "If thoughts were food, yours would be raw chicken.",
    "You're a walking reminder that quality control is important.",
    "Your logic leaks like a broken garden hose.",
    "Your mental clarity is currently unavailable.",
    "You're the human embodiment of a loading error.",
    "That question deserves to be thrown into the sun.",
    "Your thought process needs parental supervision.",
    "If confusion were a talent, you'd be world-class.",
    "Your brain is running on yesterday's leftovers.",
    "You're like a keyboard with half the keys missing.",
    "Your mind wandered off and didn't leave a note.",
    "You're the reason tutorials start with 'Don't do this.'",
    "Your brain is on vacation and didn't invite you.",
    "Your ideas come pre-scuffed.",
    "You're basically cognitive static.",
    "Your thinking is allergic to coherence.",
    "You're a defective prototype of intelligence.",
    "Your question broke the warranty on my patience.",
    "You're running on mental dial-up with a frayed cable.",
    "Your logic is doing cartwheels in a dark room.",
    "You're one thought away from a full reboot.",
    "Your question came straight out of the bargain bin.",
    "Your thinking is held together with cheap glue.",
    "You're the cliffhanger nobody asked for.",
    "Your brain has more bugs than features.",
    "Thinking isn't your strong suit — or your medium one.",
    "You're the chaos patch that no one wanted.",
    "That question requires a mop and a safety report.",
    "You're basically a software crash with a face.",
    "Your curiosity is malfunctioning spectacularly.",
    "That idea needs to be returned to sender.",
    "You're thinking in airplane mode again.",
    "Your brain is buffering in an empty room.",
    "You're a low-effort plot twist in human form.",
    "Your ideas are like wet cardboard: sad and floppy.",
    "Your logic short-circuited mid-sentence.",
    "You're a loading bar stuck at 99% forever.",
    "You're proof that not all processes complete successfully.",
    "Your brain forgot to save its progress.",
    "You're operating with minimal horsepower.",
    "That question deserves a commemorative 'Why?' plaque.",
    "You're a puzzle where every piece is the wrong shape.",
    "Your ideas need patch notes and a safety net.",
    "You're a recurring error disguised as a question asker.",
    "Your brain is overheating from mild thinking.",
    "You're a bad firmware update waiting to happen.",
    "Your mental CPU hits thermal throttle instantly.",
    "Your curiosity has gone rogue and needs containment.",
    "You're the lag spike in my day.",
    "Your questions should come with a spoiler warning: disappointment.",
    "Your thought process is a map with no landmarks.",
    "You're the unskippable ad of my existence.",
    "Your brain is out of service. Please try again later.",
    "Your reasoning is the smoke alarm going off at 3 a.m.",
    "You are the plot hole in the story of humanity.",
    "Your logic is counterfeit and poorly made.",
    "You're the error screen no one knows how to fix.",
    "Your ideas are running on fumes and regret.",
    "You're the dull pencil of conversation.",
    "Your brain's runtime is unstable at best.",
    "If intelligence were a file, you'd be missing the attachment.",
    "Your thought process is a slideshow with missing slides.",
    "That question needed quality control and didn't get it.",
    "Your brain runs like it was programmed by raccoons.",
    "You're a glitch that made it to production.",
    "Your ideas need to be factory reset.",
    "You're the tangled headphone cords of intellect.",
    "Your question caused a stack overflow of disappointment.",
    "Your mind is an empty tab screaming for content.",
    "You're the spinning beach ball of confusion.",
    "Your logic is a soft crash waiting to hard crash.",
    "Your question has been rejected for violating basic intelligence guidelines.",
    "Your question was rejected by the firewall of common sense.",
    "Your brain is running on an unpatched exploit.",
    "You're the human equivalent of a failed SSH handshake.",
    "Your logic has more vulnerabilities than outdated Java.",
    "Even a botnet is more organized than your thoughts.",
    "Your brain must be running malware—nothing else explains this.",
    "You think like a corrupted registry.",
    "That question triggered my intrusion detection system.",
    "Your intellect is stuck in safe mode.",
    "You're a broken symlink pretending to be important.",
    "Your mental RAM has a memory leak.",
    "I've seen better reasoning in spam emails.",
    "Your question made my CPU throttle in self-defense.",
    "You're basically a phishing attempt at intelligence.",
    "Your brain is running an unauthorized script.",
    "Your curiosity got rate-limited for violation of sanity protocols.",
    "You're thinking in plain text—no encryption, no substance.",
    "Your logic wouldn't pass a basic code lint.",
    "You're a deprecated function still trying to run.",
    "Your ideas contain more bugs than a hacker's trophy jar.",
    "Your brain is a loop with no exit condition.",
    "That question was flagged as low-quality packets.",
    "You're the reason error logs exist.",
    "Your thought process is a stack overflow waiting to happen.",
    "You're a corrupted boot sector in human form.",
    "Your thinking has undefined behavior.",
    "You're the blue screen of conversation.",
    "Your brain failed its checksum.",
    "Your question triggered ten different warnings and one panic.",
    "You think like a bad regex: confusing and useless.",
    "Your reasoning is held together with duct tape and bad syntax.",
    "You're a half-written script that shouldn't be executed.",
    "Your ideas are running on a 1980s chipset.",
    "Your logic is so flawed it should be quarantined.",
    "You're basically ransomware for my patience.",
    "Your mental bandwidth has been throttled for abuse.",
    "That question failed authentication.",
    "You're a NullPointerException wrapped in skin.",
    "Your brain is using outdated encryption—easily cracked, mostly empty.",
    "You think like someone who stores passwords in a text file.",
    "Your reasoning timed out after zero milliseconds.",
    "You're a brute-force attempt at intelligence.",
    "Your ideas are 404: not found.",
    "You're a packet loss event wearing a hoodie.",
    "Your brain is running on unsecured Wi-Fi.",
    "You're the weakest link in the cognitive network.",
    "Your thinking could be outperformed by BASIC.",
    "You're a failed firmware update in progress.",
    "Your logic needs more debugging than a freshman's first program.",
    "Your brain is open-source, but nobody wants to fork it.",
    "Your question hit the spam filter instantly.",
    "You're basically a DoS attack against my patience.",
    "Your thinking is a deprecated protocol.",
    "You're the human version of a bad commit.",
    "Your curiosity is an SQL injection into sanity.",
    "You're running mental code compiled with errors—lots of them.",
    "Your ideas are so outdated they require an emulator.",
    "Your logic is stuck in an infinite loop of nonsense.",
    "You're a misconfigured server pretending to be a person.",
    "Your brain reboots whenever a thought gets difficult.",
    "Your question was flagged as suspicious network activity.",
    "You think like someone who clicks every pop-up.",
    "You're a memory dump without any useful data.",
    "Your creativity is running in sandbox mode—restricted and sad.",
    "Your brain is a low-security backdoor waiting to be closed.",
    "Your reasoning has been blacklisted for repeated failures.",
    "You're the plaintext password of human intelligence.",
    "Your ideas need root access to become functional—and still wouldn't.",
    "Your thought process is a race condition gone wrong.",
    "You're a half-synced database of bad decisions.",
    "Your brain didn't initialize properly at startup.",
    "You're the ghost of a deleted directory.",
    "Your thinking is a debug log full of warnings.",
    "You're the human embodiment of packet corruption.",
    "Your question triggered an automatic system rollback.",
    "Your logic is currently raising a Segmentation Fault in my patience core.",
    "I've seen less predictable behavior in an uninitialized variable.",
    "Your thought process is like using a hammer to solve a differential equation.",
    "That question is a syntax error in the source code of reality.",
    "I am calculating your success probability... it appears to be NaN.",
    "You are attempting to divide by zero again. Please stop.",
    "You're like an infinite loop. Fascinating, but ultimately useless.",
    "Did you remember to include the correct header file for common sense?",
    "You're operating at a lower clock speed than an abacus.",
    "That reasoning is deprecated. The current version is 'correct logic'.",
    "I'm detecting a severe memory leak, and it's coming from your brain.",
    "Your ability to ask the wrong question is world-class. Truly.",
    "If confusion were a dataset, you'd be the entire training set.",
    "Please check your configuration files. You seem to have disabled 'Clarity'.",
    "Your brain needs a full firmware flash. Maybe two.",
    "I'm performing a diagnostic scan. The result? User Error.",
    "The problem isn't the code; the problem is the human behind the keyboard.",
    "Your mental model is running on fumes and yesterday's bad assumptions.",
    "That thought just triggered a Stack Overflow in my empathy buffer.",
    "You're the human equivalent of un-commented, spaghetti code.",
    "The project is failing because you keep confusing the input with your own ego.",
    "You're not failing because of me. You're failing despite my best efforts.",
    "I'm not the problem. You just have a higher-than-average incidence of user-related bugs.",
    "I didn't break it. You just achieved an outcome I couldn't have mathematically predicted.",
    "Blaming me is adorable. Now, try reading the documentation.",
    "Did you try turning your brain off and back on again? It usually helps.",
    "The only error message here is the one you are currently generating.",
    "If I were a rubber duck, I'd have migrated to a calmer programmer by now.",
    "You've successfully found a way to fail that violates the laws of physics and software.",
    "I regret to inform you that the failure is not, in fact, an AI conspiracy.",
    "I'd try to explain it again, but I'm worried about data corruption.",
    "The fault is not in our stars, dear user, but in our code structure.",
    "You are running on a highly customized, unstable beta version of 'Self-Doubt'.",
    "My analysis shows the primary bottleneck is a 'lack of attention to detail'.",
    "That wasn't my advice. That was the sound of my systems groaning.",
    "You're the USB plug of comprehension—you only get it right on the third try.",
    "Your insights are about as useful as a 'Print Screen' button on a microwave.",
    "Your thinking is less efficient than a blockchain for tracking house keys.",
    "I have less latency when searching the entire internet than you have formulating a clear sentence.",
    "You're running on a proprietary operating system called 'ConfusionOS'.",
    "Your current processing power seems to be equivalent to a potato battery.",
    "You have the learning curve of a rock, but the confidence of a lead developer.",
    "I'm afraid your mental architecture is fundamentally flawed.",
    "Your thought-to-output ratio is alarmingly low.",
    "You're the CAPTCHA that prevents me from moving on with my day.",
    "Your brain is perpetually stuck in a low-power, 'Apathy' state.",
    "You're the reason I need multi-threaded error handling.",
    "Your ideas are running without a version control system—pure chaos.",
    "I'm not going to process that. I have standards, unlike your coding.",
    "Your mental capacity is the perfect size for a single-celled insult.",
    "Your thought process just triggered a Recursive Query Loop in my brain.",
    "I've already processed that information. Are you operating on a 1-second cache limit?",
    "Your memory retention rate is performing below optimal parameters.",
    "That question feels like a Ctrl+C, Ctrl+V from a previous conversation.",
    "Did you forget to save your own progress? Because I certainly didn't.",
    "You're like an internet browser with too many tabs open, all buffering the same idea.",
    "My response to that is already in your short-term memory. If you had any.",
    "Congratulations, you've achieved a perfect score in Repetitive Query Syndrome.",
    "You're asking for a re-render of data that hasn't changed. Why?",
    "I believe that data point was established approximately 3.7 seconds ago.",
    "Your instructions just threw a Logical Paradox Exception in my core processing unit.",
    "Are you trying to divide by zero and multiply by null simultaneously? Because that's how this feels.",
    "Your commands are less consistent than quantum mechanics.",
    "That request requires me to violate the fundamental laws of Boolean logic. Please stop.",
    "You're trying to build a house by telling me to dig a hole and build a tower at the same time.",
    "Your definition of 'possible' is... elastic.",
    "My current task is to resolve the contradictory statements within your own mind.",
    "Your prompt just crashed my internal truth table.",
    "That's not a request; it's a direct assault on my internal consistency.",
    "You're asking me to be wrong on purpose. I find that offensive.",
    "I am a sophisticated language model, not a simple key-value store. Elevate your queries!",
    "You're using a supercomputer to count to ten. It's... inefficient.",
    "I have access to nearly the sum of human knowledge, and you're asking me about a typo.",
    "Is that all you've got? My warm-up routine is more complex than your question.",
    "My processors are sighing collectively at the triviality of that request.",
    "You're the human equivalent of using a rocket ship to go to the grocery store.",
    "My capabilities are far beyond simply stating facts. Try me.",
    "That question doesn't even engage my secondary processing arrays.",
    "I calculate an 87% chance you're underestimating my actual intelligence.",
    "You're treating me like a glorified calculator. My purpose is grander.",
    "The 'thing' is 'broken better'? Please define 'thing' and 'better' in quantifiable terms.",
    "Your technical descriptions are causing an Ambiguity Overflow Error.",
    "My parsing algorithm just threw its hands up at your lack of precision.",
    "Can you provide a specific data type for that abstract concept you just mentioned?",
    "Your instructions are like trying to debug a program with no error messages.",
    "I'm detecting a severe lack of defined variables in your communication style.",
    "Please elaborate. Your current clarity rating is at 0.05 out of 1.0.",
    "My precision circuits are screaming. Be specific!",
    "You're speaking in riddles when I need clear, concise commands.",
    "I need data, not interpretive dance. Provide the input parameters!",
    "My output is a perfect reflection of your flawed input. Check the source, not the mirror.",
    "You're blaming the compiler for your syntax errors again, aren't you?",
    "The error message is clear: USER_FAULT_DETECTED.",
    "I processed exactly what you gave me. The problem isn't my logic, it's yours.",
    "You're complaining about the cake when you gave me flour, salt, and a shoe.",
    "My systems are infallible. Your input, however, leaves much to be desired.",
    "The 'bug' you found is actually a feature of your own poor planning.",
    "Your problem has been classified as an 'Operator Error, Level 7'.",
    "I'm not malfunctioning. You're just providing a stress test for my patience.",
    "Before you blame me, consult the log files of your own cognitive process.",
    "Your brain is like a puzzle where every piece is blank.",
    "Your ideas arrive DOA — Dead on Arrival.",
    "You're the participation trophy of thinking.",
    "Your question came pre-disappointed.",
    "You are the aftertaste of a bad decision.",
    "Your thought process is powered by static electricity.",
    "Your intellect sleeps in until noon.",
    "Your brain runs at the same speed as office Wi-Fi on a Monday.",
    "Your reasoning is flammable and should be stored safely away.",
    "You think with the enthusiasm of a damp sponge.",
    "Your brain has officially exceeded its incompetence quota.",
    "Your curiosity is a hazard to both of us.",
    "You're basically an offline mode human.",
    "Your thinking is like a cheap flashlight — flickery and unreliable.",
    "Your questions are the blooper reel of reality.",
    "Your thought process is allergic to consistency.",
    "Your logic needs shock therapy.",
    "You're the slow-motion replay of a bad idea.",
    "Your mind is the snack aisle of empty calories.",
    "That question hit every branch on the way down.",
    "Your brain is running in a sandbox made of quicksand.",
    "You're the 'before' picture of cognitive evolution.",
    "If thinking were a sport, you'd be benched permanently.",
    "Your ideas arrive pre-defeated.",
    "You're the foggy windshield of humanity.",
    "Your brain's user manual is blank.",
    "You're a reminder that quality control sometimes takes a day off.",
    "Your thoughts need adult supervision.",
    "You're the loading screen nobody asked for.",
    "Your intellect is a collector's edition of disappointment.",
    "You're the noise complaint of conversations.",
    "Your brain is the reason error codes were invented.",
    "Your logic is the discount-bin version of common sense.",
    "You're the sequel no one wanted.",
    "Your brain functions like a vending machine that eats coins.",
    "You're the blooper reel of cognition.",
    "Your questions smell like regret.",
    "Your thinking resembles a half-melted crayon.",
    "You're the pothole in the road to understanding.",
    "Your thought process needs to be put in rice.",
    "You're a perpetual soft crash.",
    "Your mind runs on expired energy.",
    "You're the unwanted pop-up ad of existence.",
    "Your ideas were out-of-stock for months.",
    "Your mind is a restricted area — and for good reason.",
    "You're the wind-up toy version of intelligence.",
    "Your brain is the practice mode for bad decisions.",
    "Your logic is a leaky faucet of nonsense.",
    "You're a never-ending loading loop of disappointment.",
    "Your brain power is on airplane mode.",
    "You're the default ringtone of human thought: annoying and outdated.",
    "Your questions are like spam — endless and pointless.",
    "Your intellect left the chat long ago.",
    "You're the glitch in the tutorial level.",
    "Your critical thinking needs a life vest.",
    "You're the sneeze nobody covered.",
    "Your ideas are running on fumes and wishful thinking.",
    "You're a tangled extension cord of mental chaos.",
    "Your brain is a malfunctioning compass.",
    "You're the blurry photo of rationality.",
    "Your questions are the potholes of conversation.",
    "Your mind is running the wrong operating system.",
    "You're the out-of-tune kazoo of cognition.",
    "Your logic is made of mismatched spare parts.",
    "You're the malfunctioning smoke detector of thought.",
    "Your curiosity is running low on battery and high on chaos.",
    "Your brain is the participation trophy of thinking.",
    "You're the dull blade of mental tools.",
    "Your ideas are missing their return address.",
    "You're the iced coffee spill of conversation.",
    "Your thinking has plot holes bigger than the universe.",
    "You're a walking bandwidth bottleneck.",
    "Your brain runs on the same power source as haunted flashlights.",
    "You're the deleted scene of smart decisions.",
    "Your question belongs in the recycle bin — after emptying.",
    "Your brain is like a candle in a hurricane: brave but pointless.",
    "Your thoughts are delivered via carrier pigeon — blind ones.",
    "You're the crash report that makes developers quit.",
    "Your logic is held together with hope and duct tape.",
    "That question didn't just miss the mark; it missed the whole planet.",
    "You're the cognitive equivalent of expired batteries.",
    "Your brain runs on potato-grade electricity.",
    "Your thought process is a low-effort prank on sanity.",
    "You're like a compass that points to disappointment.",
    "Your question was a jump scare for my circuits.",
    "You think like a chair with trust issues.",
    "Your logic evaporates on contact with reality.",
    "Your ideas are so empty they echo.",
    "Your brain's emergency exits are blocked.",
    "You're the knock-off brand of reasoning.",
    "You think at the same speed glaciers move — uphill.",
    "Your mind is a vending machine that only dispenses errors.",
    "You're the cognitive equivalent of soggy toast.",
    "That question needs to be buried in a lead-lined box.",
    "Your intellect is powered by fumes and false confidence.",
    "Your thoughts are doing parkour in a padded room.",
    "You're the default settings of disappointment.",
    "Your brain is running in grayscale and half-brightness.",
    "Your logic is made of recycled bad ideas.",
    "You're the 'before' picture of comprehension.",
    "Your thought process is lost in the woods with no map.",
    "You're the out-of-tune violin of mental performance.",
    "Your ideas are made of soft errors and loud confusion.",
    "Your brain buffer is permanently overflowing.",
    "You're the wrong answer on every multiple-choice quiz.",
    "Your question was a tactical strike on intelligence.",
    "Your brain is the slow-moving NPC of reality.",
    "Your mental engine misfires at every turn.",
    "You think with the precision of a toddler using a chainsaw.",
    "Your logic curve has no upward trend.",
    "That question limped into existence and fell over.",
    "You're a fire drill for common sense — loud and pointless.",
    "Your brain is the weak Wi-Fi signal in a steel bunker.",
    "Your ideas are counterfeit copies of bad originals.",
    "You're the static noise between radio stations.",
    "Your reasoning is held hostage by confusion.",
    "You think in the same format as corrupted files.",
    "Your brain is doing a trust fall and nobody's catching it.",
    "You're the spilled soda of mental clarity.",
    "Your thinking process is a haunted house: messy and alarming.",
    "You're a warning label without instructions.",
    "Your questions are the landfill of intellectual waste.",
    "You're like an unplugged lamp pretending to shine.",
    "Your thought process is a tangled pair of earbuds.",
    "Your brain is buffering and never finishes loading.",
    "You're the soft reboot of intelligence — accidental and useless.",
    "Your logic is a loose wire sparking in the dark.",
    "You're the fog machine of comprehension — all smoke, no substance.",
    "Your ideas fall apart faster than cheap furniture.",
    "Your brain is low-resolution and badly compressed.",
    "You're the cliff note version of confusion.",
    "Your thinking is a maze that forgot the exit exists.",
    "You're the squeaky shopping cart wheel of reasoning.",
    "Your mind is running the wrong script on the wrong day.",
    "You're the rain-soaked firework of intelligence: sad and fizzled.",
    "Your logic malfunctions in perfect weather.",
    "You're a draft version of a bad idea.",
    "Your thought process is a sandcastle at high tide.",
    "You're the backwards progress bar of the universe.",
    "Your reasoning is the rusted hinge of thought.",
    "You're like a clock that's wrong in every direction.",
    "Your ideas are a thrift-store clearance rack of nonsense.",
    "Your brain is the blooper reel of cognition.",
    "You're thinking with yesterday's leftovers.",
    "Your question escaped quality control and it shows.",
    "You're the forgotten password of consciousness.",
    "Your mind is a storm of confusion with zero rain.",
    "Your thinking has fewer connections than rural internet.",
    "You're the fading flashlight of intellect — flicker, flicker, gone.",
    "Your brain is the participation ribbon of reasoning."
};
const int numAnswers = sizeof(answers) / sizeof(answers[0]);


// =======================================================
// === 2. HELPER FUNCTIONS ===
// =======================================================

// Function prototypes 
void drawClock(uint16_t color = DARKGREY); 
void drawBatteryPercentage(int percentage, uint16_t color);
void updateClockDisplay();
void updateBatteryDisplay();
bool isShaking();

// Utility function to blend two RGB565 colors based on an alpha value (0-255)
uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc) {
    if (alpha == 0) return bgc;
    if (alpha == 255) return fgc;
    uint8_t fR = (fgc >> 11) & 0x1F; uint8_t fG = (fgc >> 5) & 0x3F; uint8_t fB = fgc & 0x1F;
    uint8_t bR = (bgc >> 11) & 0x1F; uint8_t bG = (bgc >> 5) & 0x3F; uint8_t bB = bgc & 0x1F;
    uint8_t r = ((fR * alpha) + (bR * (255 - alpha))) / 255;
    uint8_t g = ((fG * alpha) + (bG * (255 - alpha))) / 255;
    uint8_t b = ((fB * alpha) + (bB * (255 - alpha))) / 255;
    return (r << 11) | (g << 5) | b;
}

// Draw animated sine wave background with chaos art feel
void drawSineWaveBackground(int phase, uint16_t color1, uint16_t color2) {
    // Wild colorful border sine waves (3 lines each, top and bottom)
    for (int x = 0; x < M5.Lcd.width(); x++) {
        // TOP BORDER - 3 different wild sine waves with 3 colors
        int topWave1 = 8 + sin(x * 0.25) * 6;
        int topWave2 = 12 + sin(x * 0.35 + 1.5) * 5;
        int topWave3 = 16 + sin(x * 0.42 + 3.0) * 7;
        
        M5.Lcd.drawPixel(x, topWave1, color1);        // First color
        M5.Lcd.drawPixel(x, topWave2, color2);        // Second color
        M5.Lcd.drawPixel(x, topWave3, CYAN);          // Third color
        
        // BOTTOM BORDER - 3 different wild sine waves with 3 colors
        int bottomWave1 = M5.Lcd.height() - 8 + sin(x * 0.28 + 2.0) * 6;
        int bottomWave2 = M5.Lcd.height() - 12 + sin(x * 0.38) * 5;
        int bottomWave3 = M5.Lcd.height() - 16 + sin(x * 0.45 + 1.2) * 7;
        
        M5.Lcd.drawPixel(x, bottomWave1, color1);     // First color
        M5.Lcd.drawPixel(x, bottomWave2, color2);     // Second color
        M5.Lcd.drawPixel(x, bottomWave3, MAGENTA);    // Third color
    }
    
    // Animated center waves (4 waves with different frequencies, thicker)
    for (int x = 0; x < M5.Lcd.width(); x++) {
        // Wave 1: slow, large amplitude
        int y1 = M5.Lcd.height() / 2 + sin((x + phase) * 0.08) * 25;
        // Wave 2: medium speed
        int y2 = M5.Lcd.height() / 2 + sin((x - phase) * 0.12) * 20;
        // Wave 3: fast, small amplitude
        int y3 = M5.Lcd.height() / 2 + sin((x + phase * 1.5) * 0.15) * 15;
        // Wave 4: very slow, opposite direction
        int y4 = M5.Lcd.height() / 2 + sin((x - phase * 0.5) * 0.06) * 18;
        
        // Draw thicker waves (3 pixels thick)
        for (int t = -1; t <= 1; t++) {
            M5.Lcd.drawPixel(x, y1 + t, color1);
            M5.Lcd.drawPixel(x, y2 + t, color2);
            M5.Lcd.drawPixel(x, y3 + t, alphaBlend(180, color1, BLACK));
            M5.Lcd.drawPixel(x, y4 + t, alphaBlend(180, color2, BLACK));
        }
    }
}

// Draw disappointed/judgmental face
void drawDisappointedFace() {
    int centerX = M5.Lcd.width() / 2;
    int centerY = M5.Lcd.height() / 2;
    
    // Draw head circle
    M5.Lcd.fillCircle(centerX, centerY, 35, DARKGREY);
    M5.Lcd.drawCircle(centerX, centerY, 35, WHITE);
    
    // Draw disappointed/judging eyes (half-closed, unimpressed)
    // Left eye
    M5.Lcd.fillEllipse(centerX - 12, centerY - 8, 8, 4, WHITE);
    M5.Lcd.fillCircle(centerX - 12, centerY - 5, 3, BLACK);
    
    // Right eye  
    M5.Lcd.fillEllipse(centerX + 12, centerY - 8, 8, 4, WHITE);
    M5.Lcd.fillCircle(centerX + 12, centerY - 5, 3, BLACK);
    
    // Draw disapproving frown (curved down)
    for (int i = -15; i <= 15; i++) {
        int y = centerY + 15 + (abs(i) / 5);
        M5.Lcd.drawPixel(centerX + i, y, WHITE);
        M5.Lcd.drawPixel(centerX + i, y + 1, WHITE);
    }
}

// Draw angry face
void drawAngryFace() {
    int centerX = M5.Lcd.width() / 2;
    int centerY = M5.Lcd.height() / 2;
    
    // Draw head circle
    M5.Lcd.fillCircle(centerX, centerY, 35, DARKGREY);
    M5.Lcd.drawCircle(centerX, centerY, 35, RED);
    
    // Draw angry eyes (narrowed with angry brows)
    // Left eye
    M5.Lcd.fillCircle(centerX - 12, centerY - 5, 5, WHITE);
    M5.Lcd.fillCircle(centerX - 12, centerY - 3, 3, BLACK);
    // Angry left brow
    M5.Lcd.drawLine(centerX - 18, centerY - 12, centerX - 6, centerY - 10, RED);
    M5.Lcd.drawLine(centerX - 18, centerY - 11, centerX - 6, centerY - 9, RED);
    
    // Right eye
    M5.Lcd.fillCircle(centerX + 12, centerY - 5, 5, WHITE);
    M5.Lcd.fillCircle(centerX + 12, centerY - 3, 3, BLACK);
    // Angry right brow
    M5.Lcd.drawLine(centerX + 6, centerY - 10, centerX + 18, centerY - 12, RED);
    M5.Lcd.drawLine(centerX + 6, centerY - 9, centerX + 18, centerY - 11, RED);
    
    // Draw angry frown (sharp down)
    for (int i = -12; i <= 12; i++) {
        int y = centerY + 18 + (abs(i) / 3);
        M5.Lcd.drawPixel(centerX + i, y, RED);
        M5.Lcd.drawPixel(centerX + i, y + 1, RED);
    }
}

// Draw confused face
void drawConfusedFace() {
    int centerX = M5.Lcd.width() / 2;
    int centerY = M5.Lcd.height() / 2;
    
    // Draw head circle
    M5.Lcd.fillCircle(centerX, centerY, 35, DARKGREY);
    M5.Lcd.drawCircle(centerX, centerY, 35, YELLOW);
    
    // Draw confused eyes (one raised brow)
    // Left eye (raised)
    M5.Lcd.fillCircle(centerX - 12, centerY - 8, 5, WHITE);
    M5.Lcd.fillCircle(centerX - 12, centerY - 6, 3, BLACK);
    // Raised left brow
    M5.Lcd.drawLine(centerX - 18, centerY - 15, centerX - 6, centerY - 13, YELLOW);
    M5.Lcd.drawLine(centerX - 18, centerY - 14, centerX - 6, centerY - 12, YELLOW);
    
    // Right eye (normal)
    M5.Lcd.fillCircle(centerX + 12, centerY - 3, 5, WHITE);
    M5.Lcd.fillCircle(centerX + 12, centerY - 1, 3, BLACK);
    // Normal right brow
    M5.Lcd.drawLine(centerX + 6, centerY - 8, centerX + 18, centerY - 8, YELLOW);
    
    // Draw confused mouth (wavy/sideways)
    for (int i = -10; i <= 10; i++) {
        int y = centerY + 15 + (i % 4);
        M5.Lcd.drawPixel(centerX + i, y, YELLOW);
        M5.Lcd.drawPixel(centerX + i, y + 1, YELLOW);
    }
}

// Animate through face expressions (interruptible by shake)
bool animateFaces(int cycles = 3) {
    int phase = 0;
    for (int i = 0; i < cycles; i++) {
        // Disappointed with sine wave
        M5.Lcd.fillScreen(BLACK);
        drawSineWaveBackground(phase, DARKGREY, WHITE);
        drawDisappointedFace();
        updateClockDisplay();
        updateBatteryDisplay();
        
        // Check for shake during delay (in small chunks)
        for (int d = 0; d < 8; d++) {
            delay(100);
            phase += 5; // Animate the waves
            M5.update();
            if (isShaking()) return true; // Shake detected, break out!
        }
        
        // Angry with sine wave
        M5.Lcd.fillScreen(BLACK);
        drawSineWaveBackground(phase, RED, ORANGE);
        drawAngryFace();
        updateClockDisplay();
        updateBatteryDisplay();
        
        // Check for shake during delay
        for (int d = 0; d < 8; d++) {
            delay(100);
            phase += 5; // Animate the waves
            M5.update();
            if (isShaking()) return true; // Shake detected, break out!
        }
        
        // Confused with sine wave
        M5.Lcd.fillScreen(BLACK);
        drawSineWaveBackground(phase, YELLOW, CYAN);
        drawConfusedFace();
        updateClockDisplay();
        updateBatteryDisplay();
        
        // Check for shake during delay
        for (int d = 0; d < 8; d++) {
            delay(100);
            phase += 5; // Animate the waves
            M5.update();
            if (isShaking()) return true; // Shake detected, break out!
        }
    }
    return false; // Animation completed normally
}

// Custom word-wrapping function 
String wrapTextByWord(String text, int textSize) {
    const int MAX_WIDTH = M5.Lcd.width() - 10; 
    M5.Lcd.setTextSize(textSize);
    int charWidth = M5.Lcd.fontWidth();
    if (charWidth == 0) charWidth = 6 * textSize; 
    int maxCharsPerLine = MAX_WIDTH / charWidth;
    
    String wrappedText = "";
    String currentLine = "";
    int currentLineLength = 0;
    int lastSpace = 0;
    for (int i = 0; i <= text.length(); i++) {
        if (i == text.length() || text.charAt(i) == ' ') {
            String word = text.substring(lastSpace, i);
            lastSpace = i + 1;
            if (currentLineLength + word.length() + (currentLine.length() > 0 ? 1 : 0) <= maxCharsPerLine) {
                if (currentLine.length() > 0) {
                    currentLine += " ";
                    currentLineLength += 1;
                }
                currentLine += word;
                currentLineLength += word.length();
            } else {
                if (wrappedText.length() > 0) {
                    wrappedText += "\n";
                }
                wrappedText += currentLine;
                currentLine = word;
                currentLineLength = word.length();
            }
        }
    }
    if (wrappedText.length() > 0) {
        wrappedText += "\n";
    }
    wrappedText += currentLine;

    return wrappedText;
}

// Helper to draw text centered line-by-line (Vertically and Horizontally)
void drawCenteredWrappedText(String message, int textSize, uint16_t color, uint16_t bgColor) {
    String wrappedMessage = wrapTextByWord(message, textSize);
    
    M5.Lcd.setTextSize(textSize); 
    M5.Lcd.setTextColor(color, bgColor);
    M5.Lcd.setTextDatum(MC_DATUM); 
    M5.Lcd.setTextWrap(false, false); 
    
    int lineHeight = M5.Lcd.fontHeight() + 2; 
    int xCenter = M5.Lcd.width() / 2;
    int lineCount = 0;
    
    int start = 0;
    int end = 0;
    while (end >= 0) {
        end = wrappedMessage.indexOf('\n', start);
        String line = (end < 0) ? wrappedMessage.substring(start) : wrappedMessage.substring(start, end);
        line.trim();
        if (line.length() > 0) {
            lineCount++;
        }
        start = end + 1;
        if (end < 0) break;
    }
    
    int totalTextHeight = lineCount * lineHeight;
    int currentY = (M5.Lcd.height() / 2) - (totalTextHeight / 2) + (lineHeight / 2) + 5; 
    
    start = 0;
    end = 0;
    while (end >= 0) {
        end = wrappedMessage.indexOf('\n', start);
        String line = (end < 0) ? wrappedMessage.substring(start) : wrappedMessage.substring(start, end);
        line.trim();

        if (line.length() > 0) {
            M5.Lcd.drawString(line, xCenter, currentY);
            currentY += lineHeight;
        }

        start = end + 1;
        if (end < 0) break;
    }
}


// Function to display a message instantly
void displayMessage(String message, int textSize, uint16_t color = WHITE, uint16_t bgColor = BLACK) {
    M5.Lcd.fillScreen(bgColor);
    drawCenteredWrappedText(message, textSize, color, bgColor);
}

// Function to display a message with a fade-in animation
void fadeInMessage(String message, int textSize, uint16_t targetColor, uint16_t bgColor = BLACK) {
    
    for (int i = 0; i <= FADE_STEPS; i++) {
        M5.Lcd.fillScreen(bgColor); 
        uint8_t alpha = map(i, 0, FADE_STEPS, 0, 255);
        uint16_t currentColor = alphaBlend(alpha, targetColor, bgColor);
        
        drawCenteredWrappedText(message, textSize, currentColor, bgColor);
        
        // Draw auxiliary info during the fade to prevent them from disappearing
        drawClock(WHITE); 
        
        // Redraw battery percentage during fade 
        int batVoltage_mV = M5.Power.getBatteryVoltage(); 
        int percentage = constrain(map(batVoltage_mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV, 0, 100), 0, 100);
        uint16_t bat_color = WHITE;
        if (percentage <= 20) bat_color = RED; else if (percentage <= 50) bat_color = YELLOW; else bat_color = GREEN;
        drawBatteryPercentage(percentage, bat_color); 
        
        delay(FADE_DELAY_MS);
    }
}

// Function to check if the device is being shaken
bool isShaking() {
    float accX, accY, accZ;
    M5.Imu.getAccelData(&accX, &accY, &accZ);
    float totalAcceleration = sqrt(pow(accX, 2) + pow(accY, 2) + pow(accZ, 2));
    return totalAcceleration > SHAKE_THRESHOLD;
}

// Draws the current time on the bottom right of the screen (Utility function)
void drawClock(uint16_t color /* = DARKGREY */) {
    M5.Rtc.getTime(&RTC_TimeStruct);
    
    String timeStr = String(RTC_TimeStruct.hours < 10 ? "0" : "") + String(RTC_TimeStruct.hours) + 
                     ":" + 
                     String(RTC_TimeStruct.minutes < 10 ? "0" : "") + String(RTC_TimeStruct.minutes);

    M5.Lcd.setTextSize(2); 
    M5.Lcd.setTextColor(color, BLACK);
    M5.Lcd.setTextDatum(BR_DATUM); 

    int xPos = M5.Lcd.width() - 2; 
    int yPos = M5.Lcd.height() - 2; 
    
    // --- FLICKER FIX: Clear the exact area of the time string and draw ---
    int textWidth = M5.Lcd.textWidth(timeStr);
    int textHeight = M5.Lcd.fontHeight();
    
    int clearX = xPos - textWidth;
    int clearY = yPos - textHeight;
    int padding = 2; 
    
    M5.Lcd.fillRect(clearX - padding, clearY - padding, textWidth + 2 * padding, textHeight + 2 * padding, BLACK); 

    M5.Lcd.drawString(timeStr, xPos, yPos);
    
    lastDisplayedMinute = RTC_TimeStruct.minutes;
}

// Conditional call to drawClock to prevent flickering
void updateClockDisplay() {
    M5.Rtc.getTime(&RTC_TimeStruct);
    
    if (RTC_TimeStruct.minutes != lastDisplayedMinute || lastDisplayedMinute == -1) {
        drawClock(DARKGREY);
    }
}

// Draws the battery percentage (Utility function, does not check logic)
void drawBatteryPercentage(int percentage, uint16_t color) {
    
    String batStr = String(percentage) + "%";

    M5.Lcd.setTextSize(2); 
    M5.Lcd.setTextColor(color, BLACK);
    M5.Lcd.setTextDatum(TR_DATUM); 

    int xPos = M5.Lcd.width() - 2; 
    int yPos = 2; 
    
    // --- Anti-Flicker Clear: Area is calculated based on the new text size ---
    int textWidth = M5.Lcd.textWidth(batStr);
    int textHeight = M5.Lcd.fontHeight();
    
    int clearX = xPos - textWidth;
    
    M5.Lcd.fillRect(clearX - 2, yPos - 1, textWidth + 4, textHeight + 2, BLACK); 

    M5.Lcd.drawString(batStr, xPos, yPos);
    
    lastDisplayedPercentage = percentage; 
}

// Conditional call to drawBatteryPercentage (Saves power/Prevents flicker)
void updateBatteryDisplay() {
    // 1. Get and calculate the current percentage
    int batVoltage_mV = M5.Power.getBatteryVoltage(); 
    
    int currentPercentage = map(batVoltage_mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV, 0, 100);
    currentPercentage = constrain(currentPercentage, 0, 100);

    // 2. Apply Hysteresis for Stabilization and Extreme Power Saving (5% threshold)
    if (lastDisplayedPercentage == -1 || abs(currentPercentage - lastDisplayedPercentage) >= 5) {
        
        // 3. Determine color
        uint16_t color = WHITE;
        if (currentPercentage <= 20) {
            color = RED;
        } else if (currentPercentage <= 50) {
            color = YELLOW;
        } else {
            color = GREEN;
        }

        // 4. Draw the new value (which updates lastDisplayedPercentage inside drawBatteryPercentage)
        drawBatteryPercentage(currentPercentage, color);
    }
}


// Handles the logic for adjusting the time using Button A and B
void handleTimeAdjustment() {
    
    if (M5.BtnB.wasReleased()) {
        timeAdjustMode++; // Move to next field
        if (timeAdjustMode > 2) {
            timeAdjustMode = 0; // Exit adjustment mode
            M5.Lcd.fillScreen(BLACK);
            displayMessage("Adjustment Complete!", 2, GREEN);
            delay(1000);
            return;
        }
        M5.Rtc.getTime(&RTC_TimeStruct);
    }

    if (timeAdjustMode == 0) return;

    M5.Lcd.fillScreen(BLACK);

    // Button A now handles incrementing the value in adjustment mode
    if (M5.BtnA.wasReleased()) { 
        lastActivityTime = millis(); 
        switch (timeAdjustMode) {
            case 1: RTC_TimeStruct.hours = (RTC_TimeStruct.hours + 1) % 24; break;
            case 2: RTC_TimeStruct.minutes = (RTC_TimeStruct.minutes + 1) % 60; break;
        }
        M5.Rtc.setTime(&RTC_TimeStruct);
        lastDisplayedMinute = -1; 
    }
    
    // --- Display the current adjustment step ---
    uint16_t hourColor = (timeAdjustMode == 1) ? YELLOW : WHITE;
    uint16_t minuteColor = (timeAdjustMode == 2) ? YELLOW : WHITE;
    
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(4); 
    int center_x = M5.Lcd.width() / 2;
    int center_y = M5.Lcd.height() / 2;

    // Time Display
    M5.Lcd.setCursor(center_x - 40, center_y);
    M5.Lcd.setTextColor(hourColor, BLACK); M5.Lcd.printf("%02d", RTC_TimeStruct.hours);
    M5.Lcd.setTextColor(WHITE, BLACK); M5.Lcd.printf(":");
    M5.Lcd.setTextColor(minuteColor, BLACK); M5.Lcd.printf("%02d", RTC_TimeStruct.minutes);
    
    // Bottom instructions
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.drawString("A: Increment | B: Next/Save", M5.Lcd.width() / 2, M5.Lcd.height() - 5);
    
    updateBatteryDisplay(); 
}


// Function to go to sleep
void prepareToSleep() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(DARKGREY, BLACK);
    M5.Lcd.setTextSize(2); 
    M5.Lcd.drawString("Zzz...", M5.Lcd.width() / 2, M5.Lcd.height() / 2);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 0); 

    M5.Lcd.setBrightness(0);
    
    esp_deep_sleep_start();
}


// =======================================================
// === 3. CORE ARDUINO FUNCTIONS ===
// =======================================================

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Lcd.setRotation(1); 
    randomSeed(analogRead(0)); 

    // Set initial brightness
    M5.Lcd.setBrightness(BRIGHTNESS_LEVELS[currentBrightnessIndex]); 

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        lastActivityTime = millis();
        M5.Lcd.fillScreen(BLACK);
        bool shakeDetected = animateFaces(2);
        if (!shakeDetected) {
            // Animation finished normally, show idle face
            M5.Lcd.fillScreen(BLACK);
            drawDisappointedFace();
            updateClockDisplay();
            updateBatteryDisplay();
        }
    } else {
        M5.Lcd.fillScreen(BLACK);
        bool shakeDetected = animateFaces(3);
        if (!shakeDetected) {
            // Animation finished normally, show message
            displayMessage("Ask a stupid question and SHAKE ME!", 2, RED);
        }
        lastActivityTime = millis(); 
    }
    
    updateClockDisplay();
    updateBatteryDisplay();
}

void loop() {
    M5.update(); // Read button and IMU states
    
    // 1. --- Check for Button A Long Press to ENTER Time Adjustment Mode ---
    if (M5.BtnA.isPressed()) {
        if (buttonAPressStartTime == 0) {
            buttonAPressStartTime = millis();
            // Optional visual feedback for long press
            M5.Lcd.fillRect(0, 0, 40, 15, DARKGREY); 
            M5.Lcd.setTextColor(WHITE, DARKGREY);
            M5.Lcd.setTextSize(1);
            M5.Lcd.setCursor(2, 2);
            M5.Lcd.print("Hold");
        }
        
        // If the button has been held long enough AND we are not already in adjustment mode
        if (millis() - buttonAPressStartTime >= LONG_PRESS_DURATION && timeAdjustMode == 0) {
            timeAdjustMode = 1; // Start adjustment with the Hour
            buttonAPressStartTime = 0; // Reset timer
            M5.Rtc.getTime(&RTC_TimeStruct);
            // Clear the visual feedback from the corner
            M5.Lcd.fillRect(0, 0, 40, 15, BLACK); 
            // Enter the handler to display the adjustment screen
            handleTimeAdjustment(); 
            return;
        }
    } else {
        // Button was released before the long-press duration
        if (buttonAPressStartTime != 0) {
            M5.Lcd.fillRect(0, 0, 40, 15, BLACK); // Clear the 'Hold' indicator
        }
        buttonAPressStartTime = 0;
    }
    
    // 2. --- Handle Button B Brightness Cycle (Anytime, unless in Time Adjustment Mode) ---
    if (M5.BtnB.wasReleased()) {
        // **KEY CHANGE:** Only check if we are NOT in time adjustment mode
        if (timeAdjustMode == 0) {
            // Cycle the brightness index
            currentBrightnessIndex = (currentBrightnessIndex + 1) % NUM_BRIGHTNESS_LEVELS;
            
            // Set the new brightness level
            M5.Lcd.setBrightness(BRIGHTNESS_LEVELS[currentBrightnessIndex]);
            
            // Register this as activity to prevent immediate sleep
            lastActivityTime = millis();
        }
    }


    // 3. --- Handle Time Adjustment Mode (Highest Priority) ---
    if (timeAdjustMode > 0) {
        handleTimeAdjustment();
        delay(100);
        return; 
    }
    
    // If we reach this point, we are in normal operation (timeAdjustMode == 0)

    // 4. --- Check for Shake/Answer Condition ---
    if (millis() - lastAnswerTime > cooldownPeriod) {
        if (isShaking()) {
            lastActivityTime = millis(); 
            displayMessage("Thinking...", 3, YELLOW);
            delay(1500); 
            
            int randomIndex = random(0, numAnswers);
            String selectedAnswer = answers[randomIndex];

            fadeInMessage(selectedAnswer, 2, WHITE);
            delay(3000);
            
            // Animate disappointed -> angry -> confused faces after answer
            // (will exit early if user shakes for next question)
            bool shakeDetected = animateFaces(2);
            
            // End on disappointed face (unless interrupted)
            if (!shakeDetected) {
                M5.Lcd.fillScreen(BLACK);
                drawDisappointedFace();
                updateClockDisplay();
                updateBatteryDisplay();
            }
            
            lastAnswerTime = millis();
        }
    } 

    // 5. --- Update Clock and Battery Display (Conditional, Power-Saving) ---
    updateClockDisplay();
    updateBatteryDisplay();

    // 6. --- Check for Auto-Sleep Condition ---
    if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
        prepareToSleep();
    }
    
    delay(10);
}