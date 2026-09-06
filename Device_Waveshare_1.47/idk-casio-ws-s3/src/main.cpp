#include <Arduino.h>
#include <SPI.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <cstring>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Display Pins
#define TFT_BL 46
#define TFT_RST 39
#define TFT_DC 41
#define TFT_MOSI 45
#define TFT_MISO 18
#define TFT_SCLK 40
#define TFT_CS 42

class LGFX_Custom : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
public:
    LGFX_Custom(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = TFT_MISO;
            cfg.pin_dc = TFT_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = TFT_CS;
            cfg.pin_rst = TFT_RST;
            cfg.pin_busy = -1;
            cfg.panel_width = 172;
            cfg.panel_height = 320;
            cfg.offset_x = 34;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;
            _panel_instance.config(cfg);
        }
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = TFT_BL;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        setPanel(&_panel_instance);
    }
};

LGFX_Custom display;
LGFX_Sprite canvas(&display);

#define SCREEN_W 320
#define SCREEN_H 172

// Joystick Pins
#define BTN_UP 0
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_CENTER 5

bool btnDown(int p) { return digitalRead(p) == LOW; }

// --- MATH STATE ---
double global_ans = 0;
bool deg_mode = true;
double table_x = 0;

// --- MATH EVALUATOR ---
double eval_expr(const char*& s);

void skip_ws(const char*& s) {
    while(*s == ' ') s++;
}

double eval_factor(const char*& s) {
    skip_ws(s);
    double res = 0;
    bool neg = false;
    if (*s == '-') { neg = true; s++; skip_ws(s); }
    else if (*s == '+') { s++; skip_ws(s); }
    
    if (*s == '(') {
        s++; res = eval_expr(s); skip_ws(s);
        if (*s == ')') s++;
    } else if (strncmp(s, "sin(", 4) == 0) {
        s += 4; double v = eval_expr(s); res = sin(deg_mode ? v * M_PI / 180.0 : v); if(*s==')') s++;
    } else if (strncmp(s, "cos(", 4) == 0) {
        s += 4; double v = eval_expr(s); res = cos(deg_mode ? v * M_PI / 180.0 : v); if(*s==')') s++;
    } else if (strncmp(s, "tan(", 4) == 0) {
        s += 4; double v = eval_expr(s); res = tan(deg_mode ? v * M_PI / 180.0 : v); if(*s==')') s++;
    } else if (strncmp(s, "log(", 4) == 0) {
        s += 4; res = log10(eval_expr(s)); if(*s==')') s++;
    } else if (strncmp(s, "ln(", 3) == 0) {
        s += 3; res = log(eval_expr(s)); if(*s==')') s++;
    } else if (strncmp(s, "sqrt(", 5) == 0) {
        s += 5; res = sqrt(eval_expr(s)); if(*s==')') s++;
    } else if (strncmp(s, "pi", 2) == 0) {
        s += 2; res = M_PI;
    } else if (strncmp(s, "e", 1) == 0 && s[1] != '^') {
        s += 1; res = M_E;
    } else if (strncmp(s, "ans", 3) == 0) {
        s += 3; res = global_ans;
    } else if (*s == 'x') {
        s += 1; res = table_x;
    } else {
        char* end;
        res = strtod(s, &end);
        s = end;
    }
    
    skip_ws(s);
    if (*s == '^') {
        s++;
        res = pow(res, eval_factor(s));
    }
    return neg ? -res : res;
}

double eval_term(const char*& s) {
    double res = eval_factor(s);
    while (true) {
        skip_ws(s);
        if (*s == '*') { s++; res *= eval_factor(s); }
        else if (*s == '/') { s++; res /= eval_factor(s); }
        else if (*s == '(' || *s == 'x' || strncmp(s, "sin", 3)==0 || strncmp(s, "cos", 3)==0 || 
                 strncmp(s, "tan", 3)==0 || strncmp(s, "pi", 2)==0 || strncmp(s, "e", 1)==0 || 
                 strncmp(s, "sqrt", 4)==0 || strncmp(s, "log", 3)==0 || strncmp(s, "ln", 2)==0 ||
                 strncmp(s, "ans", 3)==0) {
            res *= eval_factor(s);
        }
        else break;
    }
    return res;
}

