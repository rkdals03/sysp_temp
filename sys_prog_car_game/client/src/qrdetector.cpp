#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

extern "C" {
    int start_qr_code_detection();
}

int start_qr_code_detection() {
    QRCodeDetector detector;

    Mat frame, gray;
    VideoCapture cap;
    int deviceID = 0;
    int apiID = cv::CAP_ANY;
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return 0;
    }

    while (1) {
        cap.read(frame);
        Mat resizedframe;
        resize(frame, resizedframe, Size(1000, 750));
        if (resizedframe.empty()) {
            break;
        }
        cvtColor(resizedframe, gray, COLOR_BGR2GRAY);
        vector<Point> points;
        String info = detector.detectAndDecode(resizedframe, points);
        if (!info.empty()) {
            return stoi(info);
        }
    }
}