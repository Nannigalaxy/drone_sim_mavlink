#include "video/render/render.h"

VideoVisualizer::VideoVisualizer(const std::string &window_name)
    : window_name_(window_name) {
    cv::namedWindow(window_name_);
}

void VideoVisualizer::show(const cv::Mat &frame) {
    cv::imshow(window_name_, frame);
}

bool VideoVisualizer::should_close() {
    int key = cv::waitKey(1);

    return key == 'q' || key == 27;
}