double eval_expr(const char*& s) {
    double res = eval_term(s);
    while (true) {
        skip_ws(s);
        if (*s == '+') { s++; res += eval_term(s); }
        else if (*s == '-') { s++; res -= eval_term(s); }
        else break;
    }
    return res;
}

std::string evaluate(std::string expr) {
    if (expr.empty()) return "";
    const char* s = expr.c_str();
    double val = eval_expr(s);
    global_ans = val;
    char buf[32];
    snprintf(buf, sizeof(buf), "%g", val);
    return std::string(buf);
}

// --- APP STATE ---
enum AppMode {
    APP_HOME, APP_CALC, APP_STAT, APP_DIST, APP_SPREAD, APP_TABLE,
    APP_EQN, APP_INEQ, APP_COMPLEX, APP_BASEN, APP_MATRIX, APP_VECTOR,
    APP_RATIO, APP_MATHBOX, APP_SETTINGS
};
AppMode current_app = APP_HOME;

#define NUM_APPS 15
const char* apps[NUM_APPS] = {
    "Calculate", "Statistics", "Distribution", 
    "Spreadsheet", "Table", "Equation", 
    "Inequality", "Complex", "Base-N", 
    "Matrix", "Vector", "Ratio", 
    "Math Box", "Settings", "Exit System"
};

int home_sel = 0;
int home_scroll = 0;

// CALCULATE APP
std::string expr_str = "";
std::string res_str = "";
int kbd_x = 0;
int kbd_y = 0;
const char* kbd[5][8] = {
    {"sin(", "cos(", "tan(", "log(", "ln(", "^", "sqrt(", "x"},
    {"7",    "8",    "9",    "DEL",  "AC",  "(",  ")",     "pi"},
    {"4",    "5",    "6",    "*",    "/",   "e",  "",      ""},
    {"1",    "2",    "3",    "+",    "-",   "",   "",      ""},
    {"0",    ".",    "ans",  "MENU", "=",   "",   "",      ""}
};

// DATA FOR OTHER APPS
bool table_viewing = false;
float eqn_a = 1, eqn_b = 0, eqn_c = 0; int eqn_sel = 0; bool eqn_solving = false;
float ratio_a = 1, ratio_b = 2, ratio_d = 4; int ratio_sel = 0; bool ratio_solving = false;
int mathbox_sel = 0; int dice_val = 0; int coin_val = -1;

// STATS APP
float stat_data[3] = {0,0,0}; int stat_sel = 0;

// DIST APP
float dist_x = 0; 

// SPREADSHEET APP
float sheet[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
int sheet_r = 0, sheet_c = 0;

// INEQ APP
float ineq_a = 1, ineq_b = 0; int ineq_sel = 0;

// COMPLEX APP
float cx_a = 1, cx_b = 1; int cx_sel = 0;

// MATRIX APP
float mat[2][2] = {{1,0},{0,1}}; int mat_r = 0, mat_c = 0;

// VECTOR APP
float vec_x = 1, vec_y = 1, vec_z = 1; int vec_sel = 0;


void renderHome() {
    canvas.fillSprite(TFT_WHITE);
    canvas.fillRect(0, 0, SCREEN_W, 25, display.color565(30, 30, 200));
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1.5f);
    canvas.setCursor(10, 5);
    canvas.print("CASIO fx-880BTG (Home)");
    
    for(int i=0; i<6; i++) {
        int app_idx = home_scroll + i;
        if(app_idx >= NUM_APPS) break;
        
        int r = i / 3;
        int c = i % 3;
        int bx = 20 + c * 95;
        int by = 40 + r * 60;
        
        if (app_idx == home_sel) {
            canvas.fillRoundRect(bx, by, 90, 50, 5, display.color565(200, 230, 255));
            canvas.drawRoundRect(bx, by, 90, 50, 5, TFT_BLUE);
        } else {
            canvas.fillRoundRect(bx, by, 90, 50, 5, display.color565(240, 240, 240));
            canvas.drawRoundRect(bx, by, 90, 50, 5, TFT_LIGHTGREY);
        }
        
        canvas.setTextColor(app_idx == NUM_APPS-1 ? TFT_RED : TFT_BLACK);
        canvas.setTextSize(1.0f);
        canvas.setCursor(bx + 5, by + 20);
        canvas.print(apps[app_idx]);
    }
    
    canvas.fillRect(315, 30, 5, 140, TFT_LIGHTGREY);
    int sb_h = 140 / ((NUM_APPS+2)/3);
    int sb_y = 30 + (home_scroll / 3) * sb_h;
    canvas.fillRect(315, sb_y, 5, sb_h, TFT_DARKGREY);
}

