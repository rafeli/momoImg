#ifndef _MOMOIMG_HPP
#define _MOMOIMG_HPP

#include <iostream>
#include <opencv2/core/core.hpp>                    // file handling
#include <opencv2/imgproc.hpp>                      //
#include <opencv2/imgproc/imgproc_c.h>              // CV_BGR2HSV
#include <opencv2/highgui/highgui.hpp>              // window management
//#include <opencv2/imgcodecs.hpp>                  //
#include <opencv2/imgcodecs/imgcodecs_c.h>          // CV_LOAD_IMAGE_UNCHANGED


#include "momo/logging.hpp"
#include "momo/systemCall.hpp"
#include "momo/binaryCoding.hpp"

#define MINP 100        // how many pixel required to get valid averages
#define MAXSIZE 1E5     // used in various calls, upper limit img.rows, img.cols

using namespace cv;

class MomoImg {

private:

  int centerRow, centerCol;


public:

  // todo: move to private
  Mat img;

  // constructor
  MomoImg();
  MomoImg(Mat);

  MomoImg clone();

  // reading writing (to file or string)

  // TODO: rename to readFromFile
  MomoImg readImage(std::string fileName);

  void saveImage(std::string);

  std::string toHTML();

  MomoImg readFromString(const std::string& data, const std::string type);

  MomoImg createFromMat(const cv::Mat&);

  void checkImage(std::string comment, std::string filename, std::string suffix);


  // METHODS GETTING (numeric) DATA ON IMG

  bool isValidPoint(Point p);

  Point getCenter() ; // sets baricentric coordinates

  long getNumOnes();

  int getMedian(long upToPixel, bool countRows);

  /// determine the angle of rodlike structure (center determined before)
  /// TODO: shouldnt that be highest EV of covarianz matrix ???
  double getAngle(bool horizontal);

  // METHODS TRANSFORMING a 0/1-image

  MomoImg selectShape(int hSize, int vSize, int repeat);

  MomoImg dilateShape(int hSize, int vSize, int repeat);

  MomoImg selectContrast(unsigned int size, bool horizontal);

  // set the pixel to one that have a value between the given
  // histogramm percentages
  MomoImg selectFromHistogram(double min, double max); 

  MomoImg selectCannyLines(unsigned int lowThreshold, unsigned int highThreshold);

  MomoImg selectRegion(unsigned int rowMin,unsigned int rowMax,unsigned int colMin,unsigned int colMax);

  MomoImg crop(unsigned int rowMin,unsigned int rowMax,unsigned int colMin,unsigned int colMax);


  // METHODS to turn rgb-image into a 0/1-  or grayscale- images

  /// get according to hsv values
  MomoImg selectHSV(int hMin, int hMax, int sMin, int sMax, int vMin, int vMax);

  MomoImg threshold(int channel, double limit, bool selectHigherThanLimit);

  /// transform rgb picture to either hue, saturation or value 
  MomoImg selectHSVChannel(unsigned int x);

  /// return a mask where the given Channel (h, s , v) exceeds a given threshold
  Mat getHSVMask(unsigned int channel, unsigned int threshold);

  /// select shape
  MomoImg selectShapeColor(int hSize, int vSize, int hsvChannel);


  // METHODS to edit an image

  /// add a second Image to a binary TODO: 
  MomoImg addImage(Mat img);
  MomoImg addImage(Mat img, unsigned int atRow, unsigned int atCol);

  /// rotate the image by angle
  MomoImg rotate(double angle);

  MomoImg setSize(unsigned int rows, unsigned int cols);

  // TODO: pure opencv method-wrappers should go to a opencv-wrapper
  MomoImg equalizeHist();
  MomoImg gaussianBlur(double sigmaX, double sigmaY);
  MomoImg medianBlur(const int kSize);
  MomoImg resize(double fx, double fy);
  MomoImg bitwise_not();
  std::vector<double> reduce(int dim, int rType);

  /// improve quality by subtracting x times the laplacian
  MomoImg blurMaskFilter(double rho, const int kSize);

  //// DRAWING functions
  MomoImg drawArrow(const Point , const Point, int thickness=0);

  MomoImg drawSquare(const Point , const Point);

  MomoImg label(const Point , std::string label);


};




#endif //MOMOIMG
