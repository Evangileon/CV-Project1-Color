#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char **argv) {

    Mat image = imread("fruits.jpg", CV_LOAD_IMAGE_UNCHANGED);  // Read the image
    if (image.empty()) { // invalid input
        cerr <<  "Couldn't open or find the image " << "fruits.jpg" << endl ;
        return (-1);
    }

    namedWindow("Display window", CV_WINDOW_AUTOSIZE); // create window
    imshow( "Display window", image); // Show image inside it.

    waitKey(0); // Wait for a keystroke
    destroyWindow("Display window");
}