void renderKeyboard(int sy) {
    int kw = SCREEN_W / 8;
    int kh = (SCREEN_H - sy) / 5;
    for(int r=0; r<5; r++) {
        for(int c=0; c<8; c++) {
            if (kbd[r][c][0] == '\0') continue;
            int bx = c * kw; int by = sy + r * kh;
            if (r == kbd_y && c == kbd_x) {
                canvas.fillRoundRect(bx+1, by+1, kw-2, kh-2, 3, TFT_BLUE);
                canvas.setTextColor(TFT_WHITE);
            } else {
                canvas.fillRoundRect(bx+1, by+1, kw-2, kh-2, 3, (r==0)?display.color565(50, 50, 50):display.color565(220, 220, 220));
                canvas.setTextColor((r==0)?TFT_WHITE:TFT_BLACK);
                if (strcmp(kbd[r][c], "DEL") == 0 || strcmp(kbd[r][c], "AC") == 0) {
                    canvas.fillRoundRect(bx+1, by+1, kw-2, kh-2, 3, TFT_RED);
                    canvas.setTextColor(TFT_WHITE);
                }
            }
            canvas.setTextSize(1.0f); canvas.setCursor(bx + 4, by + 6); canvas.print(kbd[r][c]);
        }
    }
}

void renderCalc() {
    canvas.fillSprite(TFT_WHITE);
    canvas.fillRect(0, 0, SCREEN_W, 55, display.color565(245, 255, 250));
    canvas.drawRect(0, 0, SCREEN_W, 55, TFT_DARKGREY);
    canvas.setTextColor(TFT_DARKGREY); canvas.setTextSize(1.0f); canvas.setCursor(5, 2); canvas.print(deg_mode ? "D" : "R");
    canvas.setTextColor(TFT_BLACK); canvas.setTextSize(1.5f); canvas.setCursor(5, 15); canvas.print(expr_str.c_str());
    canvas.setTextSize(2.0f); canvas.setCursor(SCREEN_W - 10 - (res_str.length() * 12), 35); canvas.print(res_str.c_str());
    renderKeyboard(60);
}

void renderTable() {
    canvas.fillSprite(TFT_WHITE);
    if (!table_viewing) {
        canvas.fillRect(0, 0, SCREEN_W, 55, display.color565(245, 255, 250));
        canvas.setTextColor(TFT_BLACK); canvas.setTextSize(1.5f); canvas.setCursor(5, 5);
        canvas.print("f(x) = "); canvas.print(expr_str.c_str());
        renderKeyboard(60);
    } else {
        canvas.fillRect(0, 0, SCREEN_W, 25, TFT_BLUE);
        canvas.setTextColor(TFT_WHITE); canvas.setTextSize(1.5f); canvas.setCursor(5, 5); canvas.printf("Table: f(x)=%s", expr_str.c_str());
        canvas.setTextColor(TFT_BLACK);
        for(int i=1; i<=6; i++) {
            table_x = i; 
            const char* s = expr_str.c_str();
            double val = eval_expr(s);
            canvas.setCursor(10, 30 + (i-1)*20); canvas.printf("x = %d   |   f(x) = %g", i, val);
        }
    }
}

