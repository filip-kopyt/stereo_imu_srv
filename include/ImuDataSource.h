#pragma once

#include <unistd.h>
#include <string>
#include "DataSource.h"

using namespace std::string_literals;

class ImuDataSource : public DataSource {
public:
    virtual ~ImuDataSource() {
        if (-1 != fd_) {
            close(fd_);
            fd_ = -1;
        }
    }

    static ImuDataSource& getInstance() {
        static ImuDataSource instance;
        return instance;
    }

    virtual void open(bool enable);
    virtual uint32_t read(std::vector<uint8_t>& buffer);

private:
    ImuDataSource() : PATH{"/dev/stereo_imu"s}, fd_{-1}, buffer_(1024u), size_{0u} {}
    uint32_t read_(uint8_t* pBuffer, uint32_t size);

    const std::string PATH;
    int fd_;

    std::vector<uint8_t> buffer_;
    uint32_t size_;
};
