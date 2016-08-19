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

int TestMomoImg::testAll(){

  int numTests = 0;

  numTests += testReadWrite();

  return numTests;
}


int TestMomoImg::testReadWrite(){

  cv::Mat imgA(30,40,CV_8U,25),                  // grayscale 30x40 image
          imgB(30,20,CV_8UC3, Scalar(20,30,40));  // color 30x20 image

  MomoImg testA(imgA),    
          testB(imgB),
          xA, xB, A_minus_B;



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
    
    
    

    // -2- test conversion to html, grayscale and color
    actual_.str(testA.toHTML());
    TestTools::report(actual_.str(), testA_html, "writing CV_8UC1 to html");

    actual_.str(testB.toHTML());
    TestTools::report(testB.toHTML(), testB_html, "writing CV_8UC3 to html jpeg");

    // -3- convert from html data, grayscale and color
    ///    REMARK: the converted images are *not* identical, but A_minus_B is minimal
    test_ = "reading CV_8UC1 from base64 string";
    xA.readFromString(dataA,"image/jpeg"); 
    A_minus_B.createFromMat(testA.img - xA.img);
    if (A_minus_B.getNumOnes() < 1000) { 
      TestTools::report("OK", "OK", test_);
    } else {
      TestTools::report(testA_html, xA.toHTML(), test_);
    }

    test_ = "reading CV_8UC3 from base64 string";
    xB.readFromString(dataB,"image/jpeg"); 
    A_minus_B.createFromMat(testB.img - xB.img);
    if (A_minus_B.getNumOnes() < 1000) { 
      TestTools::report("OK", "OK", test_);
    } else {
      TestTools::report(testA_html, xA.toHTML(), test_);
    }
    

    // -4- write to file

    // -5 read from file


  } 
  catch(std::string s) {
    std::cout << "Exception thrown in test"<< test_ <<":" << s << std::endl;
  }


  // return
  return 4;
}

