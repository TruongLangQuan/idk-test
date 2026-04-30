#pragma once
// GameContext.h - All pet stats and persistent game state

#include <Arduino.h>
#include <Preferences.h>

// Maximum inventory slots per category
static const int INV_MAX_TOYS   = 8;
static const int INV_MAX_SNACKS = 8;

struct Inventory {
    // Toy names (null = empty slot)
    const char* toys[INV_MAX_TOYS]   = {"Feather", "Yarn ball", "Laser", nullptr};
    int         toyCount             = 3;

    // Snack names
    const char* snacks[INV_MAX_SNACKS] = {"Treat", "Kibble", nullptr};
    int         snackCount             = 2;
};

struct EnvironmentCtx {
    // Indices into option arrays (see Settings.h HOUR_OPTS/SEASON_OPTS etc.)
    int time_of_day = 12; // 0-23 hours, default noon
    int season      = 0;  // 0=Spring..3=Winter
    int moon_phase  = 4;  // 0=New..7=WanCres, default Full
    int weather     = 0;  // 0=Clear..4=Snow
};

struct GameContext {
    // ── Rapidly changing (daily) ─────────────────────────────────────────
    float fullness    = 50.0f;  // inverse of hunger
    float energy      = 50.0f;  // how rested
    float comfort     = 50.0f;  // physical comfort
    float playfulness = 50.0f;  // mood to play
    float focus       = 50.0f;  // concentration

    // ── Medium changing (weekly) ─────────────────────────────────────────
    float health        = 50.0f;
    float fulfillment   = 50.0f;
    float cleanliness   = 50.0f;
    float curiosity     = 50.0f;
    float independence  = 50.0f;
    float sociability   = 50.0f;
    float routine       = 50.0f;
    float intelligence  = 50.0f;
    float resilience    = 50.0f;
    float maturity      = 50.0f;
    float grace         = 50.0f;
    float affection     = 50.0f;

    // ── Slow changing (monthly) ──────────────────────────────────────────
    float fitness    = 50.0f;
    float appetite   = 50.0f;
    float patience   = 50.0f;
    float charisma   = 50.0f;
    float craftiness = 50.0f;
    float serenity   = 50.0f;

    // ── Traits (near-static) ─────────────────────────────────────────────
    float courage         = 50.0f;
    float loyalty         = 50.0f;
    float mischievousness = 50.0f;
    float dignity         = 50.0f;

    // ── Inventory ────────────────────────────────────────────────────────
    Inventory inventory;

    // ── Environment ──────────────────────────────────────────────────────
    EnvironmentCtx environment;

    // ── Minigame records ─────────────────────────────────────────────────
    int zoomiesHighScore   = 0;
    int mazeBestTimeSec    = 0;   // 0 = not yet played
    int highScoreZoomies   = 0;
    int highScoreMaze      = 0;
    int highScoreBreakout  = 0;

    // ── Behavior override (set by behavior system) ───────────────────────
    const char* overrideNextBehavior = nullptr;

    // ── Helpers ──────────────────────────────────────────────────────────
    float getStat(const char* name) const {
        if (strcmp(name,"fullness")    ==0) return fullness;
        if (strcmp(name,"energy")      ==0) return energy;
        if (strcmp(name,"comfort")     ==0) return comfort;
        if (strcmp(name,"playfulness") ==0) return playfulness;
        if (strcmp(name,"focus")       ==0) return focus;
        if (strcmp(name,"health")      ==0) return health;
        if (strcmp(name,"fulfillment") ==0) return fulfillment;
        if (strcmp(name,"cleanliness") ==0) return cleanliness;
        if (strcmp(name,"curiosity")   ==0) return curiosity;
        if (strcmp(name,"independence")==0) return independence;
        if (strcmp(name,"sociability") ==0) return sociability;
        if (strcmp(name,"routine")     ==0) return routine;
        if (strcmp(name,"intelligence")==0) return intelligence;
        if (strcmp(name,"resilience")  ==0) return resilience;
        if (strcmp(name,"maturity")    ==0) return maturity;
        if (strcmp(name,"grace")       ==0) return grace;
        if (strcmp(name,"affection")   ==0) return affection;
        if (strcmp(name,"fitness")     ==0) return fitness;
        if (strcmp(name,"appetite")    ==0) return appetite;
        if (strcmp(name,"patience")    ==0) return patience;
        if (strcmp(name,"charisma")    ==0) return charisma;
        if (strcmp(name,"craftiness")  ==0) return craftiness;
        if (strcmp(name,"serenity")    ==0) return serenity;
        if (strcmp(name,"courage")     ==0) return courage;
        if (strcmp(name,"loyalty")     ==0) return loyalty;
        if (strcmp(name,"mischievousness")==0) return mischievousness;
        if (strcmp(name,"dignity")     ==0) return dignity;
        return 50.0f;
    }

