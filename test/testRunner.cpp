#include <iostream>
#include "testMomoImg.hpp"
#include "momo/logging.hpp"

int main(int argc, char ** argv) {

  std::string logFileName="test.log";

  Logging::prepare();

  // -1- open logfile
  if (argc==2) {
    logFileName = argv[1];
  }
  TestTools::prepare(logFileName);

  // -2- test each unit
  // -2.1- MomoImg
  TestMomoImg testMomoImg;
  testMomoImg.testAll();

  // -3- finalize
  TestTools::testingComplete();
  std::cout << "All tests finished \n";

}
