#pragma once

#include <opencv2/opencv.hpp>
#include <string>

class RtspConnector {
  public:
    explicit RtspConnector(const std::string &url);

    bool open();

    bool read(cv::Mat &frame);

    void close();

  private:
    std::string url_;
    cv::VideoCapture cap_;
};
