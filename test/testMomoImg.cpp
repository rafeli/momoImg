#include "testMomoImg.hpp"
#include <stddef.h>

MomoImg testA, testB;

TestMomoImg::TestMomoImg(){
   
   // -1- request logstream
   logStream = TestTools::getLogStream();

//    // -2- read both images
//    testA.readImage("testA.jpg");
//    testB.readImage("testB.jpg");

}

TestMomoImg::~TestMomoImg(){}

void TestMomoImg::testAll(){

  testReadWrite();

// TODO: testGetAngle()

}


void TestMomoImg::testReadWrite(){

  cv::Mat imgA(30,40,CV_8U,25),                  // grayscale 30x40 image
          imgB(30,20,CV_8UC3, Scalar(20,30,40));  // color 30x20 image

  MomoImg testA(imgA),    
          testB(imgB);



  try{

    // -1- make testImages a bit more interesting
    int numChannel = testB.img.channels();
    for (int row=0; row<testA.img.rows; row++) {
      uchar *dataA = testA.img.ptr<uchar>(row);
      for (int col=0; col<testA.img.cols; col++) {
          dataA[col] = 256 * sin ((row*col % 360) / 360.0);
      }
    }
    for (int row=0; row<testB.img.rows; row++) {
      uchar *dataB = testB.img.ptr<uchar>(row);
      for (int col=0; col<testB.img.cols; col++) {
        for (int channel=0; channel<numChannel; channel++)
          dataB[col*numChannel + channel] = 256 * sin ((channel*row*col % 360) / 360.0);
      }
    }
    testA.label(Point(5,35), "R");
    testB.label(Point(5,15), "R");
    
    
    

    // -2- convert to html
    test_ = "MomoImg::toHTML()";
    actual_.str(testA.toHTML());
    expected_ = "????\n";
    TestTools::report(actual_.str(), testA_html, test_);

    actual_.str(testB.toHTML());
    expected_ = "????\n";
    TestTools::report(actual_.str(), testB_html, test_);

    // -3- convert from html

    // -4- write to file

    // -5 read from file


  } 
  catch(std::string s) {
    std::cout << "Exception thrown in test"<< test_ <<":" << s << std::endl;
  }
}

