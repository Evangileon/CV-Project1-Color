#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

void runOnWindow(int W1, int H1, int W2, int H2, Mat inputImage, char *outName) {
    int rows = inputImage.rows;
    int cols = inputImage.cols;

    vector<Mat> i_planes;
    split(inputImage, i_planes);
    Mat iB = i_planes[0];
    Mat iG = i_planes[1];
    Mat iR = i_planes[2];

    // dynamically allocate RGB arrays of size rows x cols
    int **R = new int *[rows];
    int **G = new int *[rows];
    int **B = new int *[rows];
    
    for (int i = 0 ; i < rows ; i++) {
        R[i] = new int[cols];
        G[i] = new int[cols];
        B[i] = new int[cols];
    }

    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            R[i][j] = iR.at<uchar>(i, j);
            G[i][j] = iG.at<uchar>(i, j);
            B[i][j] = iB.at<uchar>(i, j);
        }
    }


    //       The transformation should be based on the
    //       historgram of the pixels in the W1,W2,H1,H2 range.
    //       The following code goes over these pixels

    for (int i = H1 ; i <= H2 ; i++) {
        for (int j = W1 ; j <= W2 ; j++) {
            double r = R[i][j];
            double g = G[i][j];
            double b = B[i][j];
            int gray = static_cast<int>(0.3 * r + 0.6 * g + 0.1 * b + 0.5);

            R[i][j] = G[i][j] = B[i][j] = gray;
        }
    }

    Mat oR(rows, cols, CV_8UC1);
    Mat oG(rows, cols, CV_8UC1);
    Mat oB(rows, cols, CV_8UC1);
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            oR.at<uchar>(i, j) = R[i][j];
            oG.at<uchar>(i, j) = G[i][j];
            oB.at<uchar>(i, j) = B[i][j];
        }
    }

    Mat o_planes[] = {oB, oG, oR};
    Mat outImage;
    merge(o_planes, 3, outImage);

    namedWindow("output", CV_WINDOW_AUTOSIZE);
    imshow("output", outImage);
    imwrite(outName, outImage);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        cerr << argv[0] << ": "
             << "got " << argc - 1
             << " arguments. Expecting six: w1 h1 w2 h2 ImageIn ImageOut."
             << endl ;
        cerr << "Example: proj1b 0.2 0.1 0.8 0.5 fruits.jpg out.bmp" << endl;
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

    Mat inputImage = imread(inputName, CV_LOAD_IMAGE_UNCHANGED);
    if (inputImage.empty()) {
        cout <<  "Could not open or find the image " << inputName << endl;
        return (-1);
    }

    string windowInput("input: ");
    windowInput += inputName;

    namedWindow(windowInput, CV_WINDOW_AUTOSIZE);
    imshow(windowInput, inputImage);

    if (inputImage.type() != CV_8UC3) {
        cout <<  inputName << " is not a standard color image  " << endl;
        return (-1);
    }

    int rows = inputImage.rows;
    int cols = inputImage.cols;
    int W1 = (int) (w1 * (cols - 1));
    int H1 = (int) (h1 * (rows - 1));
    int W2 = (int) (w2 * (cols - 1));
    int H2 = (int) (h2 * (rows - 1));

    runOnWindow(W1, H1, W2, H2, inputImage, outputName);

    waitKey(0); // Wait for a keystroke
    return (0);
}
