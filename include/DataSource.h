#pragma once

#include <cstdint>
#include <vector>

class DataSource {
public:
    virtual ~DataSource() = default;

    virtual void open(bool enable) = 0;
    virtual uint32_t read(std::vector<uint8_t> &buffer) = 0;
};
