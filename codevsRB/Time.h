#pragma once

#include <chrono>

#define TIME std::chrono::system_clock::now()
#define MILLISEC(t) (std::chrono::duration_cast<std::chrono::milliseconds>(t).count())


// std::chrono::system_clock::time_point ttt;
// inline void tic() { ttt = TIME; }
// inline void toc() { clog << "TIME : " << MILLISEC(TIME - ttt) << "msec\n"; }
