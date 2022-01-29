/*
 * @Author: h3n4l
 * @Date: 2022-01-27 18:08:34
 * @LastEditors: h3n4l
 * @LastEditTime: 2022-01-28 22:22:15
 * @FilePath: /CS144-sponge-2022/libsponge/tcp_receiver.cc
 */
#include "tcp_receiver.hh"

// For debug
#include <assert.h>
#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    // Ignore data before SYN
    if (!_SYN_received && !header.syn) {
        return;
    }
    // Ignore duplicate SYN
    if (_SYN_received && header.syn) {
        return;
    }
    // Refer the last test case in recv_special.cc, it will receive more
    // segment even though it had received FIN segment.
    // // Ignore data after FIN
    // if (_FIN_received) {
    //     return;
    // }
    string payload = seg.payload().copy();
    WrappingInt32 payload_first_seqno = header.seqno;
    // Mount SYN
    if (header.syn) {
        _SYN_received = true;
        _isn = header.seqno;
        payload_first_seqno = header.seqno + 1;
    }
    // Mount EOF
    if (header.fin) {
        _FIN_received = true;
    }

    // Push in stream reassembler
    // In your TCP implementation, you’ll use the index of the last reassembled
    // byte as the checkpoint.
    uint64_t checkpoint = _reassembler.stream_out().bytes_written();
    uint64_t absolute_seqno_64 = unwrap(payload_first_seqno, _isn, checkpoint);
    if (absolute_seqno_64 == 0) {
        // Invalid stream index
        return;
    }
    // Stream_index = absolute_seqno_64 - 1
    _reassembler.push_substring(payload, absolute_seqno_64 - 1, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_SYN_received) {
        return nullopt;
    }
    uint64_t absolute_seqno = stream_out().bytes_written() + 1;
    // if (_FIN_received && _reassembler.empty()) {
    //     return wrap(absolute_seqno + 1, _isn);
    // }
    if (stream_out().input_ended()) {
        return wrap(absolute_seqno + 1, _isn);
    }
    return wrap(absolute_seqno, _isn);
}

size_t TCPReceiver::window_size() const {
    return stream_out().remaining_capacity();
}