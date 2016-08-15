#ifndef _TESTMOMOIMG_HPP
#define _TESTMOMOIMG_HPP

#include "testTools.hpp"
#include "momo/logging.hpp"
#include "../src/momoImg.hpp"

class TestMomoImg {

private:

  std::ofstream *logStream;
  std::stringstream actual_;
  std::string test_,
            expected_ ;


public:

  TestMomoImg();

  ~TestMomoImg();

  void testAll();

  void testRotate();

  void testGetAngle();


};







#endif // _TESTMOMOIMG_HPP
