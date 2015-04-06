#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

#define uw (static_cast<double>(4 * 0.95 / (0.95 + 15 * 1.0 + 3 * 1.09)))
#define vw (static_cast<double>(9 * 1.0 / (0.95 + 15 * 1.0 + 3 * 1.09)))

#define invgamma(v) ((v < 0.03928) ? (v / 12.92) : (pow((v + 0.055) / 1.055, 2.4)))

#define gamma(D) ((D < 0.00304) ? (12.92 * D) : (1.055 * pow(D, 1.0 / 2.4) - 0.055))

void linear_scaling(Mat& L, int rows, int cols, double L_max, double L_min) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float _L = L.at<float>(i, j);

            if (_L < L_min) {
                _L = 0;
            } else if (_L > L_max) {
                _L = 100;
            } else {
                _L = (_L - L_min) * 100 / (L_max - L_min);
            }

            L.at<float>(i, j) = _L;
        }
    }
}

class SampleHistogram {
public:

    int *histogram;
    int map_length;
    int rows;
    int cols;
    float S_max;
    float S_min;

    SampleHistogram(int _rows, int _cols) {
        histogram = NULL;
        map_length = 0;
        rows = _rows;
        cols = _cols;
        S_max = -1;
        S_min = 256;
    }
    ~SampleHistogram() {
        if (histogram != NULL) {
            delete[] histogram;
            histogram = NULL;
        }
    }

    void init_histogram(int length) {
        map_length = length;
        histogram = new int[map_length];

        for (unsigned i = 0; i < map_length; ++i) {
            histogram[i] = 0;
        }
    }
};

const SampleHistogram& sample_window(Mat& S, int rows, int cols, int W1,
        int H1, int W2, int H2) {
    SampleHistogram *sample = new SampleHistogram(rows, cols);

    sample->init_histogram(255);

    // histogram
    for (int i = H1; i <= H2; i++) {
        for (int j = W1; j <= W2; j++) {
            float _S = S.at<float>(i, j);

            sample->S_max = max(_S, sample->S_max);
            sample->S_min = min(_S, sample->S_min);

            sample->histogram[static_cast<int>(_S)]++;
        }
    }

    int L_max_int = static_cast<int>(sample->S_max);
    int L_min_int = static_cast<int>(sample->S_min);
    int map_length = L_max_int - L_min_int + 1;

    sample->map_length = map_length;

    return *sample;
}

int main(int argc, char** argv) {

    if (argc != 7) {
        cerr << argv[0] << ": " << "got " << argc - 1
                << " arguments. Expecting six: w1 h1 w2 h2 ImageIn ImageOut."
                << endl;
        cerr << "Example: proj1 0.2 0.1 0.8 0.5 fruits.jpg out.bmp" << endl;
        return (-1);
    }

    double w1 = atof(argv[1]);
    double h1 = atof(argv[2]);
    double w2 = atof(argv[3]);
    double h2 = atof(argv[4]);
    char *inputName = argv[5];
    char *outputName = argv[6];

    if (w1 < 0 || h1 < 0 || w2 <= w1 || h2 <= h1 || w2 > 1 || h2 > 1) {
        cerr << " arguments must satisfy 0 <= w1 < w2 <= 1"
                << " ,  0 <= h1 < h2 <= 1" << endl;
        return (-1);
    }

    Mat inputImage = imread(inputName, CV_LOAD_IMAGE_UNCHANGED); // Read the image
    if (inputImage.empty()) {
        cerr << "Could not open or find the image " << argv[1] << endl;
        return (-1);
    }

    cout << "Image name: " << inputName << endl;
    imshow("input", inputImage);

    int rows = inputImage.rows;
    int cols = inputImage.cols;

    // window size
    int W1 = (int) (w1 * (cols - 1));
    int H1 = (int) (h1 * (rows - 1));
    int W2 = (int) (w2 * (cols - 1));
    int H2 = (int) (h2 * (rows - 1));

    // first method: access image pixel by pixel, save values in int[][][] array
    // this is not the recommended approach

    // dynamically allocate an array of size rows x cols x 3
    int *** colorValues = new int **[rows];
    for (int i = 0; i < rows; i++) {
        colorValues[i] = new int *[cols];
        for (int j = 0; j < cols; j++)
            colorValues[i][j] = new int[3];
    }

    // access in raster order, one pixel at a time
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
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
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                const int *pij = colorValues[i][j];
                cout << "(" << pij[0] << "," << pij[1] << "," << pij[2]
                        << "), ";
            }
            cout << endl;
        }
    }

    // delete the dynamically allocated arrays
    // delete color array
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
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
    double **X = new double *[rows];
    double **Y = new double *[rows];
    double **Z = new double *[rows];
    
    for (int i = 0; i < rows; i++) {
        X[i] = new double[cols];
        Y[i] = new double[cols];
        Z[i] = new double[cols];
    }

    Mat R(rows, cols, CV_32F);
    Mat G(rows, cols, CV_32F);
    Mat B(rows, cols, CV_32F);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            R.at<float>(i, j) = (float)red.at<uchar>(i, j);
            G.at<float>(i, j) = (float)green.at<uchar>(i, j);
            B.at<float>(i, j) = (float)blue.at<uchar>(i, j);
        }
    }
    
    R *= 1 / 255.0;
    G *= 1 / 255.0;
    B *= 1 / 255.0;

    Mat rgbimage;
    Mat planes2[] = {B, G, R};
    merge(planes2, 3, rgbimage);
    //cout << "RGB image=" << rgbimage << endl;

    // // RGB -> XYZ
    // Mat xyzimage(rgbimage);
    // cvtColor(xyzimage, xyzimage, CV_BGR2XYZ);
    // //cout << "XYZ image =" << xyzimage << endl;

    // RGB -> Luv
    Mat luvimage(rgbimage);
    cvtColor(luvimage, luvimage, CV_BGR2Luv);
    
    vector<Mat> Luv_planes;
    split(luvimage, Luv_planes);
    
    Mat L = Luv_planes[0];
    Mat u = Luv_planes[1];
    Mat v = Luv_planes[2];

    const SampleHistogram& sample = sample_window(L, rows, cols, W1, H1, W2, H2);

    linear_scaling(L, rows, cols, sample.S_max, sample.S_min);

    Mat luvImage3;
    Mat planes3[] = {L, u, v};
    merge(planes3, 3, luvImage3);

    Mat rgbImage3(luvImage3);
    cvtColor(rgbImage3, rgbImage3, CV_Luv2BGR);
    
    std::vector<Mat> RGB_planes3;
    split(rgbImage3, RGB_planes3);
    Mat B3 = RGB_planes3[0];
    Mat G3 = RGB_planes3[1];
    Mat R3 = RGB_planes3[2];

    // R3 *= 255;
    // G3 *= 255;
    // B3 *= 255;
    namedWindow("Luv Linear Scaling", CV_WINDOW_AUTOSIZE);
    imshow("Luv Linear Scaling", rgbImage3);
    imwrite(outputName, rgbImage3);
    
    waitKey(0); // Wait for a keystroke
    return (0);
}

