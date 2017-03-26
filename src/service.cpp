#include <iostream>

#include "service.hpp"
#include "momo/binaryCoding.hpp"
#include "momo/http.hpp"
#include "momo/json.hpp"
#include "momoImg.hpp"

#define TEXTTYPE 0
#define INTTYPE 1
#define DBLTYPE 2

std::map<std::string,MomoImg> images; // TODO rethink for multiuser design
std::vector<jsValue> processOptions;

// parse body (JSON String), e.g. into a jsObject having "molecules" and "reactants"
jsObject parseBody(std::string body) {
  calcxx_driver driver;
  driver.parse(body);
  return driver.result.getObject();
}


bool callback(MomoMessage request, int socket_fd) {


  std::string url = request.url, mName="", pText ="", reqBody,
              response="", responseBody="", responseCode="200";
  jsObject o = getJSONObject("{}");
  jsValue jsBody(std::move(o));


  // -0- ENTER

  // -1- try to create the correct response
  responseBody = "hallo aus momoImg service, response to: " + url;
  responseCode = "200";
  try {

    // -1.1- check that url starts with /molgen, then delete this part
    // -     and extract methodName from url
    if (url.find("/img") == 0) {
      size_t pos=0;
      url = url.substr(4,std::string::npos);
      pos = url.find("/",1); // find end of aaa in molgen/aaa/bbb
      mName = url.substr(1,pos-1);
      if (pos < std::string::npos) pText = url.substr(pos+1, std::string::npos);
    } else {
      responseCode = "404";
      responseBody = "NOT FOUND";
      throw std::string("incorrect url " + url);
    }

    // -1.2- call requested method
    const int64 startTime = cv::getTickCount();
    if (mName == "getImage") {
     
     if (images.count(pText)>0) {
       jsValue imgJSON(images[pText].toHTML());
       // getJSONValue reads a JSON string, not a plain string !!
       jsBody.add("img", imgJSON);
     } else {
       responseCode = "400";
       responseBody = "image not found: " + pText;
     }

    } else if (mName == "getImgKeys") {
     
     try {
       std::vector<std::string> keys;
       keys.clear();
       for (auto const& it : images) keys.push_back(it.first);
       if (keys.size()==0) keys.push_back("NO IMAGES YET");
       jsValue keysAsJSON(keys);
       jsBody.add("imgKeys",keysAsJSON);
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else if (mName == "getProcessingOptions") {
     
     try {
       jsBody.add("options", getJSONValue(PROCESSINGOPTIONS));
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else if (mName == "postImage") {
     
     try {
       jsObject myImage = parseBody(atob64(request.body));
       std::string imgData = myImage.get("imgData").getString(),
                   imgType = myImage.get("imgType").getString(),
                   imgKey = myImage.get("imgKey").getString();
       MomoImg momoImg;
       momoImg.readFromString(imgData, imgType);
       images[imgKey] = momoImg;
       // no need to return data
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else if (mName == "convert") {

     std::string  convName;
     
     try {
       jsObject myConversion = parseBody(atob64(request.body));
       std::string srcKey = myConversion.get("sourceImage").getString();
       convName = myConversion.get("name").getString();

       if (images.count(srcKey)>0) {
         MomoImg srcImage = images[srcKey],
                 result = srcImage.clone();
         if (convName=="test") {
           result = result.rotate(91,0.5,0.5)
                    .crop(300,1630,400,2000)
                    .resize(1.62,1.462) 
                     ;

           images["result"] = result;


         } else if (convName=="bitwise_not") {
           images["result"] = result.bitwise_not();
         } else if (convName=="medianBlur") {
           int kSize = myConversion.get("kSize").getInt();
           images["result"] = result.medianBlur(kSize);
         } else if (convName=="gaussianBlur") {
           double sigmaX = myConversion.get("sigmaX").getDbl(),
                  sigmaY = myConversion.get("sigmaY").getDbl();
           images["result"] = result.gaussianBlur(sigmaX, sigmaY);
         } else if (convName=="equalizeHist") {
           images["result"] = result.equalizeHist();
         } else if (convName=="rotate") {
           double angle = myConversion.get("angle").getDbl(),
                  centerX = myConversion.get("centerX").getDbl(),
                  centerY = myConversion.get("centerY").getDbl();
           images["result"] = result.rotate(angle, centerX, centerY);
         } else if (convName=="resize") {
           double fx = myConversion.get("fx").getDbl(),
                  fy = myConversion.get("fy").getDbl();
           images["result"] = result.resize(fx,fy);
         } else if (convName=="blurMaskFilter") {
           double rho = myConversion.get("rho").getDbl();
           int kSize = myConversion.get("kSize").getInt();
           images["result"] = result.blurMaskFilter(rho,kSize);
         } else if (convName=="selectHSVChannel") {
           int channel = myConversion.get("channel").getInt();
           images["result"] = result.selectHSVChannel(channel);
         } else if (convName=="reduce") {
           int rType = myConversion.get("rType").getInt(),
               dim = myConversion.get("dim").getInt();
           std::vector<double> reduce_ = (Mat) result.reduce(dim, rType),
                               baseLine;

// //         hat mal funktioniert, aber nicht immer 
//            cv::dft(reduce_,reduce_);
          
           // folgendes funktioniert, liefert relativ gleichmaessige "sinus" 
           cv::blur(reduce_, reduce_,Size(3,1));
           cv::blur(reduce_, baseLine,Size(80,1));
           reduce_ = (Mat) (((Mat) reduce_) - ((Mat) baseLine));
           jsValue x(reduce_);
           jsBody.add("plotData", x);
           images["result"] = result;
         } else if (convName=="threshold") {
           int channel = myConversion.get("channel").getInt(),
               isLowerLimit = myConversion.get("isLowerLimit").getInt();
           double limit = myConversion.get("limit").getDbl();
           images["result"] = result.threshold(channel, limit, isLowerLimit!=0);
         } else if (convName=="selectFromHistogram") {
           double min = myConversion.get("min").getDbl(),
               max = myConversion.get("max").getDbl();
           images["result"] = result.selectFromHistogram(min,max);
         } else if (convName=="crop" || convName=="selectRegion") {
           int top  = myConversion.get("top").getInt(),
               left = myConversion.get("left").getInt(),
               height = myConversion.get("height").getInt(),
               width = myConversion.get("width").getInt();
           images["result"] = (convName=="crop") ? 
             result.crop(top,top+height,left,left+width) :
             result.selectRegion(top,top+height,left,left+width);
         } else {
           responseCode = "400";  // any convName that doesnt fit above if () statements
           throw std::string("conversion not found: " + convName);
         }

         // final step for all conversions: include result image in response
         jsValue imgJSON(images["result"].toHTML());
         jsBody.add("img", imgJSON);
         jsBody.add("rows",images["result"].img.rows);
         jsBody.add("cols",images["result"].img.cols);
         jsBody.add("channels",images["result"].img.channels());

       } else {
         responseCode = "400";  // images[srcKey].count==0
         throw std::string("image not found: " + srcKey);
       }
       
     } catch (std::string s) {
       throw std::string(" in conversion " + convName + ": " + s);
     }
    } else {
      responseCode = "404";
      responseBody = "NOT FOUND: " + url;
      throw std::string("incorrect method: " + mName);
    }

    // create response body for call to opencv
    jsValue duration((cv::getTickCount() - startTime) / cv::getTickFrequency());
    jsBody.add("time",duration);
    responseBody = jsBody.stringify();

  } catch (std::string s) {
    if (responseCode == "200") responseCode = "400";
    responseBody = "SERVICE ERROR: ";
    responseBody += s;
  }


  // -2- send response
  if (responseCode == "200") {
    responseBody = b64toa(responseBody);
    response = "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\nContent-Length: "
             + std::to_string(responseBody.length()) + "\r\n\r\n"
             + responseBody;
  } else {
    response = "HTTP/1.1 " + responseCode + " see Body\r\n"
             + "Content-Type: text/html\r\nContent-Length: "
             + std::to_string(responseBody.length()) + "\r\n\r\n"
             + responseBody;
  }
  send(socket_fd, response.c_str(), response.length(),0);

  // -4- RETURN
  return true;

}

int main (int arc, char**argv) {

  

  // -0- start
  Logging::prepare();


  // -1- init processing options
  try {
    calcxx_driver driver;
    driver.parse(PROCESSINGOPTIONS);
    processOptions = driver.result.getArray();
  } catch (std::string s) {
    std::cout <<  "INTERNAL ERROR: reading processing options: " <<  s;
    return -1;
  }
  
  

  // -1- start the server
  try {
    Http server(8076, callback);
  } catch (std::string s) {
  } catch (const char *s) {
  }

  // -2- cleanup and exit
  return 0;

}
