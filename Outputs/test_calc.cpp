#include <iostream>
#include <string>
#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace std;

double ans = 0;
bool deg_mode = true;
double x_val = 0;

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
        s++;
        res = eval_expr(s);
        skip_ws(s);
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
        s += 3; res = ans;
    } else if (*s == 'x') {
        s += 1; res = x_val;
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
            // Implicit multiplication
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

int main() {
    const char* str = "3pi+2x";
    x_val = 5;
    cout << eval_expr(str) << endl; // expect 3*3.1415 + 10 = 19.4247
    
    const char* str2 = "2sin(30)";
    cout << eval_expr(str2) << endl; // expect 2 * 0.5 = 1
    
    return 0;
}
