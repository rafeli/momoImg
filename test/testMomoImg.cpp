#include "testMomoImg.hpp"
#include <stddef.h>

MomoImg testA, testB;

TestMomoImg::TestMomoImg(){
   
   // -1- request logstream
   logStream = TestTools::getLogStream();

   // -2- read both images
   testA.readImage("testA.jpg");
   testB.readImage("testB.jpg");
}

TestMomoImg::~TestMomoImg(){}

void TestMomoImg::testAll(){

  testRotate();

// TODO: testGetAngle()

}


void TestMomoImg::testRotate(){

  test_ = "MomoImg::rotate()";

  try{

    throw std::string("not implemented yet");
    expected_ = "????\n";
    actual_.str("");
    actual_ << "xxxx" ; 
    TestTools::report(actual_.str(), expected_, test_);

  } 
  catch(std::string s) {
    std::cout << "Exception thrown in test"<< test_ <<":" << s << std::endl;
  }
}

