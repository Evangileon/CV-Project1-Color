/**
 * Copyright 2015
 */

#include "opencv2/highgui.hpp"
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

const double EPS = 1.0e-8;

const double X_W = 0.95;
const double Y_W = 1.0;
const double Z_W = 1.09;


#define uw (static_cast<double>(4 * 0.95 / (0.95 + 15 * 1.0 + 3 * 1.09)))
#define vw (static_cast<double>(9 * 1.0 / (0.95 + 15 * 1.0 + 3 * 1.09)))

#define invgamma(v) ((v < 0.03928) ? (v / 12.92) : (pow((v + 0.055) / 1.055, 2.4)))

#define gamma(D) ((D < 0.00304) ? (12.92 * D) : (1.055 * pow(D, 1.0 / 2.4) - 0.055))

void linear_scaling(double **L, int rows, int cols, double L_max, double L_min) {
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            double _L = (L[i][j] - L_min) * 1.0 / (L_max - L_min);

            if (_L < 0) {
                _L = 0;
            }

            if (_L > 100) {
                _L = 100;
            }

            L[i][j] = _L;
        }
    }
}

int main(int argc, char **argv) {
    

    if (argc != 2) {
        cerr << argv[0] << ": "
             << "got " << argc - 1 << " arguments. Expecting one: an image."
             << endl;
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
    double **X = new double *[rows];
    double **Y = new double *[rows];
    double **Z = new double *[rows];

    for (int i = 0 ; i < rows ; i++) {
        R[i] = new int[cols];
        G[i] = new int[cols];
        B[i] = new int[cols];
        X[i] = new double[cols];
        Y[i] = new double[cols];
        Z[i] = new double[cols];
    }

    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            R[i][j] = red.at<uchar>(i, j);
            G[i][j] = green.at<uchar>(i, j);
            B[i][j] = blue.at<uchar>(i, j);
        }
    }

    // if the image is small, print RGB values
    // RGB -> XYZ
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            double _R = static_cast<double>(R[i][j]) / 255;
            double _G = static_cast<double>(G[i][j]) / 255;
            double _B = static_cast<double>(B[i][j]) / 255;
            
            _R = invgamma(_R);
            _G = invgamma(_G);
            _B = invgamma(_B);

            X[i][j] = 0.412453 * _R + 0.357580 * _G + 0.180423 * _B;
            Y[i][j] = 0.212671 * _R + 0.715160 * _G + 0.072169 * _B;
            Z[i][j] = 0.019334 * _R + 0.119193 * _G + 0.950227 * _B;
        }
    }

    double Y_max = -1;
    double Y_min = 1000;


    double **x = new double *[rows];
    double **y = new double *[rows];
    for (int i = 0 ; i < rows ; i++) {
        x[i] = new double[cols];
        y[i] = new double[cols];
    }

    
    // XYZ -> xyY
    for(unsigned i = 0; i < rows; ++i) {
        for(unsigned j = 0; j < cols; ++j) {
            double _X = X[i][j];
            double _Y = Y[i][j];
            double _Z = Z[i][j];

            x[i][j] = _X / (_X + _Y + _Z);
            y[i][j] = _Y / (_X + _Y + _Z);

            Y_max = max(_Y, Y_max);
            Y_min = min(_Y, Y_min);
        }
    }

    // linear scaling on Y
    linear_scaling(Y, rows, cols, Y_max, Y_min);

    // transform back to RGB
    Mat R2(rows, cols, CV_8UC1);
    Mat G2(rows, cols, CV_8UC1);
    Mat B2(rows, cols, CV_8UC1);


    double **X2 = new double *[rows];
    double **Y2 = new double *[rows];
    double **Z2 = new double *[rows];
    for (int i = 0 ; i < rows ; i++) {
        X2[i] = new double[cols];
        Y2[i] = new double[cols];
        Z2[i] = new double[cols];
    }


    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {

            // xyY -> XYZ
            double _x = x[i][j];
            double _y = y[i][j];
            double _Y = Y[i][j];
            
            double _X2 = _x * _Y / _y;
            double _Y2 = _Y;
            double _Z2 = (1 - _x - _y) * _Y / _y;
            //cout << "Y = " << _Y2 << endl;

            // XYZ -> sRGB
            double _R2 = 3.240479 * _X2 - 1.53715 * _Y2 - 0.498535 * _Z2;
            double _G2 = -0.969256 * _X2 + 1.875991 * _Y2 + 0.041556 * _Z2;
            double _B2 = 0.055648 * _X2 - 0.204043 * _Y2 +  1.057311 * _Z2;

            _R2 = gamma(_R2);
            _G2 = gamma(_G2);
            _B2 = gamma(_B2);

            // if (_R2 > 1 || _G2 > 1 || _B2 > 1) {
            //     cout << "Error here: R = " << _R2 << ", G = " << _G2 << ", B = " << _B2 << endl;
            // }

            int r2 = static_cast<int>(_R2 * 255);
            int g2 = static_cast<int>(_G2 * 255);
            int b2 = static_cast<int>(_B2 * 255);

            // matrix assign
            R2.at<uchar>(i, j) = r2 <= 255 ? r2 : 255;
            G2.at<uchar>(i, j) = g2 <= 255 ? g2 : 255;
            B2.at<uchar>(i, j) = b2 <= 255 ? b2 : 255;
        }
    }

    // delete arrays
    for (int i = 0 ; i < rows ; i++) {
        delete[] R[i];
        delete[] G[i];
        delete[] B[i];
        delete[] X[i];
        delete[] Y[i];
        delete[] Z[i];
    }
    delete[]R; delete[]G; delete[]B;
    delete[]X; delete[]Y; delete[]Z;

    // draw graph
    Mat Luv;
    Mat Luv_planes[] = {B2, G2, R2};
    merge(Luv_planes, 3, Luv);
    namedWindow("Luv Linear Scaling", CV_WINDOW_AUTOSIZE);
    imshow("Luv Linear Scaling", Luv);
    cin.get();
    waitKey(0);  // Wait for a keystroke
    return (0);
}
