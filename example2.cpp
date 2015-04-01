#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    VideoCapture videocapture(0); // open the default camera
    if (!videocapture.isOpened()) {
        cerr <<  "Can't open default video camera" << endl ;
        return (-1);
    }
    namedWindow("video", CV_WINDOW_AUTOSIZE);

    Mat frame;
    bool showframe = true;
    while (showframe) {
        if (!videocapture.read(frame)) {
            cout <<  "Can't capture frame" << endl ;
            break;
        }
        imshow("video", frame);
        if (waitKey(30) >= 0) showframe = false;
    }
    return (0);
}
