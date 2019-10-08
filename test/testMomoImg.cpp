#include "testMomoImg.hpp"
#include <stddef.h>


cv::Mat imgA(30,40,CV_8U,25),                   // grayscale 30x40 image, see Constructor
        imgB(30,20,CV_8UC3, cv::Scalar(20,30,40)),  // color 30x20 image, see Constructor
        imgC(10,20,CV_8U,25),                   // grayscale 10x20 image, homogeneous gray
        imgD(20,10,CV_8UC3, cv::Scalar(20,30,40));  // color 20x10 image, homoegeneous ?? color

MomoImg testA(imgA), testB(imgB), testC(imgC), testD(imgD);

TestMomoImg::TestMomoImg(){
   
   // -1- request logstream
   logStream = TestTools::getLogStream();

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
   testA.label(cv::Point(5,35), "R");
   testB.label(cv::Point(5,15), "R");

}

TestMomoImg::~TestMomoImg(){}

int TestMomoImg::testAll(){

  int numTests = 0;

  numTests += testReadWrite();

  numTests += testAddImage();

  return numTests;
}


int TestMomoImg::testAddImage(){


  try {
    MomoImg source, inset, check;
    int checkSum;
  
    // TODO: trying to add outside should throw
    source = testB.clone();
    inset = testD.clone();
    try{
      source.addImage(inset.img, 31,2);
      TestTools::report("OK", "OK", "addImage at row>nrows");
    } catch (std::string s) {
      TestTools::report("from addImage: cannot add at 31:2 in image of size 30:20", s, "addImage at row>nrows");
    }
    try{
      source.addImage(inset.img, 1,22);
      TestTools::report("OK", "OK", "addImage at row>nrows");
    } catch (std::string s) {
      TestTools::report("from addImage: cannot add at 1:22 in image of size 30:20", s, "addImage at col>ncols");
    }
 
    // check correct addition, first color images
    // TODO: check that source is unchanged outside inserted pic
    source = testB.clone();
    inset = testD.clone();
    source.addImage(inset.img, 1,2);
    check = source.crop(1,21,2,12);
    check.img -= inset.img;
    checkSum = check.getNumOnes();
    if (checkSum <10) {
      TestTools::report("OK", "OK", "addImage color, 10x10 into 20x30");
    } else {
      TestTools::report("NOT OK", "OK", "addImage color, 10x10 into 20x30");
    }
  
    // check correct addition, grayscale images
    source = testA.clone();
    inset = testC.clone();
    source.addImage(inset.img, 1,2);
    check = source.crop(1,11,2,22);
    check.img -= inset.img;
    checkSum = check.getNumOnes();
    if (checkSum <10) {
      TestTools::report("OK", "OK", "addImage grayscale, 10x10 into 20x30");
    } else {
      TestTools::report("NOT OK", "OK", "addImage grayscale, 10x10 into 20x30");
    }
  
    // return
    return 4;

  } catch (std::string s) {
    throw std::string(" in testAddImage: " + s);
  }

}

int TestMomoImg::testReadWrite(){


  MomoImg tA = testA.clone(),    
          tB = testB.clone(),
          xA, xB, A_minus_B;

  try{
    

    // -2- test conversion to html, grayscale and color
    actual_.str(tA.toHTML());
    TestTools::report(actual_.str(), testA_html, "writing CV_8UC1 to html");

    actual_.str(testB.toHTML());
    TestTools::report(testB.toHTML(), testB_html, "writing CV_8UC3 to html jpeg");

    // -3- convert from html data, grayscale and color
    ///    REMARK: the converted images are *not* identical, but A_minus_B is minimal
    test_ = "reading CV_8UC1 from base64 string";
    xA.readFromString(dataA,"image/jpeg"); 
    A_minus_B.createFromMat(tA.img - xA.img);
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

