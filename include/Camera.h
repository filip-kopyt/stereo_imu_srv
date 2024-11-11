#pragma once

#include <chrono>
#include <mutex>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

using namespace std::string_literals;

class Camera {
public:
    Camera(uint8_t sensorId) : pipeline_(pipeline(sensorId)), vc_{pipeline_, cv::CAP_GSTREAMER}, run_{true} {
        if (!vc_.isOpened()) {
            throw std::runtime_error("Camera("s + std::to_string(sensorId) + ") failed, VideoCapture not opened, pipeline=\""s + pipeline_ +
                                     "\""s);
        }

        std::cout << pipeline_ << std::endl;

        thread_ = std::thread(&Camera::run, this);
    }
    virtual ~Camera() {
        run_ = false;
        thread_.join();
    }

    bool read(std::chrono::time_point<std::chrono::high_resolution_clock> &ts, std::vector<uint8_t> &buffer) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (0u != result_.second.size()) {
            ts = result_.first;
            result_.second.swap(buffer);
            result_.second.resize(0u);

            return true;
        }

        return false;
    }

private:
    static std::string pipeline(uint8_t sensorId, uint32_t captureWidth = 1280u, uint32_t captureHeight = 720u,
                                uint32_t displayWidth = 1280u, uint32_t displayHeight = 720u, uint32_t frameRate = 2u,
                                uint32_t flipMethod = 0u);
    void run() {
        std::chrono::time_point<std::chrono::high_resolution_clock> ts;
        cv::Mat mat0, mat1;
        std::vector<uint8_t> buffer;

        while (run_) {
            if (vc_.grab()) {
                ts = std::chrono::high_resolution_clock::now();
                if (vc_.retrieve(mat0)) {
                    cv::resize(mat0, mat1, cv::Size(), 0.5f, 0.5f, cv::INTER_AREA);
                    if (cv::imencode(".png"s, mat1, buffer)) {
                        std::lock_guard<std::mutex> lock(mutex_);
                        result_.first = ts;
                        result_.second.swap(buffer);

                    } else {
                        std::cerr << "Could not encode!"s << std::endl;
                    }
                } else {
                    std::cerr << "Could not retrieve!"s << std::endl;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    const std::string pipeline_;
    cv::VideoCapture vc_;

    bool run_;
    std::thread thread_;

    std::mutex mutex_;
    std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, std::vector<uint8_t>> result_;
};
