#pragma once

#include <array>
#include <chrono>
#include <sstream>
#include "Camera.h"
#include "DataSource.h"

using namespace std::string_literals;

class StereoDataSource : public DataSource {
public:
    StereoDataSource() : left_{0u}, right_{1u} {}
    virtual ~StereoDataSource() = default;

    static StereoDataSource& getInstance() {
        static StereoDataSource instance;
        return instance;
    }

    virtual void open(bool enable) {
        if (enable) {
            left_.valid = false;
            right_.valid = false;
        }
    }
    virtual uint32_t read(std::vector<uint8_t>& buffer) {
        uint32_t result = 0u;

        if (!left_.valid) {
            left_.valid = left_.camera.read(left_.ts, left_.data);
        }
        if (!right_.valid) {
            right_.valid = right_.camera.read(right_.ts, right_.data);
        }

        if (left_.valid && right_.valid) {
            uint32_t leftSize = left_.data.size();
            uint32_t rightSize = right_.data.size();

            std::vector<uint8_t> header;
            {
                std::stringstream ss;
                ss << std::setfill('0') << "ts="s << std::setw(11)
                   << std::chrono::duration_cast<std::chrono::milliseconds>(left_.ts.time_since_epoch()).count()
                   << ", type=DATA_STEREO, left="s << std::setw(8) << leftSize << ", right="s << std::setw(8) << rightSize << '\n';
                std::string s = ss.str();
                header = std::vector<uint8_t>(s.cbegin(), s.cend());
            }
            uint32_t headerSize = header.size();

            if ((headerSize + leftSize + rightSize) <= buffer.size()) {
                std::memcpy(&buffer[0], &header[0], headerSize);
                result += headerSize;

                std::memcpy(&buffer[result], &(left_.data[0]), leftSize);
                result += leftSize;

                std::memcpy(&buffer[result], &(right_.data[0]), rightSize);
                result += rightSize;

                left_.valid = false;
                right_.valid = false;
            }
        }

        return result;
    }

private:
    struct CameraMetadata {
        CameraMetadata(uint8_t sensorId) : camera{sensorId}, valid{false} {}

        Camera camera;

        bool valid;
        std::chrono::time_point<std::chrono::high_resolution_clock> ts;
        std::vector<uint8_t> data;
    };
    CameraMetadata left_, right_;
};
