#pragma once

#include <opencv2/opencv.hpp>
#include <string>

class VideoVisualizer {
  public:
    explicit VideoVisualizer(const std::string &window_name);

    void show(const cv::Mat &frame);

    bool should_close();

  private:
    std::string window_name_;
};
