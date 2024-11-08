#pragma once

#include <chrono>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>

using namespace std::string_literals;

class StereoCam {
public:
    StereoCam() : cams_{0u, 1u} {
        for (int i = 0; i < cams_.size(); i++) {
            if (!cams_[i]) {
                std::cout << "cams_["s << i << "] not opened"s << std::endl;
            }
        }
    }
    virtual ~StereoCam() = default;

    void process() {
        bool ready[2];

        ready[0] = cams_[0].capture();
        ready[1] = cams_[1].capture();

        if (ready[0]) {
            cams_[0].process();
        }
        if (ready[1]) {
            cams_[1].process();
        }

        if (ready[0] || ready[1]) {
            std::cout << ready[0] << " / " << ready[1] << ", diff="
                      << std::chrono::duration_cast<std::chrono::milliseconds>(cams_[0].data().first - cams_[1].data().first).count()
                      << std::endl;
        }
    }

private:
    class Cam {
    public:
        Cam(uint8_t sensorId) : cam_{pipeline(sensorId), cv::CAP_GSTREAMER} {}
        virtual ~Cam() {
            if (cam_.isOpened()) {
                cam_.release();
            }
        }

        bool operator!() { return !cam_.isOpened(); }

        bool capture() {
            if (cam_.read(mat_)) {
                ts_ = std::chrono::high_resolution_clock::now();
                return true;
            }
            return false;
        }

        void process() { cv::imencode(".png"s, mat_, buf_); }

        std::pair<const std::chrono::time_point<std::chrono::high_resolution_clock>&, const std::vector<uint8_t>&> data() {
            return {ts_, buf_};
        }

    private:
        std::string pipeline(uint8_t sensorId, uint32_t captureWidth = 1280u, uint32_t captureHeight = 720u, uint32_t displayWidth = 1280u,
                             uint32_t displayHeight = 720u, uint32_t frameRate = 2u, uint32_t flipMethod = 0u) {
            std::stringstream ss;

            // ss << "nvarguscamerasrc sensor-id="s << sensorId;
            // ss << " ! video/x-raw(memory:NVMM), width=(int)"s << captureWidth << ", height=(int)"s << captureHeight << ",
            // framerate=(fraction)"s
            //    << frameRate << "/1 ! "s;
            // ss << " ! nvvidconv flip-method="s << flipMethod;
            // ss << " ! video/x-raw, width=(int)"s << displayWidth << ", height=(int)"s << displayHeight << ", format=(string)BGRx"s;
            // ss << " ! videoconvert"s;
            // ss << " ! video/x-raw, format=(string)BGR"s;
            // ss << " ! appsink"s;

            // ss << "nvarguscamerasrc"s;
            ss << "nvarguscamerasrc sensor-id="s << static_cast<uint16_t>(sensorId);
            ss << " ! video/x-raw(memory:NVMM), width=(int)"s << captureWidth << ", height=(int)"s << captureHeight
               << ", framerate=(fraction)"s << static_cast<uint16_t>(frameRate) << "/1"s;
            ss << " ! nvvidconv flip-method="s << static_cast<uint16_t>(flipMethod);
            ss << " ! video/x-raw, width=(int)"s << displayWidth << ", height=(int)"s << displayHeight << ", format=(string)BGRx"s;
            ss << " ! videoconvert"s;
            ss << " ! video/x-raw, format=(string)BGR"s;
            ss << " ! appsink"s;

            return ss.str();
        }

        cv::VideoCapture cam_;
        cv::Mat mat_;
        std::chrono::time_point<std::chrono::high_resolution_clock> ts_;
        std::vector<uint8_t> buf_;
    };

    std::array<Cam, 2u> cams_;
};
