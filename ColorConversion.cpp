#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char** argv) {
 
  if(argc != 1) {
    cout << argv[0] << ": no arguments" << endl ;
    return(-1);
  }
  
  // create the image as a rows x cols x 3 float matrix
  int colorValues[4][4][3] = {
    {{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
    {{255,0,0},{255,0,0},{255,0,0},{255,0,0}},
    {{100,100,100},{100,100,100},{100,100,100},{100,100,100}},
    {{0,100,100},{0,100,100},{0,100,100},{0,100,100}}
  };

  int rows = 4;
  int cols = 4;

  Mat R(rows, cols, CV_32F);
  Mat G(rows, cols, CV_32F);
  Mat B(rows, cols, CV_32F);

 
  for(int i = 0 ; i < rows ; i++)
    for(int j = 0 ; j < cols ; j++) {
      int r = colorValues[i][j][0];
      int g = colorValues[i][j][1];
      int b = colorValues[i][j][2];

      R.at<float>(i,j) = (float)r;
      G.at<float>(i,j) = (float)g;
      B.at<float>(i,j) = (float)b;
    }
  R *= 1/255.0;
  G *= 1/255.0;
  B *= 1/255.0;
 
  Mat rgbimage;
  Mat planes[] = {B, G, R};
  merge(planes, 3, rgbimage);
  cout << "RGB image=" << rgbimage << endl;

  Mat xyzimage(rgbimage);
  cvtColor(xyzimage, xyzimage, CV_BGR2XYZ);
  cout << "XYZ image =" << xyzimage << endl; 

  Mat luvimage(rgbimage);
  cvtColor(luvimage, luvimage, CV_BGR2Luv);
  vector<Mat> Luv_planes;
  split(luvimage, Luv_planes);
  Mat L = Luv_planes[0];
  Mat u = Luv_planes[1];
  Mat v = Luv_planes[2];
  
  cout << "L =" << L << endl; 
  cout << "u =" << u << endl; 
  cout << "v =" << v << endl; 

  waitKey(0); // Wait for a keystroke
  return(0);
}

