/*
 * @Author: h3n4l
 * @Date: 2022-02-05 08:13:39
 * @LastEditors: h3n4l
 * @LastEditTime: 2022-02-09 12:14:04
 * @FilePath: /CS144-sponge-2022/libsponge/tcp_sender.cc
 */
#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

#include <assert.h>
#include <iostream>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before
//! retransmitting the oldest outstanding segment \param[in] fixed_isn the
//! Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout,
                     const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()})),
      _initial_retransmission_timeout{retx_timeout},
      _stream(capacity),
      _timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - _ackno; }

void TCPSender::fill_window() {
    cerr << "Calling fill_window()" << endl;
    assert(_next_seqno >= _ackno);
    uint64_t payload_size =
        _rwnd > (_next_seqno - _ackno) ? (_rwnd - (_next_seqno - _ackno)) : 0;
    if (_rwnd == 0 && _track.size() != 0) {
        return;
    }
    payload_size = _rwnd == 0 ? 1 : payload_size;
    for (;;) {
        TCPSegment segment{};
        TCPHeader &header = segment.header();
        // If connection had not been established yet, send SYN
        if (_next_seqno == 0) {
            header.syn = true;
            payload_size--;
        }
        Buffer buf{_stream.read(payload_size > TCPConfig::MAX_PAYLOAD_SIZE
                                    ? TCPConfig::MAX_PAYLOAD_SIZE
                                    : payload_size)};
        segment.payload() = buf;
        payload_size -= buf.size();
        // If eof, set FIN
        if (_stream.eof() && (!_FIN_send) && payload_size > 0) {
            header.fin = true;
            _FIN_send = true;
            payload_size--;
        }
        if (segment.length_in_sequence_space() != 0) {
            header.seqno = wrap(_next_seqno, _isn);
            _next_seqno += segment.length_in_sequence_space();
            // Push it in _segments_out
            _segments_out.push(segment);
            // Push it in _track
            _track.push(segment);
            // update_checkpoint
            _checkpoint += segment.length_in_sequence_space();
            // Active timer
            _timer.active();
        }
        if (segment.length_in_sequence_space() == 0) {
            break;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno,
                             const uint16_t window_size) {
    uint64_t abs_no = unwrap(ackno, _isn, _checkpoint);
    // Impossible ackno (beyond next seqno) or overdue ackno are ignored
    if (abs_no > _next_seqno || abs_no < _ackno) {
        return;
    }
    // Refresh timer when ack new data
    if (abs_no > _ackno) {
        _timer.reset(_initial_retransmission_timeout);
    }
    // Valid Ackno need update the _rwnd
    _rwnd = window_size;
    _ackno = abs_no;
    // Pop the acknowledgement segment
    for (; _track.size() > 0;) {
        auto old_segment = _track.front();
        if (unwrap(old_segment.header().seqno, _isn, _checkpoint) +
                old_segment.length_in_sequence_space() <=
            abs_no) {
            _track.pop();
            continue;
        }
        break;
    }
    if (_track.size() == 0) {
        // No more data need to track, banned the timer
        _timer.banned();
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call
//! to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (_timer.tick(ms_since_last_tick, _rwnd != 0)) {
        if (_track.size() == 0) {
            _timer.banned();
            _timer.reset(_initial_retransmission_timeout);
            return;
        }
        // retransmit first segment in _track
        auto segment = _track.front();
        _segments_out.push(segment);
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _timer.get_consecutive_retransmissions();
}

void TCPSender::send_empty_segment() {
    TCPSegment segment{};
    TCPHeader &header = segment.header();
    header.seqno = wrap(_next_seqno, _isn);
    // Push it in _segments_out
    _segments_out.push(segment);
}

Timer::Timer(size_t _init_rto) : _rto(_init_rto) {}

void Timer::double_rto() { _rto = (_rto << 1) > _rto ? (_rto << 1) : _rto; }

bool Timer::tick(const size_t ms_since_last_tick, bool need_double_rto) {
    if (!_active) {
        return false;
    }
    if (_ms + ms_since_last_tick < _ms || _ms + ms_since_last_tick >= _rto) {
        // overflow or overtime, return true and reset _ms
        _ms = 0;
        if (need_double_rto) {
            double_rto();
        }
        _consecutive_retransmissions++;
        return true;
    }
    _ms += ms_since_last_tick;
    return false;
}
