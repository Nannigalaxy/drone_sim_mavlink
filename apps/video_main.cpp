#include <iostream>

#include "video/connector/rtsp.h"
#include "video/processor/frame_overlay.h"
#include "video/render/render.h"

int main() {
    const char *env = std::getenv("RTSP_URL");

    std::string rtsp_url = env ? env : "rtsp://127.0.0.1:8554/live";

    RtspConnector connector(rtsp_url);

    if (!connector.open()) {
        return -1;
    }

    FrameOverlay overlay;

    VideoVisualizer visualizer("RTSP Viewer");

    cv::Mat frame;

    while (true) {
        if (!connector.read(frame)) {
            std::cerr << "Failed to read frame\n";
            break;
        }

        cv::Mat processed = overlay.process(frame);

        visualizer.show(processed);

        if (visualizer.should_close()) {
            break;
        }
    }

    connector.close();

    return 0;
}