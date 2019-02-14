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
#include "momo/json.hpp"

#define MINP 100        // how many pixel required to get valid averages
#define MAXSIZE 1E5     // used in various calls, upper limit img.rows, img.cols

class MomoImg {

private:

  int centerRow, centerCol;

  cv::Point js2Point(momo::jsValue p, std::string key);

  cv::Rect js2Rect(momo::jsValue r, std::string key);

public:

  // todo: move to private
  cv::Mat img;

  // constructor
  MomoImg();
  MomoImg(cv::Mat);
  MomoImg(int rows, int cols); // creates color image rows*cols

  MomoImg clone();

  // reading writing (to file or string)

  // TODO: rename to readFromFile
  MomoImg readImage(std::string fileName);

  void saveImage(std::string);

  std::string toHTML();
  std::string getImgData(std::string type);

  MomoImg readFromString(const std::string& data, const std::string type);

  MomoImg createFromMat(const cv::Mat&);

  void checkImage(std::string comment, std::string filename, std::string suffix);


  // METHODS GETTING (numeric) DATA ON IMG

  bool isValidPoint(cv::Point p);

  cv::Point getCenter() ; // sets baricentric coordinates

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

  MomoImg crop(momo::jsValue rect);
  MomoImg crop(int rowMin,int rowMax,int colMin,int colMax);


  // METHODS to turn rgb-image into a 0/1-  or grayscale- images

  /// get according to hsv values
  MomoImg selectHSV(int hMin, int hMax, int sMin, int sMax, int vMin, int vMax);

  MomoImg threshold(int channel, double limit, bool selectHigherThanLimit);

  /// transform rgb picture to either hue, saturation or value 
  MomoImg selectHSVChannel(unsigned int x);

  /// return a mask where the given Channel (h, s , v) exceeds a given threshold
  cv::Mat getHSVMask(unsigned int channel, unsigned int threshold);

  /// select shape
  MomoImg selectShapeColor(int hSize, int vSize, int hsvChannel);


  // METHODS to edit an image

  /// add a second Image to a binary TODO: 
  MomoImg addImage(cv::Mat img);
  MomoImg addImage(cv::Mat img, int atRow, int atCol);

  /// rotate the image by angle
  MomoImg rotate(momo::jsValue options);
  MomoImg rotate(double angle, double centerX=0.5, double centerY=0.5);

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
  MomoImg drawArrow(const cv::Point , const cv::Point, int thickness=0);

  MomoImg drawSquare(const cv::Point , const cv::Point);

  MomoImg label(const cv::Point , std::string label);


};




#endif //MOMOIMG
