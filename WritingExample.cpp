#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char **argv) {

    if (argc != 1) {
        cout << argv[0] << ": no arguments" << endl ;
        return (-1);
    }

    // create a color image as a rows x cols x 3 int matrix
    int colorValues[4][4][3] = {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
        {{100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
        {{0, 100, 100}, {0, 100, 100}, {0, 100, 100}, {0, 100, 100}}
    };

    // create a gray image as a rows x cols int matrixgetTest
    int grayValues[4][4];
    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 2 ; j++)
            grayValues[i][j] = 100;
        for (int j = 2 ; j < 4 ; j++)
            grayValues[i][j] = 200;
    }

    int rows = 4;
    int cols = 4;
    Mat cimage(rows, cols, CV_8UC3); // standard color image
    Mat gimage(rows, cols, CV_8UC1); // standard gray image

    // first method: access image pixel by pixel
    // this is not the recommended approach

    // access in raster order, one pixel at a time
    for (int i = 0 ; i < rows ; i++)
        for (int j = 0 ; j < cols ; j++) {
            int r = colorValues[i][j][0];
            int g = colorValues[i][j][1];
            int b = colorValues[i][j][2];

            Vec<uchar, 3> cpixel (b, g, r);
            cimage.at<Vec3b>(i, j) = cpixel;
        }

    // write as bmp image
    imwrite("write1.bmp", cimage);

    // second method: color planes
    // this is the recommended approach

    Mat R(rows, cols, CV_8UC1);
    Mat G(rows, cols, CV_8UC1);
    Mat B(rows, cols, CV_8UC1);

    for (int i = 0 ; i < rows ; i++)
        for (int j = 0 ; j < cols ; j++) {
            int r = colorValues[i][j][0];
            int g = colorValues[i][j][1];
            int b = colorValues[i][j][2];

            R.at<uchar>(i, j) = r;
            G.at<uchar>(i, j) = g;
            B.at<uchar>(i, j) = b;
        }
    Mat cimage2;
    Mat planes[] = {B, G, R};
    merge(planes, 3, cimage2);

    // write as bmp image
    imwrite("write2.bmp", cimage);

    // gray level image
    Mat GRAY(rows, cols, CV_8UC1);
    for (int i = 0 ; i < rows ; i++)
        for (int j = 0 ; j < cols ; j++) {
            int gray = grayValues[i][j];
            GRAY.at<uchar>(i, j) = gray;
        }
    // write as bmp image
    imwrite("writeGray2.bmp", GRAY);

    waitKey(0); // Wait for a keystroke
    return (0);
}

