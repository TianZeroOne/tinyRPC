#include <memory>
#include <string.h>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/net/tcp/tcp_buffer.h"

namespace tinyRPC {
TcpBuffer::TcpBuffer(int size) : size(size) {
    m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {

}

int TcpBuffer::readAble() {
    return m_write_index - m_read_index;
}

int TcpBuffer::writeAble() {
    return m_buffer.size() - m_write_index;
}

int TcpBuffer::readIndex() {
    return m_read_index;
}

int TcpBuffer::writeIndex() {
    return m_write_index;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
    if (size > writeAble()) {
        // 调整 buffer 的大小，扩容
        int new_size = (int) (1.5 * (m_write_index + size));
        resizeBuffer(new_size);
    }
    memcpy(&m_buffer[m_write_index], buf, size);
    m_write_index += size;

}

void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
    if (readAble() == 0) {
        return;
    }

    int read_size = readAble() > size ? size : readAble();

    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

    re.swap(tmp);
    m_read_index += read_size;

    adjustBuffer();
}

void TcpBuffer::resizeBuffer(int new_size) {
    std::vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());

    memcpy(&tmp[0], &m_buffer[m_read_index], count);
    m_buffer.swap(tmp);

    m_read_index = 0;
    m_write_index = m_read_index + count;

}

void TcpBuffer::adjustBuffer() {
    if (m_read_index < int(m_buffer.size() / 3)) {
        return;
    }
    std::vector<char> buffer(m_buffer.size());
    int count = readAble();

    memcpy(&buffer[0], &m_buffer[m_read_index], count);
    m_buffer.swap(buffer);
    m_read_index = 0;
    m_write_index = m_read_index + count;

}

void TcpBuffer::moveReadIndex(int size) {
    size_t j = m_read_index + size;
    if (j >= m_buffer.size()) {
        ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
        return;
    }
    m_read_index = j;

    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size) {
    size_t j = m_write_index + size;
    if (j >= m_buffer.size()) {
        ERRORLOG("moveWriteIndex error, invalid size %d, old_write_index %d, buffer size %d", size, m_write_index, m_buffer.size());
        return;
    }
    m_write_index = j;

}

}