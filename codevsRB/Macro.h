#pragma once

// GCC
#pragma GCC optimize ("O3")


#ifndef _GLIBCXX_NO_ASSERT
#include <cassert>
#endif
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <ciso646>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <cfenv>
#include <cinttypes>
//#include <cstdalign>
//#include <cstdbool>
#include <cstdint>
//#include <ctgmath>
#include <cwchar>
#include <cwctype>

#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>


using namespace std;
using ll = long long int;

#define debugos cout
#define debug(v) {printf("L%d %s > ",__LINE__,#v);debugos<<(v)<<endl;}
#define debugv(v) {printf("L%d %s > ",__LINE__,#v);for(auto e:(v)){debugos<<e<<" ";}debugos<<endl;}
#define debuga(m,w) {printf("L%d %s > ",__LINE__,#m);for(int x=0;x<(w);x++){debugos<<(m)[x]<<" ";}debugos<<endl;}
#define debugaa(m,h,w) {printf("L%d %s >\n",__LINE__,#m);for(int y=0;y<(h);y++){for(int x=0;x<(w);x++){debugos<<(m)[y][x]<<" ";}debugos<<endl;}}
#define ALL(v) (v).begin(),(v).end()
#define repeat(cnt,l) for(auto cnt=decltype(l)();(cnt)<(l);++(cnt))
#define rrepeat(cnt,l) for(auto cnt=(l)-1;0<=(cnt);--(cnt))
#define iterate(cnt,b,e) for(auto cnt=(b);(cnt)!=(e);++(cnt))
#define diterate(cnt,b,e) for(auto cnt=(b);(cnt)!=(e);--(cnt))
const ll MD = 1000000007ll; const long double PI = 3.1415926535897932384626433832795L;
inline void assert_call(bool assertion, function<void()> f) { if (!assertion) { cerr << "assertion fault:" << endl; f(); abort(); } }
template<typename T1, typename T2> inline ostream& operator <<(ostream &o, const pair<T1, T2> p) { o << '(' << p.first << ':' << p.second << ')'; return o; }
template<typename Vec> inline ostream& _ostream_vecprint(ostream& os, const Vec& a) {
    os << '['; for (const auto& e : a) os << ' ' << e << ' '; os << ']'; return os;
}
template<typename T> inline ostream& operator<<(ostream& o, const vector<T>& v) { return _ostream_vecprint(o, v); }
template<typename T, size_t S> inline ostream& operator<<(ostream& o, const array<T, S>& v) { return _ostream_vecprint(o, v); }
template<typename T> inline T& chmax(T& to, const T& val) { return to = max(to, val); }
template<typename T> inline T& chmin(T& to, const T& val) { return to = min(to, val); }
void bye(string s, int code = 0) { cout << s << endl; exit(code); }
mt19937_64 randdev((random_device())());
template<typename T, typename Random = decltype(randdev), typename enable_if<is_integral<T>::value>::type* = nullptr>
inline T rand(T l, T h, Random& rand = randdev) { return uniform_int_distribution<T>(l, h)(rand); }
template<typename T, typename Random = decltype(randdev), typename enable_if<is_floating_point<T>::value>::type* = nullptr>
inline T rand(T l, T h, Random& rand = randdev) { return uniform_real_distribution<T>(l, h)(rand); }


