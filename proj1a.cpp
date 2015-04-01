

#include "opencv2/highgui.hpp"
#include <iostream>
#include <cmath>
using namespace cv;
using namespace std;


#define uw (static_cast<double>(4 * 0.95 / (0.95 + 15 * 1.0 + 3 * 1.09)))
#define vw (static_cast<double>(9 * 1.0 / (0.95 + 15 * 1.0 + 3 * 1.09)))

int main(int argc, char **argv) {
    const double EPS = 1.0e-8;

    if (argc != 3) {
        cout << argv[0] << ": "
             << "got " << argc - 1 << " arguments. Expecting two: width height."
             << endl;
        return -1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int **RED1 = new int *[height];
    int **GREEN1 = new int *[height];
    int **BLUE1 = new int *[height];
    int **RED2 = new int *[height];
    int **GREEN2 = new int *[height];
    int **BLUE2 = new int *[height];

    for (int i = 0 ; i < height ; i++) {
        RED1[i] = new int[width];
        GREEN1[i] = new int[width];
        BLUE1[i] = new int[width];
        RED2[i] = new int[width];
        GREEN2[i] = new int[width];
        BLUE2[i] = new int[width];
    }

    for (int i = 0 ; i < height ; i++) {
        for (int j = 0 ; j < width ; j++) {
            int r1, g1, b1;
            int r2, g2, b2;

            double x = (double)j / (double)width;
            double y = (double)i / (double)height;
            double Y = 1.0;

            double L = 90;
            double u = x * 512 - 255;
            double v = y * 512 - 255;


            /* Your code should be placed here
               It should translate xyY to byte sRGB
               and Luv to byte sRGB
            */

            double X1;
            double Y1 = 1.0;
            double Z1;

            X1 = x * Y1 / y;
            Z1 = ( 1.0 - x - y) * Y1 / y;

            // XYZ -> sRGB
            double R1 = 3.240479 * X1 - 1.53715 * Y1 - 0.498535 * Z1;
            double G1 = -0.969256 * X1 + 1.875991 * Y1 + 0.041556 * Z1;
            double B1 = 0.055648 * X1 - 0.204043 * Y1 +  1.057311 * Z1;

            r1 = static_cast<int>(R1 * 255);
            g1 = static_cast<int>(G1 * 255);
            b1 = static_cast<int>(B1 * 255);


            double X2;
            double Y2;
            double Z2;

            // Luv -> XYZ
            double u_prime = (u + 13 * uw * L) / (13 * L);
            double v_prime = (v + 13 * vw * L) / (13 * L);

            Y2 = pow(((L + 16) / 116), 3) * 1.0;
            X2 = Y2 * 2.25 * (u_prime / v_prime);
            Z2 = Y2 * (3 - 0.75 * u_prime - 5 * v_prime) / v_prime;

            // XYZ -> sRGB
            double R2 = 3.240479 * X2 - 1.53715 * Y2 - 0.498535 * Z2;
            double G2 = -0.969256 * X2 + 1.875991 * Y2 + 0.041556 * Z2;
            double B2 = 0.055648 * X2 - 0.204043 * Y2 +  1.057311 * Z2;

            r2 = static_cast<int>(R2 * 255);
            g2 = static_cast<int>(G2 * 255);
            b2 = static_cast<int>(B2 * 255);

            // this is the end of your code

            RED1[i][j] = r1;
            GREEN1[i][j] = g1;
            BLUE1[i][j] = b1;
            RED2[i][j] = r2;
            GREEN2[i][j] = g2;
            BLUE2[i][j] = b2;
        }
    }


    Mat R1(height, width, CV_8UC1);
    Mat G1(height, width, CV_8UC1);
    Mat B1(height, width, CV_8UC1);

    Mat R2(height, width, CV_8UC1);
    Mat G2(height, width, CV_8UC1);
    Mat B2(height, width, CV_8UC1);

    for (int i = 0 ; i < height ; i++) {
        for (int j = 0 ; j < width ; j++) {
            R1.at<uchar>(i, j) = RED1[i][j];
            G1.at<uchar>(i, j) = GREEN1[i][j];
            B1.at<uchar>(i, j) = BLUE1[i][j];

            R2.at<uchar>(i, j) = RED2[i][j];
            G2.at<uchar>(i, j) = GREEN2[i][j];
            B2.at<uchar>(i, j) = BLUE2[i][j];
        }
    }

    Mat xyY;
    Mat xyY_planes[] = {B1, G1, R1};
    merge(xyY_planes, 3, xyY);
    namedWindow("xyY", CV_WINDOW_AUTOSIZE);
    imshow("xyY", xyY);

    Mat Luv;
    Mat Luv_planes[] = {B2, G2, R2};
    merge(Luv_planes, 3, Luv);
    namedWindow("Luv", CV_WINDOW_AUTOSIZE);
    imshow("Luv", Luv);
    cin.get();
    waitKey(0);  // Wait for a keystroke
    return (0);
}
