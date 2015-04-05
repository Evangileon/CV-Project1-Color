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


void histogram_equalization(double **L, int rows, int cols, double L_max, double L_min) {
    // discretion conversion
    for(unsigned i = 0; i < rows; ++i) {
        for(unsigned j = 0; j < cols; ++j) {
            double _L = static_cast<int>(L[i][j]);

            if (_L < 0) {
                _L = 0;
            }

            if (_L > 100) {
                _L = 100;
            }

            L[i][j] = _L;
        }
    }

    int L_max_int = static_cast<int>(L_max);
    int L_min_int = static_cast<int>(L_min);

    int map_length = L_max_int - L_min_int + 1;
    int *histogram_map = new int[map_length];
    for(unsigned i = 0; i < map_length; ++i) {
        histogram_map[i] = 0;
    }

    int *histogram = new int[map_length];
    for(unsigned i = 0; i < map_length; ++i) {
        histogram[i] = 0;
    }

    // histogram
    for(unsigned i = 0; i < rows; ++i) {
        for(unsigned j = 0; j < cols; ++j) {
            double _L = L[i][j];
            if (_L > 100 || _L < 0) {
                continue;
            }

            histogram[static_cast<int>(_L)]++;
        }
    }

    // equalization
    int previous_f = 0;
    for(unsigned i = 0; i < map_length; ++i) {
        int histogram_f = previous_f + histogram[i];
        histogram_map[i] = (previous_f + histogram_f) * map_length / (2 * rows * cols);
        previous_f = histogram_f;
    }

    // map old value to equalized value
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            double _L = L[i][j];
            if (_L > 100 || _L < 0) {
                continue;
            }

            L[i][j] = histogram_map[static_cast<int>(_L)];
        }
    }

    delete[] histogram_map;
    delete[] histogram;
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
    double **L = new double *[rows];
    double **u = new double *[rows];
    double **v = new double *[rows];

    for (int i = 0 ; i < rows ; i++) {
        R[i] = new int[cols];
        G[i] = new int[cols];
        B[i] = new int[cols];
        X[i] = new double[cols];
        Y[i] = new double[cols];
        Z[i] = new double[cols];
        L[i] = new double[cols];
        u[i] = new double[cols];
        v[i] = new double[cols];
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

    double L_max = -1;
    double L_min = 1000;
    // XYZ -> Luv
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            double t = Y[i][j] / Y_W;
            double _L;

            if (t > 0.008856) {
                _L = 116 * pow(t, 1.0 / 3) - 16;
            } else {
                _L = 903.3 * t;
            }

            if (_L < 0) {
                _L = 0;
            }
            if (_L > 100) {
                cout << "L = " << _L << endl;
                //_L = 100;
            }

            L[i][j] = _L;
            L_min = min(_L, L_min);
            L_max = max(_L, L_max);

            double d = X[i][j] + 15 * Y[i][j] + 3 * Z[i][j];
            double u_prime = 4 * X[i][j] / d;
            double v_prime = 9 * Y[i][j] / d;
            u[i][j] = 13 * L[i][j] * (u_prime - uw);
            v[i][j] = 13 * L[i][j] * (v_prime - vw);
        }
    }

    // linear scaling on L
    histogram_equalization(L, rows, cols, L_max, L_min);

    // transform back to RGB
    Mat R2(rows, cols, CV_8UC1);
    Mat G2(rows, cols, CV_8UC1);
    Mat B2(rows, cols, CV_8UC1);

    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            double _L = L[i][j];
            double _u = u[i][j];
            double _v = v[i][j];
            double _X2, _Y2, _Z2;
        
            // Luv -> XYZ
            double u_prime = static_cast<double>(_u + 13 * uw * _L) / (13 * _L);
            double v_prime = static_cast<double>(_v + 13 * vw * _L) / (13 * _L);

            if (_L > 7.9996) {
                _Y2 = pow((static_cast<double>(_L + 16) / 116), 3) * Y_W;
            } else {
                _Y2 = _L * Y_W / 903.3;
            }

            //cout << "Y = " << _Y2 << endl;

            _X2 = _Y2 * 2.25 * (u_prime / v_prime);
            _Z2 = _Y2 * (3 - 0.75 * u_prime - 5 * v_prime) / v_prime;

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
        delete[] L[i];
        delete[] u[i];
        delete[] v[i];
    }
    delete[]R; delete[]G; delete[]B;
    delete[]X; delete[]Y; delete[]Z;
    delete[]L; delete[]u; delete[]v;

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
