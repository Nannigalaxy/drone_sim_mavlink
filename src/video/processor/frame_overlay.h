#pragma once

#include <opencv2/opencv.hpp>

class FrameOverlay {
  public:
    cv::Mat process(const cv::Mat &frame);
};
