#include <iostream>

#include "service.hpp"
#include "momo/binaryCoding.hpp"
#include "momo/http.hpp"
#include "momo/json.hpp"
#include "momoImg.hpp"

#define TEXTTYPE 0
#define INTTYPE 1
#define DBLTYPE 2

std::vector<momo::jsValue> images; // TODO rethink for multiuser design



//bool callback(MomoMessage request, int socket_fd) {
//
//
//    // -1.2- call requested method
//    const int64 startTime = cv::getTickCount();
//    } else if (mName == "convert") {
//
//     std::string  convName;
//     
//     try {
//       jsObject myConversion = parseBody(atob64(request.body));
//       std::string srcKey = myConversion.get("sourceImage").getString();
//       convName = myConversion.get("name").getString();
//
//       if (images.count(srcKey)>0) {
//         MomoImg srcImage = images[srcKey],
//                 result = srcImage.clone();
//         if (convName=="test") {
//           result = result.rotate(91,0.5,0.5)
//                    .crop(300,1630,400,2000)
//                    .resize(1.62,1.462) 
//                     ;
//
//           images["result"] = result;
//
//
//         } else if (convName=="bitwise_not") {
//           images["result"] = result.bitwise_not();
//         } else if (convName=="medianBlur") {
//           int kSize = myConversion.get("kSize").getInt();
//           images["result"] = result.medianBlur(kSize);
//         } else if (convName=="gaussianBlur") {
//           double sigmaX = myConversion.get("sigmaX").getDbl(),
//                  sigmaY = myConversion.get("sigmaY").getDbl();
//           images["result"] = result.gaussianBlur(sigmaX, sigmaY);
//         } else if (convName=="equalizeHist") {
//           images["result"] = result.equalizeHist();
//         } else if (convName=="rotate") {
//           double angle = myConversion.get("angle").getDbl(),
//                  centerX = myConversion.get("centerX").getDbl(),
//                  centerY = myConversion.get("centerY").getDbl();
//           images["result"] = result.rotate(angle, centerX, centerY);
//         } else if (convName=="resize") {
//           double fx = myConversion.get("fx").getDbl(),
//                  fy = myConversion.get("fy").getDbl();
//           images["result"] = result.resize(fx,fy);
//         } else if (convName=="blurMaskFilter") {
//           double rho = myConversion.get("rho").getDbl();
//           int kSize = myConversion.get("kSize").getInt();
//           images["result"] = result.blurMaskFilter(rho,kSize);
//         } else if (convName=="selectHSVChannel") {
//           int channel = myConversion.get("channel").getInt();
//           images["result"] = result.selectHSVChannel(channel);
//         } else if (convName=="reduce") {
//           int rType = myConversion.get("rType").getInt(),
//               dim = myConversion.get("dim").getInt();
//           std::vector<double> reduce_ = (Mat) result.reduce(dim, rType),
//                               baseLine;
//
//// //         hat mal funktioniert, aber nicht immer 
////            cv::dft(reduce_,reduce_);
//          
//           // folgendes funktioniert, liefert relativ gleichmaessige "sinus" 
//           cv::blur(reduce_, reduce_,Size(3,1));
//           cv::blur(reduce_, baseLine,Size(80,1));
//           reduce_ = (Mat) (((Mat) reduce_) - ((Mat) baseLine));
//           jsValue x(reduce_);
//           jsBody.add("plotData", x);
//           images["result"] = result;
//         } else if (convName=="threshold") {
//           int channel = myConversion.get("channel").getInt(),
//               isLowerLimit = myConversion.get("isLowerLimit").getInt();
//           double limit = myConversion.get("limit").getDbl();
//           images["result"] = result.threshold(channel, limit, isLowerLimit!=0);
//         } else if (convName=="selectFromHistogram") {
//           double min = myConversion.get("min").getDbl(),
//               max = myConversion.get("max").getDbl();
//           images["result"] = result.selectFromHistogram(min,max);
//         } else if (convName=="crop" || convName=="selectRegion") {
//           int top  = myConversion.get("top").getInt(),
//               left = myConversion.get("left").getInt(),
//               height = myConversion.get("height").getInt(),
//               width = myConversion.get("width").getInt();
//           images["result"] = (convName=="crop") ? 
//             result.crop(top,top+height,left,left+width) :
//             result.selectRegion(top,top+height,left,left+width);
//         } else {
//           responseCode = "400";  // any convName that doesnt fit above if () statements
//           throw std::string("conversion not found: " + convName);
//         }
//
//         // final step for all conversions: include result image in response
//         jsValue imgJSON(images["result"].toHTML());
//         jsBody.add("img", imgJSON);
//         jsBody.add("rows",images["result"].img.rows);
//         jsBody.add("cols",images["result"].img.cols);
//         jsBody.add("channels",images["result"].img.channels());
//
//       } else {
//         responseCode = "400";  // images[srcKey].count==0
//         throw std::string("image not found: " + srcKey);
//       }
//       
//     } catch (std::string s) {
//       throw std::string(" in conversion " + convName + ": " + s);
//     }
//    } else {
//      responseCode = "404";
//      responseBody = "NOT FOUND: " + url;
//      throw std::string("incorrect method: " + mName);
//    }
//
//    // create response body for call to opencv
//    jsValue duration((cv::getTickCount() - startTime) / cv::getTickFrequency());
//    jsBody.add("time",duration);
//    responseBody = jsBody.stringify();
//}