// Mini Apps Renderers
void renderHeader(const char* title, uint16_t color) {
    canvas.fillSprite(TFT_WHITE);
    canvas.fillRect(0, 0, SCREEN_W, 25, color);
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1.5f);
    canvas.setCursor(5, 5);
    canvas.print(title);
}

void renderStat() {
    renderHeader("Statistics (1-Var)", display.color565(50, 150, 200));
    canvas.setTextColor(TFT_BLACK);
    for(int i=0; i<3; i++) {
        canvas.setCursor(10, 40 + i*25);
        canvas.printf("%c Data %d = %g", (stat_sel==i)?'>':' ', i+1, stat_data[i]);
    }
    float sum = stat_data[0]+stat_data[1]+stat_data[2];
    canvas.setCursor(150, 40); canvas.printf("Sum = %g", sum);
    canvas.setCursor(150, 65); canvas.printf("Mean = %g", sum/3.0);
}

void renderDist() {
    renderHeader("Distribution (Norm)", display.color565(100, 100, 200));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(10, 40); canvas.printf("> x = %g", dist_x);
    float pdf = (1.0 / sqrt(2 * M_PI)) * exp(-0.5 * dist_x * dist_x);
    canvas.setCursor(10, 80); canvas.printf("P(x) = %g", pdf);
}

void renderSpread() {
    renderHeader("Spreadsheet", display.color565(50, 200, 100));
    canvas.setTextColor(TFT_BLACK);
    for(int r=0; r<3; r++) {
        for(int c=0; c<3; c++) {
            int bx = 20 + c*90; int by = 40 + r*35;
            if(sheet_r==r && sheet_c==c) canvas.drawRect(bx-2, by-2, 85, 25, TFT_BLUE);
            canvas.setCursor(bx, by); canvas.printf("%g", sheet[r][c]);
        }
    }
    float sum = 0; for(int r=0; r<3; r++) for(int c=0; c<3; c++) sum += sheet[r][c];
    canvas.setCursor(20, 145); canvas.printf("SUM = %g", sum);
}

void renderEqn() {
    renderHeader("Equation (ax^2+bx+c=0)", display.color565(200, 50, 50));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(10, 40); canvas.printf("%c a = %g", (eqn_sel==0)?'>':' ', eqn_a);
    canvas.setCursor(10, 65); canvas.printf("%c b = %g", (eqn_sel==1)?'>':' ', eqn_b);
    canvas.setCursor(10, 90); canvas.printf("%c c = %g", (eqn_sel==2)?'>':' ', eqn_c);
    
    double delta = eqn_b*eqn_b - 4*eqn_a*eqn_c;
    canvas.setCursor(140, 50);
    if (delta < 0) { canvas.print("No Real Roots"); } 
    else {
        canvas.printf("x1 = %g", (-eqn_b + sqrt(delta)) / (2*eqn_a));
        canvas.setCursor(140, 75); canvas.printf("x2 = %g", (-eqn_b - sqrt(delta)) / (2*eqn_a));
    }
}

void renderIneq() {
    renderHeader("Inequality (ax+b > 0)", display.color565(200, 100, 50));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(10, 40); canvas.printf("%c a = %g", (ineq_sel==0)?'>':' ', ineq_a);
    canvas.setCursor(10, 65); canvas.printf("%c b = %g", (ineq_sel==1)?'>':' ', ineq_b);
    canvas.setCursor(10, 100);
    if(ineq_a == 0) canvas.print("No solution (a=0)");
    else {
        canvas.printf("x %s %g", ineq_a > 0 ? ">" : "<", -ineq_b/ineq_a);
    }
}

void renderComplex() {
    renderHeader("Complex (a+bi)", display.color565(150, 50, 150));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(10, 40); canvas.printf("%c a (Re) = %g", (cx_sel==0)?'>':' ', cx_a);
    canvas.setCursor(10, 65); canvas.printf("%c b (Im) = %g", (cx_sel==1)?'>':' ', cx_b);
    
    float mag = sqrt(cx_a*cx_a + cx_b*cx_b);
    float ang = atan2(cx_b, cx_a) * 180.0 / M_PI;
    canvas.setCursor(10, 110); canvas.printf("|z| = %g", mag);
    canvas.setCursor(10, 135); canvas.printf("arg(z) = %g deg", ang);
}

