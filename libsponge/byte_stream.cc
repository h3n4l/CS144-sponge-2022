/*
 * @Author: h3n4l
 * @Date: 2022-01-15 12:52:05
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-01-19 15:27:48
 * @FilePath: /sponge/libsponge/byte_stream.cc
 */
#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

#include <iostream>  // For debug
using namespace std;

ByteStream::ByteStream(const size_t capacity) : cap(capacity), total_write(0), total_read(0), buffer({}), _end(false) {}

size_t ByteStream::write(const string &data) {
    DUMMY_CODE(data);
    if (this->buffer.size() == this->cap || this->error() || this->input_ended()) {
        return 0;
    }
    size_t n_write = this->remaining_capacity() >= data.size() ? data.size() : this->remaining_capacity();
    string write_contents = data.substr(0, n_write);
    buffer += write_contents;
    this->total_write += n_write;
    return n_write;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    DUMMY_CODE(len);
    return len >= this->buffer.size() ? this->buffer : this->buffer.substr(0, len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    DUMMY_CODE(len);
    size_t clear_sz = this->buffer.size() > len ? len : this->buffer.size();
    this->total_read += clear_sz;
    this->buffer.erase(0, clear_sz);
    return;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    DUMMY_CODE(len);
    if (this->error())
        return "";
    string contents = this->peek_output(len);
    this->pop_output(len);
    return contents;
}

void ByteStream::end_input() { this->_end = true; }

bool ByteStream::input_ended() const { return this->_end; }

size_t ByteStream::buffer_size() const { return this->buffer.size(); }

bool ByteStream::buffer_empty() const { return this->buffer_size() == 0; }

bool ByteStream::eof() const { return this->buffer.size() == 0 && this->_end; }

size_t ByteStream::bytes_written() const { return this->total_write; }

size_t ByteStream::bytes_read() const { return this->total_read; }

size_t ByteStream::remaining_capacity() const { return cap - this->buffer.size(); }
