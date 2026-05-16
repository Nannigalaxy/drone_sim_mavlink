#include "video/connector/rtsp.h"

#include <iostream>

RtspConnector::RtspConnector(const std::string &url) : url_(url) {}

bool RtspConnector::open() {
    cap_.open(url_, cv::CAP_FFMPEG);

    if (!cap_.isOpened()) {
        std::cerr << "Failed to open RTSP stream\n";
        return false;
    }

    return true;
}

bool RtspConnector::read(cv::Mat &frame) { return cap_.read(frame); }

void RtspConnector::close() {
    if (cap_.isOpened()) {
        cap_.release();
    }
}