void renderBaseN() {
    renderHeader("Base-N (from Ans)", display.color565(100, 150, 50));
    canvas.setTextColor(TFT_BLACK);
    long val = (long)global_ans;
    canvas.setCursor(20, 50); canvas.printf("DEC : %ld", val);
    canvas.setCursor(20, 80); canvas.printf("HEX : %lX", val);
    canvas.setCursor(20, 110); canvas.printf("OCT : %lo", val);
}

void renderMatrix() {
    renderHeader("Matrix (2x2)", display.color565(200, 50, 150));
    canvas.setTextColor(TFT_BLACK);
    for(int r=0; r<2; r++) {
        for(int c=0; c<2; c++) {
            int bx = 20 + c*70; int by = 40 + r*40;
            if(mat_r==r && mat_c==c) canvas.drawRect(bx-2, by-2, 60, 25, TFT_BLUE);
            canvas.setCursor(bx, by); canvas.printf("%g", mat[r][c]);
        }
    }
    float det = mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
    canvas.setCursor(180, 60); canvas.printf("Det = %g", det);
}

void renderVector() {
    renderHeader("Vector (3D)", display.color565(150, 150, 50));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(10, 40); canvas.printf("%c x = %g", (vec_sel==0)?'>':' ', vec_x);
    canvas.setCursor(10, 65); canvas.printf("%c y = %g", (vec_sel==1)?'>':' ', vec_y);
    canvas.setCursor(10, 90); canvas.printf("%c z = %g", (vec_sel==2)?'>':' ', vec_z);
    
    float len = sqrt(vec_x*vec_x + vec_y*vec_y + vec_z*vec_z);
    canvas.setCursor(140, 65); canvas.printf("|v| = %g", len);
}

void renderRatio() {
    renderHeader("Ratio (A:B=X:D)", display.color565(150, 50, 150));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(10, 40); canvas.printf("%c A = %g", (ratio_sel==0)?'>':' ', ratio_a);
    canvas.setCursor(10, 65); canvas.printf("%c B = %g", (ratio_sel==1)?'>':' ', ratio_b);
    canvas.setCursor(10, 90); canvas.printf("%c D = %g", (ratio_sel==2)?'>':' ', ratio_d);
    
    if (ratio_b != 0) {
        canvas.setCursor(140, 65); canvas.printf("X = %g", (ratio_a*ratio_d)/ratio_b);
    }
}

void renderMathBox() {
    renderHeader("Math Box", display.color565(50, 150, 50));
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(20, 50); canvas.printf("%c Dice Roll", (mathbox_sel==0)?'>':' ');
    canvas.setCursor(20, 80); canvas.printf("%c Coin Toss", (mathbox_sel==1)?'>':' ');
    canvas.setCursor(160, 50); if(dice_val>0) canvas.printf("Result: %d", dice_val);
    canvas.setCursor(160, 80); if(coin_val!=-1) canvas.print(coin_val==0?"Result: HEADS":"Result: TAILS");
}

void renderSettings() {
    renderHeader("Settings", TFT_DARKGREY);
    canvas.setTextColor(TFT_BLACK);
    canvas.setCursor(20, 50); canvas.print("> Angle Unit: ");
    canvas.setTextColor(TFT_BLUE); canvas.print(deg_mode ? "Degree" : "Radian");
}

void setup() {
    Serial.begin(115200);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_CENTER, INPUT_PULLUP);
    
    display.init();
    display.setRotation(1); // Landscape
    display.setBrightness(255);
    canvas.createSprite(SCREEN_W, SCREEN_H);
}

unsigned long last_press = 0;

