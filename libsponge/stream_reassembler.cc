/*
 * @Author: h3n4l
 * @Date: 2022-01-19 15:35:17
 * @LastEditors: h3n4l
 * @LastEditTime: 2022-01-28 20:25:27
 * @FilePath: /CS144-sponge-2022/libsponge/stream_reassembler.cc
 */
#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in
// `stream_reassembler.hh`
#include <assert.h>
#include <iostream>  // For debug
template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _unasm_bytes(0), _eof_idx(0), _expect_index(0), _set_eof(false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);
    node nd(data, index);
    if (_set_eof && nd._end_index > _eof_idx) {
        nd._data.erase(_eof_idx - nd._index, _eof_idx - nd._end_index);
        nd._end_index -= _eof_idx - nd._end_index;
    }
    // Overdue data
    if (nd._end_index < _expect_index) {
        return;
    }
    // Some Overdue data
    if (nd._index < _expect_index) {
        nd._data.erase(0, _expect_index - nd._index);
        nd._index = _expect_index;
    }
    // For the last test in fsm_stream_reassembler_cap.cc
    // if (nd._data.size() > _output.remaining_capacity()) {
    //     nd._data.erase(nd._data.size() - _output.remaining_capacity());
    //     nd._end_index = nd._index + nd._data.size();
    // }
    size_t max_index = _expect_index + _output.remaining_capacity();

    if (nd._end_index > max_index) {
        nd._data.erase(nd._data.begin() + (max_index - nd._index), nd._data.end());
        nd._end_index = nd._index + nd._data.size();
    } else {
        // Mount eof
        if (eof) {
            assert(nd._end_index == _eof_idx || _eof_idx == 0);
            _eof_idx = nd._end_index;
            _set_eof = true;
        }
    }
    // Try merge in list
    for (auto iter = _pad.begin(); iter != _pad.end();) {
        if (nd._index < iter->_index && nd._end_index >= iter->_index && nd._end_index < iter->_end_index) {
            // delete nd's overlap
            size_t delete_size = nd._end_index - iter->_index;
            iter->_data.erase(0, delete_size);
            iter->_index += delete_size;
            /* // delete iter's overlap
            // size_t delete_size = nd._end_index - iter->_index;
            // nd._data.erase(iter->_index - nd._index, delete_size);
            // nd._end_index -= delete_size; */
            break;
        }
        if (nd._index >= iter->_index && nd._end_index <= iter->_end_index) {
            nd._data.clear();
            nd._end_index = nd._index;
            break;
        }
        if (nd._index >= iter->_index && nd._index <= iter->_end_index && nd._end_index >= iter->_end_index) {
            size_t delete_size = iter->_end_index - nd._index;
            nd._data.erase(0, delete_size);
            nd._index = iter->_end_index;
            iter++;
            continue;
        }
        if (nd._index <= iter->_index && nd._end_index >= iter->_end_index) {
            // delete this node
            _unasm_bytes -= iter->_data.size();
            iter = _pad.erase(iter);
            // iter++;
            continue;
        }
        if (nd._end_index <= iter->_index) {
            break;
        }
        iter++;
    }
    if (nd._data.size() != 0) {
        for (auto iter = _pad.begin();; iter++) {
            if (nd._index <= iter->_index || iter == _pad.end()) {
                _pad.insert(iter, nd);
                break;
            }
        }
        _unasm_bytes += nd._data.size();
    }
    for (auto iter = _pad.begin(); iter != _pad.end();) {
        assert(iter->_index >= _expect_index);
        if (iter->_index == _expect_index) {
            /* // delete nd's overlap
            size_t max_write_bytes = 0;
            if (_set_eof) {
                max_write_bytes =
                    min(_eof_idx, iter->_end_index) - iter->_index;
            } else {
                max_write_bytes = iter->_data.size();
            }
            size_t success_bytes =
                _output.write(iter->_data.substr(0, max_write_bytes)); */

            // delete iter's overlap
            size_t success_bytes = _output.write(iter->_data);
            _unasm_bytes -= success_bytes;
            _expect_index += success_bytes;
            if (!(_expect_index <= _eof_idx || !_set_eof)) {
                cerr << _expect_index << "\t" << _eof_idx << endl;
            }
            assert(_expect_index <= _eof_idx || !_set_eof);
            assert(success_bytes == iter->_data.size());
            // if (success_bytes != iter->_data.size()) {
            //     iter->_data.erase(0, success_bytes);
            //     iter->_index += success_bytes;
            //     break;
            // } else {
            //     iter = _pad.erase(iter);
            // }
            iter = _pad.erase(iter);
            if (_expect_index == _eof_idx && _set_eof) {
                break;
            }
        } else {
            break;
        }
    }
    if (_expect_index == _eof_idx && _set_eof) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unasm_bytes; }

bool StreamReassembler::empty() const { return _pad.empty(); }
