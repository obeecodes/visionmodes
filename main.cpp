#include <opencv2/opencv.hpp>
#include <cmath>
#include <string>
#include <vector>

float minMotion = 1.5f;
int mode = -1;

std::vector<std::pair<cv::Rect, int>> buttons;
const std::vector<std::string> modeLabels = {
    "Grayscale", "Sobel X", "Sobel Y", "Blur",
    "Optical Flow", "Canny", "Laplacian", "Threshold",
    "Sobel Mag", "Blue Mask"
};

cv::Mat updateMode(const cv::Mat& frame, const cv::Mat& gray, cv::Mat& prevGray, int mode);
void onMouse(int event, int x, int y, int, void*);
void drawButtons(cv::Mat& output);

int main() {
    // initialize camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Camera error.\n";
        return -1;
    }

    cv::namedWindow("Vision Modes", cv::WINDOW_NORMAL);
    cv::setMouseCallback("Vision Modes", onMouse);

    // store frame data
    cv::Mat prevFrame, prevGray;
    cap >> prevFrame;
    if (prevFrame.empty()) return -1;
    cv::resize(prevFrame, prevFrame, cv::Size(1280, 960));
    cv::cvtColor(prevFrame, prevGray, cv::COLOR_BGR2GRAY);

    // main loop
    while (true) {

        cv::Mat frame, gray;
        cap >> frame;
        if (frame.empty()) break;
        cv::resize(frame, frame, cv::Size(1280, 960));
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // update frame
        cv::Mat output = updateMode(frame, gray, prevGray, mode);

        drawButtons(output);
        cv::imshow("Vision Modes", output);

        // close program with escape key
        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    // close program
    cap.release();
    cv::destroyAllWindows();
    return 0;
}

// convert angle to color
cv::Scalar angleToColor(float angleDegrees) {
    angleDegrees += 360;
    if(angleDegrees > 360){
        angleDegrees -=360;
    }
    float normalized = angleDegrees / 360.0f;
    int r = static_cast<int>(255 * std::fabs(std::sin(normalized * CV_PI)));
    int g = static_cast<int>(255 * std::fabs(std::sin(normalized * 2 * CV_PI)));
    int b = static_cast<int>(255 * std::fabs(std::cos(normalized * CV_PI)));
    return cv::Scalar(b, g, r);
}

cv::Mat updateMode(const cv::Mat& frame, const cv::Mat& gray, cv::Mat& prevGray, int mode) {
    cv::Mat output;
    switch (mode) {
        case 0:
            output = gray;
            break;
        case 1:
            cv::Sobel(gray, output, CV_16S, 1, 0);
            cv::convertScaleAbs(output, output);
            break;
        case 2:
            cv::Sobel(gray, output, CV_16S, 0, 1);
            cv::convertScaleAbs(output, output);
            break;
        case 3:
            cv::GaussianBlur(gray, output, cv::Size(21, 21), 5);
            break;
        case 4: {
            cv::Mat flow;
            cv::calcOpticalFlowFarneback(prevGray, gray, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
            output = frame.clone();
            cv::parallel_for_(cv::Range(0, output.rows), [&](const cv::Range& range) {
                for (int y = range.start; y < range.end; y += 15) {
                    for (int x = 0; x < output.cols; x += 15) {
                        cv::Point2f f = flow.at<cv::Point2f>(y, x);
                        float mag = std::sqrt(f.x * f.x + f.y * f.y);
                        if (mag < minMotion) continue;
                        float angle = std::atan2(f.y, f.x) * 180 / CV_PI;
                        cv::Scalar color = angleToColor(angle);
                        cv::arrowedLine(output, cv::Point(x, y), cv::Point(cvRound(x + f.x), cvRound(y + f.y)), color, 1, cv::LINE_AA, 0, 0.3);
                    }
                }
            });            
            prevGray = gray.clone();
            break;
        }
        case 5:
            cv::Canny(gray, output, 50, 150);
            break;
        case 6:
            cv::Laplacian(gray, output, CV_16S);
            cv::convertScaleAbs(output, output);
            break;
        case 7:
            cv::threshold(gray, output, 100, 255, cv::THRESH_BINARY);
            break;
        case 8: {
            cv::Mat dx, dy, mag;
            cv::Sobel(gray, dx, CV_32F, 1, 0);
            cv::Sobel(gray, dy, CV_32F, 0, 1);
            cv::magnitude(dx, dy, mag);
            cv::normalize(mag, output, 0, 255, cv::NORM_MINMAX);
            output.convertTo(output, CV_8U);
            break;
        }
        case 9: {
            cv::Mat hsv;
            cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
            cv::inRange(hsv, cv::Scalar(100, 100, 100), cv::Scalar(130, 255, 255), output);
            break;
        }
        default:
            output = frame.clone();
    }
    return output;
}

void onMouse(int event, int x, int y, int, void*) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        for (const auto& [rect, m] : buttons) {
            if (rect.contains(cv::Point(x, y))) {
                mode = m;
                break;
            }
        }
    }
}

void drawButtons(cv::Mat& output) {
    buttons.clear();
    int bx = 10, by = 40, bw = 150, bh = 30;
    for (int i = 0; i < 10; ++i) {
        cv::Rect rect(bx, by + i * (bh + 5), bw, bh);
        buttons.push_back({rect, i});
        cv::rectangle(output, rect, (mode == i ? cv::Scalar(0, 255, 0) : cv::Scalar(200, 200, 200)), -1);
        cv::putText(output, modeLabels[i], rect.tl() + cv::Point(5, 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
    cv::putText(output, "Click buttons or press 0–9", cv::Point(10, 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1);
}

