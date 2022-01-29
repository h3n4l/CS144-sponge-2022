/*
 * @Author: h3n4l
 * @Date: 2022-01-27 18:08:34
 * @LastEditors: h3n4l
 * @LastEditTime: 2022-01-28 16:42:16
 * @FilePath: /CS144-sponge-2022/libsponge/wrapping_integers.cc
 */
#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    DUMMY_CODE(n, isn);
    if (n + isn.raw_value() < n) {
        n -= 0x1'0000'0000U;
    }
    n += isn.raw_value();
    n %= 0x1'0000'0000U;
    return WrappingInt32{uint32_t(n)};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    DUMMY_CODE(n, isn, checkpoint);
    const uint64_t max32 = 1UL << 32;
    uint64_t offset = n.raw_value() - isn.raw_value();
    if (checkpoint <= offset) {
        return offset;
    }
    uint64_t quotient = (checkpoint - offset) >> 32;
    uint64_t remainder = ((checkpoint - offset) << 32) >> 32;
    if (remainder <= max32 / 2) {
        return offset + (max32 * quotient);
    }
    return offset + (max32 * (quotient + 1));
    // uint64_t absolute_seqno_64 = n.raw_value() - isn.raw_value();
    // if (checkpoint <= absolute_seqno_64)  // 比较少见的情况
    //     return absolute_seqno_64;
    // else {
    //     uint64_t size_period = 1ul << 32, quotient, remainder;
    //     quotient = (checkpoint - absolute_seqno_64) >> 32;
    //     remainder = ((checkpoint - absolute_seqno_64) << 32) >> 32;
    //     if (remainder < size_period / 2)
    //         return absolute_seqno_64 + quotient * size_period;
    //     else
    //         return absolute_seqno_64 + (quotient + 1) * size_period;
    // }
}