#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char **argv) {

    if (argc != 2) {
        cerr << argv[0] << ": "
             << "got " << argc - 1 << " arguments. Expecting one: an image."
             << endl ;
        return (-1);
    }
    Mat inputImage = imread(argv[1], CV_LOAD_IMAGE_UNCHANGED);  // Read the image
    if (inputImage.empty()) {
        cerr <<  "Could not open or find the image " << argv[1] << endl ;
        return (-1);
    }

    cout << "Image name: " << argv[1] << endl;
    imshow("input", inputImage);

    int rows = inputImage.rows;
    int cols = inputImage.cols;


    if (inputImage.type() == CV_8UC3) { // standard bgr color image

        // first method: access image pixel by pixel, save values in int[][][] array
        // this is not the recommended approach

        // dynamically allocate an array of size rows x cols x 3
        int *** colorValues = new int **[rows];
        for (int i = 0 ; i < rows ; i++) {
            colorValues[i] = new int *[cols];
            for (int j = 0 ; j < cols ; j++)
                colorValues[i][j] = new int[3];
        }

        // access in raster order, one pixel at a time
        for (int i = 0 ; i < rows ; i++) {
            for (int j = 0 ; j < cols ; j++) {
                Vec3b cpixel = inputImage.at<Vec3b>(i, j);
                uchar blue = cpixel.val[0];
                uchar green = cpixel.val[1];
                uchar red = cpixel.val[2];

                colorValues[i][j][0] = red;
                colorValues[i][j][1] = green;
                colorValues[i][j][2] = blue;
            }
        }

        // if the image is small, print color values
        if (rows * cols < 1000) {
            cout << "color values " << endl;
            for (int i = 0 ; i < rows ; i++) {
                for (int j = 0 ; j < cols ; j++) {
                    const int *pij = colorValues[i][j];
                    cout << "("
                         << pij[0] << "," << pij[1] << "," << pij[2]
                         << "), ";
                }
                cout << endl;
            }
        }

        // delete the dynamically allocated arrays
        // delete color array
        for (int i = 0 ; i < rows ; i++) {
            for (int j = 0 ; j < cols ; j++)
                delete[] colorValues[i][j];
            delete[] colorValues[i];
        }
        delete[] colorValues;

        // second method: Split image into color planes. Save in int[][] R,G,B
        // this is the recommended approach

        vector<Mat> planes;
        split(inputImage, planes);
        Mat blue = planes[0];
        Mat green = planes[1];
        Mat red = planes[2];

        // dynamically allocate RGB arrays of size rows x cols
        int **R = new int *[rows];
        int **G = new int *[rows];
        int **B = new int *[rows];
        for (int i = 0 ; i < rows ; i++) {
            R[i] = new int[cols];
            G[i] = new int[cols];
            B[i] = new int[cols];
        }

        for (int i = 0 ; i < rows ; i++)
            for (int j = 0 ; j < cols ; j++) {
                R[i][j] = red.at<uchar>(i, j);
                G[i][j] = green.at<uchar>(i, j);
                B[i][j] = blue.at<uchar>(i, j);
            }

        // if the image is small, print RGB values
        if (rows * cols < 1000) {
            cout << "RGB values " << endl;
            for (int i = 0 ; i < rows ; i++) {
                for (int j = 0 ; j < cols ; j++)
                    cout << "("
                         << R[i][j] << "," << G[i][j] << "," << B[i][j]
                         << "), ";
                cout << endl;
            }
        }

        // delete arrays
        for (int i = 0 ; i < rows ; i++) {
            delete[] R[i];
            delete[] G[i];
            delete[] B[i];
        }
        delete[]R; delete[]G; delete[]B;

    } else if (inputImage.type() == CV_8UC1) { // standard gray image
        // access image pixel by pixel, save values in int[][] array
        // this is the recommended approach

        // dynamically allocate an array of size rows x cols
        int **grayValues = new int *[rows];
        for (int i = 0 ; i < rows ; i++)
            grayValues[i] = new int[cols];

        for (int i = 0 ; i < rows ; i++)
            for (int j = 0 ; j < cols ; j++)
                grayValues[i][j] = inputImage.at<uchar>(i, j);

        // delete arrays
        for (int i = 0 ; i < rows ; i++)
            delete[] grayValues[i];
        delete[] grayValues;

        cout << "gray values " << endl;
        for (int i = 0 ; i < rows ; i++) {
            for (int j = 0 ; j < cols ; j++)
                cout << grayValues[i][j] << " ";
            cout << endl;
        }
    } else {
        cerr <<  argv[1] << " is not a standard color/gray image" << endl ;
        return (-1);
    }

    waitKey(0); // Wait for a keystroke
    return (0);
}

