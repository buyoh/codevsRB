#pragma once
#include "Macro.h"

#if defined(_WIN32) || defined(_WIN64)
#define getc_x _getc_nolock
#define putc_x _putc_nolock
#elif defined(__GNUC__)
#define getc_x getc_unlocked
#define putc_x putc_unlocked
#else
#define getc_x getc
#define putc_x putc
#endif
#define isvisiblechar(c) (0x21<=(c)&&(c)<=0x7E)
class MaiScanner {
    FILE* fp_;
public:
    inline MaiScanner(FILE* fp) :fp_(fp) {}
    template<typename T> void input_integer(T& var) noexcept {
        var = 0; T sign = 1;
        int cc = getc_x(fp_);
        for (; cc < '0' || '9' < cc; cc = getc_x(fp_))
            if (cc == '-') sign = -1;
        for (; '0' <= cc && cc <= '9'; cc = getc_x(fp_))
            var = (var << 3) + (var << 1) + cc - '0';
        var = var * sign;
    }
    inline int c() noexcept { return getc_x(fp_); }
    inline MaiScanner& operator>>(int& var) noexcept { input_integer<int>(var); return *this; }
    inline MaiScanner& operator>>(long long& var) noexcept { input_integer<long long>(var); return *this; }
    inline MaiScanner& operator>>(string& var) {
        int cc = getc_x(fp_);
        for (; !isvisiblechar(cc); cc = getc_x(fp_));
        for (; isvisiblechar(cc); cc = getc_x(fp_))
            var.push_back(cc);
        return *this;
    }
    template<typename IT> inline void in(IT begin, IT end) { for (auto it = begin; it != end; ++it) *this >> *it; }
};
class MaiPrinter {
    FILE* fp_;
public:
    inline MaiPrinter(FILE* fp) :fp_(fp) {}
    template<typename T>
    void output_integer(T var) noexcept {
        if (var == 0) { putc_x('0', fp_); return; }
        if (var < 0)
            putc_x('-', fp_),
            var = -var;
        char stack[32]; int stack_p = 0;
        while (var)
            stack[stack_p++] = '0' + (var % 10),
            var /= 10;
        while (stack_p)
            putc_x(stack[--stack_p], fp_);
    }
    inline MaiPrinter& operator<<(char c) noexcept { putc_x(c, fp_); return *this; }
    inline MaiPrinter& operator<<(int var) noexcept { output_integer<int>(var); return *this; }
    inline MaiPrinter& operator<<(long long var) noexcept { output_integer<long long>(var); return *this; }
    inline MaiPrinter& operator<<(char* str_p) noexcept { while (*str_p) putc_x(*(str_p++), fp_); return *this; }
    inline MaiPrinter& operator<<(const string& str) {
        const char* p = str.c_str();
        const char* l = p + str.size();
        while (p < l) putc_x(*p++, fp_);
        return *this;
    }
    template<typename IT> void join(IT begin, IT end, char sep = ' ') { for (bool b = 0; begin != end; ++begin, b = 1) b ? *this << sep << *begin : *this << *begin; }
};
MaiScanner scanner(stdin);
MaiPrinter printer(stdout);