void loop() {
    unsigned long now = millis();
    bool changed = false;
    
    if (now - last_press > 150) {
        bool u = btnDown(BTN_UP);
        bool d = btnDown(BTN_DOWN);
        bool l = btnDown(BTN_LEFT);
        bool r = btnDown(BTN_RIGHT);
        bool c = btnDown(BTN_CENTER);
        
        // Universal Menu shortcut (Left + Center)
        if (l && c) {
            current_app = APP_HOME;
            last_press = now + 300;
            changed = true;
            return;
        }
        
        if (u || d || l || r || c) {
            last_press = now;
            changed = true;
            
            if (current_app == APP_HOME) {
                if (r && (home_sel % 3) < 2 && home_sel < NUM_APPS - 1) home_sel++;
                if (l && (home_sel % 3) > 0) home_sel--;
                if (d && home_sel + 3 < NUM_APPS) home_sel += 3;
                if (u && home_sel - 3 >= 0) home_sel -= 3;
                
                int r_sel = home_sel / 3;
                if (r_sel < home_scroll/3) home_scroll = r_sel * 3;
                if (r_sel >= (home_scroll/3) + 2) home_scroll = (r_sel - 1) * 3;
                
                if (c) {
                    if (home_sel == 0) { current_app = APP_CALC; kbd_x=0; kbd_y=1; expr_str=""; res_str=""; }
                    else if (home_sel == 1) current_app = APP_STAT;
                    else if (home_sel == 2) current_app = APP_DIST;
                    else if (home_sel == 3) current_app = APP_SPREAD;
                    else if (home_sel == 4) { current_app = APP_TABLE; kbd_x=0; kbd_y=1; expr_str=""; table_viewing=false; }
                    else if (home_sel == 5) current_app = APP_EQN;
                    else if (home_sel == 6) current_app = APP_INEQ;
                    else if (home_sel == 7) current_app = APP_COMPLEX;
                    else if (home_sel == 8) current_app = APP_BASEN;
                    else if (home_sel == 9) current_app = APP_MATRIX;
                    else if (home_sel == 10) current_app = APP_VECTOR;
                    else if (home_sel == 11) current_app = APP_RATIO;
                    else if (home_sel == 12) current_app = APP_MATHBOX;
                    else if (home_sel == 13) current_app = APP_SETTINGS;
                    else if (home_sel == 14) ESP.restart(); // Exit Mode!
                }
            } else if (current_app == APP_CALC || (current_app == APP_TABLE && !table_viewing)) {
                if (r && kbd_x < 7) { kbd_x++; while(kbd[kbd_y][kbd_x][0]=='\0' && kbd_x<7) kbd_x++; if(kbd[kbd_y][kbd_x][0]=='\0') kbd_x--; }
                if (l && kbd_x > 0) { kbd_x--; while(kbd[kbd_y][kbd_x][0]=='\0' && kbd_x>0) kbd_x--; }
                if (d && kbd_y < 4) { kbd_y++; if(kbd[kbd_y][kbd_x][0]=='\0') kbd_x=0; }
                if (u && kbd_y > 0) kbd_y--;
                
                if (c) {
                    std::string key = kbd[kbd_y][kbd_x];
                    if (key == "AC") { expr_str = ""; res_str = ""; }
                    else if (key == "DEL") { if (!expr_str.empty()) expr_str.pop_back(); }
                    else if (key == "=") {
                        if (current_app == APP_CALC) res_str = evaluate(expr_str);
                        else if (current_app == APP_TABLE) table_viewing = true;
                    } else if (key == "MENU") { current_app = APP_HOME; }
                    else { expr_str += key; }
                }
            } 
            // Simple generic navigators for the mini apps:
            else if (current_app == APP_STAT) { if(d&&stat_sel<2)stat_sel++; if(u&&stat_sel>0)stat_sel--; if(r)stat_data[stat_sel]++; if(l)stat_data[stat_sel]--; }
            else if (current_app == APP_DIST) { if(r)dist_x+=0.1; if(l)dist_x-=0.1; }
            else if (current_app == APP_SPREAD) { if(d&&sheet_r<2)sheet_r++; if(u&&sheet_r>0)sheet_r--; if(r)sheet[sheet_r][sheet_c]++; if(l)sheet[sheet_r][sheet_c]--; if(c){sheet_c++; if(sheet_c>2)sheet_c=0;} }
            else if (current_app == APP_EQN) { if(d&&eqn_sel<2)eqn_sel++; if(u&&eqn_sel>0)eqn_sel--; if(r){if(eqn_sel==0)eqn_a++;if(eqn_sel==1)eqn_b++;if(eqn_sel==2)eqn_c++;} if(l){if(eqn_sel==0)eqn_a--;if(eqn_sel==1)eqn_b--;if(eqn_sel==2)eqn_c--;} }
            else if (current_app == APP_INEQ) { if(d&&ineq_sel<1)ineq_sel++; if(u&&ineq_sel>0)ineq_sel--; if(r){if(ineq_sel==0)ineq_a++;if(ineq_sel==1)ineq_b++;} if(l){if(ineq_sel==0)ineq_a--;if(ineq_sel==1)ineq_b--;} }
            else if (current_app == APP_COMPLEX) { if(d&&cx_sel<1)cx_sel++; if(u&&cx_sel>0)cx_sel--; if(r){if(cx_sel==0)cx_a++;if(cx_sel==1)cx_b++;} if(l){if(cx_sel==0)cx_a--;if(cx_sel==1)cx_b--;} }
            else if (current_app == APP_MATRIX) { if(d&&mat_r<1)mat_r++; if(u&&mat_r>0)mat_r--; if(r)mat[mat_r][mat_c]++; if(l)mat[mat_r][mat_c]--; if(c){mat_c++; if(mat_c>1)mat_c=0;} }
            else if (current_app == APP_VECTOR) { if(d&&vec_sel<2)vec_sel++; if(u&&vec_sel>0)vec_sel--; if(r){if(vec_sel==0)vec_x++;if(vec_sel==1)vec_y++;if(vec_sel==2)vec_z++;} if(l){if(vec_sel==0)vec_x--;if(vec_sel==1)vec_y--;if(vec_sel==2)vec_z--;} }
            else if (current_app == APP_RATIO) { if(d&&ratio_sel<2)ratio_sel++; if(u&&ratio_sel>0)ratio_sel--; if(r){if(ratio_sel==0)ratio_a++;if(ratio_sel==1)ratio_b++;if(ratio_sel==2)ratio_d++;} if(l){if(ratio_sel==0)ratio_a--;if(ratio_sel==1)ratio_b--;if(ratio_sel==2)ratio_d--;} }
            else if (current_app == APP_MATHBOX) { if(d&&mathbox_sel<1)mathbox_sel++; if(u&&mathbox_sel>0)mathbox_sel--; if(c){if(mathbox_sel==0)dice_val=(rand()%6)+1; if(mathbox_sel==1)coin_val=(rand()%2);} }
            else if (current_app == APP_SETTINGS) { if (c) deg_mode = !deg_mode; }
            
            if (current_app != APP_HOME && current_app != APP_CALC && current_app != APP_TABLE) {
                // If they press MENU inside any of these apps without the universal shortcut, let's just make Center do stuff.
            }
        }
    }
    
    static unsigned long last_render = 0;
    if (changed || now - last_render > 50) {
        if (current_app == APP_HOME) renderHome();
        else if (current_app == APP_CALC) renderCalc();
        else if (current_app == APP_TABLE) renderTable();
        else if (current_app == APP_STAT) renderStat();
        else if (current_app == APP_DIST) renderDist();
        else if (current_app == APP_SPREAD) renderSpread();
        else if (current_app == APP_EQN) renderEqn();
        else if (current_app == APP_INEQ) renderIneq();
        else if (current_app == APP_COMPLEX) renderComplex();
        else if (current_app == APP_BASEN) renderBaseN();
        else if (current_app == APP_MATRIX) renderMatrix();
        else if (current_app == APP_VECTOR) renderVector();
        else if (current_app == APP_RATIO) renderRatio();
        else if (current_app == APP_MATHBOX) renderMathBox();
        else if (current_app == APP_SETTINGS) renderSettings();
        
        canvas.pushSprite(0, 0);
        last_render = now;
    }
}