    void setStat(const char* name, float v) {
        v = max(0.0f, min(100.0f, v));
        if (strcmp(name,"fullness")    ==0) { fullness    = v; return; }
        if (strcmp(name,"energy")      ==0) { energy      = v; return; }
        if (strcmp(name,"comfort")     ==0) { comfort     = v; return; }
        if (strcmp(name,"playfulness") ==0) { playfulness = v; return; }
        if (strcmp(name,"focus")       ==0) { focus       = v; return; }
        if (strcmp(name,"health")      ==0) { health      = v; return; }
        if (strcmp(name,"fulfillment") ==0) { fulfillment = v; return; }
        if (strcmp(name,"cleanliness") ==0) { cleanliness = v; return; }
        if (strcmp(name,"curiosity")   ==0) { curiosity   = v; return; }
        if (strcmp(name,"independence")==0) { independence= v; return; }
        if (strcmp(name,"sociability") ==0) { sociability = v; return; }
        if (strcmp(name,"routine")     ==0) { routine     = v; return; }
        if (strcmp(name,"intelligence")==0) { intelligence= v; return; }
        if (strcmp(name,"resilience")  ==0) { resilience  = v; return; }
        if (strcmp(name,"maturity")    ==0) { maturity    = v; return; }
        if (strcmp(name,"grace")       ==0) { grace       = v; return; }
        if (strcmp(name,"affection")   ==0) { affection   = v; return; }
        if (strcmp(name,"fitness")     ==0) { fitness     = v; return; }
        if (strcmp(name,"appetite")    ==0) { appetite    = v; return; }
        if (strcmp(name,"patience")    ==0) { patience    = v; return; }
        if (strcmp(name,"charisma")    ==0) { charisma    = v; return; }
        if (strcmp(name,"craftiness")  ==0) { craftiness  = v; return; }
        if (strcmp(name,"serenity")    ==0) { serenity    = v; return; }
        if (strcmp(name,"courage")     ==0) { courage     = v; return; }
        if (strcmp(name,"loyalty")     ==0) { loyalty     = v; return; }
        if (strcmp(name,"mischievousness")==0){mischievousness=v; return;}
        if (strcmp(name,"dignity")     ==0) { dignity     = v; return; }
    }

    void addStat(const char* name, float delta) {
        setStat(name, getStat(name) + delta);
    }

    // ── Persistence (ESP32 NVS) ───────────────────────────────────────────
    void saveEnv() {
        Preferences p;
        p.begin("catode_env", false);
        p.putInt("time",    environment.time_of_day);
        p.putInt("season",  environment.season);
        p.putInt("moon",    environment.moon_phase);
        p.putInt("weather", environment.weather);
        p.end();
    }

    void loadEnv() {
        Preferences p;
        p.begin("catode_env", true);
        environment.time_of_day = p.getInt("time",    12);
        environment.season      = p.getInt("season",   0);
        environment.moon_phase  = p.getInt("moon",     4);
        environment.weather     = p.getInt("weather",  0);
        p.end();
    }

    void savePetStats() {
        Preferences p;
        p.begin("catode_pet", false);
        p.putFloat("fullness",    fullness);
        p.putFloat("energy",      energy);
        p.putFloat("affection",   affection);
        p.putFloat("cleanliness", cleanliness);
        p.putFloat("playfulness", playfulness);
        p.putFloat("comfort",     comfort);
        p.putFloat("curiosity",   curiosity);
        p.putFloat("serenity",    serenity);
        p.end();
    }

    void loadPetStats() {
        Preferences p;
        p.begin("catode_pet", true);
        fullness    = p.getFloat("fullness",    50.0f);
        energy      = p.getFloat("energy",      60.0f);
        affection   = p.getFloat("affection",   60.0f);
        cleanliness = p.getFloat("cleanliness", 80.0f);
        playfulness = p.getFloat("playfulness", 50.0f);
        comfort     = p.getFloat("comfort",     60.0f);
        curiosity   = p.getFloat("curiosity",   40.0f);
        serenity    = p.getFloat("serenity",    60.0f);
        p.end();
    }
};
