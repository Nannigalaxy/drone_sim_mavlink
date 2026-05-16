#include "video/processor/frame_overlay.h"

cv::Mat FrameOverlay::process(const cv::Mat &frame) {
    cv::Mat output;

    frame.copyTo(output);

    cv::putText(
        output,
        "RTSP STREAM",
        cv::Point(30, 50),
        cv::FONT_HERSHEY_SIMPLEX,
        1.0,
        cv::Scalar(0, 255, 0),
        2
    );

    return output;
}
