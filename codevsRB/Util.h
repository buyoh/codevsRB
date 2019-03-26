#pragma once
#include "Macro.h"


template<typename T1, typename T2>
struct Tag {

    T1 first; T2 second;

    Tag(T1 t1, T2 t2) :first(t1), second(t2) {}
    Tag(const pair<T1, T2>& p) :first(p.first), second(p.second) { }
    Tag(const pair<T1, T2>&& p) :first(p.first), second(p.second) { }

    inline bool operator == (const Tag& t) const {
        return first == t.first;
    }
    inline bool operator <(const Tag& t) const {
        return first < t.first;
    }
};


struct P {
    using T = int8_t;
    T y, x;

    P(T _y = 0, T _x = 0) :y(_y), x(_x) {}

    inline bool operator == (P p) const { return y == p.y && x == p.x; }
    inline bool operator < (P p) const { return y == p.y ? x < p.x : y < p.y; }
    inline P operator+(P p) const { return P(y + p.y, x + p.x); }
    inline P operator-(P p) const { return P(y - p.y, x - p.x); }
    inline P operator+=(P p) { y += p.y; x += p.x; return *this; }
    inline P operator-=(P p) { y -= p.y; x -= p.x; return *this; }
};
inline ostream& operator<<(ostream& os, P p) { os << '(' << p.y << ',' << p.x << ')'; return os; }

const P FourMoving[] = { P(-1,0),P(0,1), P(1,0), P(0,-1) };
const P FiveMoving[] = { P(-1,0),P(0,1), P(1,0), P(0,-1), P(0,0) };
const P EightMoving[] = { P(-1,0),P(0,1), P(1,0), P(0,-1), P(-1,-1), P(-1,1), P(1,-1), P(1,1) };