bool postImage(MomoMessage request, int socket_fd) {

  MomoMessage response;
  momo::json json;

  // -0- ENTER
  MYLOG(DEBUG,"ENTER url:" << request.url << " method:" << request.method);

  try {

    // -1- read xyz from body
    momo::jsValue myImage = json.parse(momo::tools::atob64(request.body));
    MomoImg opencvImg;
    opencvImg.readFromString(myImage.getString("imgData"), myImage.getString("imgType"));
    myImage.add("rows",opencvImg.img.rows);
    myImage.add("cols",opencvImg.img.cols);
    images.push_back(myImage);
    response.responseCode = "200";
    response.body = momo::tools::b64toa("{}");

  } catch (std::string s) {
    response.responseCode = "400";
    response.body = " FROM " + request.url + ": " + s;
    response.body = "ERROR: " + s; // momo::tools::atob64(response.body);
    std::cout << "sending body: " << response.body << std::endl;
  }
  
  // -3- send response
  sendResponse(socket_fd, response);
  return true; 
}

bool process(MomoMessage request, int socket_fd) {

  MomoMessage response;
  momo::json json;

  try {

    // -0- ENTER, read source and options from request, initialize result to copy of source
    momo::jsValue jsRequest = json.parse(momo::tools::atob64(request.body)),
      options = jsRequest.get("options");

    std::cout << jsRequest.stringify() << std::endl;
    std::string action = jsRequest.getString("action");
    momo::jsValue jsSources = jsRequest.getArray("source"); // an array of int indices
    int sourceIndex = jsSources.getInt(0);
    if (sourceIndex<0 || ((uint)sourceIndex) >= images.size()) {
      throw std::string(" non-existing index: please reload page ");
    }
    momo::jsValue jsSource = images[jsSources.getInt(0)];   // the first source image
    MomoImg result;
    result.readFromString(jsSource.getString("imgData"), jsSource.getString("imgType"));

    // -1- perform processing on result
    if (action == "crop") {
      result = result.crop(options);
    } else if (action == "rotate") {
      result = result.rotate(options);
    } else {
      throw std::string("action not implemented: ") + action;
    }

    // create response
    momo::jsValue jsImg, jsResponse;
    jsImg.add("rows", result.img.rows);
    jsImg.add("cols", result.img.cols);
    jsImg.add("imgData", result.getImgData(".png"));
    jsImg.add("imgType", ".png");
    jsResponse.add("img", jsImg);
    response.body = momo::tools::b64toa(momo::jsValue(jsResponse).stringify());    

  } catch (std::string s) {
    response.responseCode = "400";
    response.body = " from " + request.url + ": " + s;
    std::cout << "sending body: " << response.body << std::endl;
  }
  
  // -3- send response
  sendResponse(socket_fd, response);
  return true; 
}


bool getImages(MomoMessage request, int socket_fd) {

  MomoMessage response;

  try {

    // -0- ENTER, determine imgKey
    MYLOG(DEBUG,"ENTER url:" << request.url << " method:" << request.method);
    response.body = momo::tools::b64toa(momo::jsValue(images).stringify());
  } catch (std::string s) {
    response.responseCode = "400";
    response.body = " FROM " + request.url + ": " + s;
    response.body = "ERROR: " + s; // momo::tools::atob64(response.body);
    std::cout << "sending body: " << response.body << std::endl;
  }
  
  // -3- send response
  sendResponse(socket_fd, response);
  return true; 
}


bool getActions(MomoMessage request, int socket_fd) {

  MomoMessage response;
  momo::json json;

  // -0- ENTER
  MYLOG(DEBUG,"ENTER url:" << request.url << " method:" << request.method);

  // options are points, rects or conventional json key-value pairs
  // here in getActions the points and rects are communicated
  // as an array of attribute-names. Example, here we provide for rotation:
  //   angle : 0
  //   point : ["centre"]
  // so that we receive as a rotation instruction e.g.:
  //   angle : 192
  //   centre : [50, 200]

  try {
    momo::jsValue options;

    options.add("crop", momo::jsValue());
    options.getRef("crop").add("rect", json.parse("[\"crop\"]")); // top, bottom, left, right

    options.add("rotate", momo::jsValue());
    options.getRef("rotate").add("phi", momo::jsValue(0)); // 
    options.getRef("rotate").add("point", json.parse("[\"centre\"]")); // 

    options.add("selectHSVChannel", momo::jsValue());
    options.getRef("selectHSVChannel").add("channel", momo::jsValue(2));

    response.body = momo::tools::b64toa(options.stringify());
    response.responseCode = "200";

  } catch (std::string s) {
    response.responseCode = "400";
    response.body = " FROM " + request.url + ": " + s;
    response.body = "ERROR: " + s; // momo::tools::atob64(response.body);
    std::cout << "sending body: " << response.body << std::endl;
  }
  
  // -3- send response
  sendResponse(socket_fd, response);
  return true; 
}



int main (int arc, char**argv) {

  

  // -0- start
  Logging::prepare();
  MYLOG(DEBUG, "ENTER");

  // -1- start the server
  try {
    Http server(8076);
    server.addRoute("/img/postImage", postImage);
    server.addRoute("/img/getImages", getImages);
    server.addRoute("/img/process", process);
    server.addRoute("/img/getActions", getActions);
    server.listen();
  } catch (std::string s) {
  } catch (const char *s) {
  }

  // -2- cleanup and exit
  return 0;

}
