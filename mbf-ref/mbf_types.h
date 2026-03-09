#pragma once

#include <cstdint>

typedef uint32_t mbf5;
typedef uint64_t mbf6;

#ifdef MBF_USE_BOOST

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::uint128_t mbf7;
typedef boost::multiprecision::uint256_t mbf8;
typedef boost::multiprecision::uint512_t mbf9;

const mbf7 F7_MAX = mbf7("0") - 1;
const mbf8 F8_MAX = mbf8("0") - 1;

#else

struct mbf7 { mbf6 f1; mbf6 f2; };
struct mbf8 { mbf7 f1; mbf7 f2; };
struct mbf9 { mbf8 f1; mbf8 f2; };

inline mbf7 operator~(const mbf7& a) { return {~a.f1, ~a.f2}; }
inline mbf7 operator|(const mbf7& a, const mbf7& b) { return {a.f1 | b.f1, a.f2 | b.f2}; }
inline mbf7 operator^(const mbf7& a, const mbf7& b) { return {a.f1 ^ b.f1, a.f2 ^ b.f2}; }
inline bool operator==(const mbf7& a, const mbf7& b) { return a.f1 == b.f1 && a.f2 == b.f2; }

inline mbf8 operator~(const mbf8& a) { return {~a.f1, ~a.f2}; }
inline mbf8 operator|(const mbf8& a, const mbf8& b) { return {a.f1 | b.f1, a.f2 | b.f2}; }
inline mbf8 operator^(const mbf8& a, const mbf8& b) { return {a.f1 ^ b.f1, a.f2 ^ b.f2}; }
inline bool operator==(const mbf8& a, const mbf8& b) { return a.f1 == b.f1 && a.f2 == b.f2; }

inline mbf9 operator~(const mbf9& a) { return {~a.f1, ~a.f2}; }
inline mbf9 operator|(const mbf9& a, const mbf9& b) { return {a.f1 | b.f1, a.f2 | b.f2}; }
inline bool operator==(const mbf9& a, const mbf9& b) { return a.f1 == b.f1 && a.f2 == b.f2; }

#endif

const mbf5 F5_MAX = (mbf5)(-1);
const mbf6 F6_MAX = (mbf6)(-1);
