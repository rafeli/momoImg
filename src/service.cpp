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
    if (mName == "getImage") {
     
     if (images.count(pText)>0) {
       responseBody = images[pText].toHTML();
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
       responseBody = keysAsJSON.stringify();
       
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else if (mName == "getProcessingOptions") {
     
     try {
       responseBody = PROCESSINGOPTIONS;
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else if (mName == "postImage") {
     
     try {
       jsObject myImage = parseBody(base64ToString(request.body));
       std::string imgData = myImage.get("imgData").getString(),
                   imgType = myImage.get("imgType").getString(),
                   imgKey = myImage.get("imgKey").getString();
       MomoImg momoImg;
       momoImg.readFromString(imgData, imgType);
       images[imgKey] = momoImg;
       responseBody = images[imgKey].toHTML();
       
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else if (mName == "convert") {
     
     try {
       jsObject myConversion = parseBody(base64ToString(request.body));
       std::string srcKey = myConversion.get("sourceImage").getString(),
                convName = myConversion.get("name").getString();

       if (images.count(srcKey)>0) {
         MomoImg srcImage = images[srcKey],
                 result = srcImage.clone();
         if (convName=="medianFilter") {
           int kSize = myConversion.get("kSize").getInt();
           images["result"] = result.medianFilter(kSize);
           responseBody = images["result"].toHTML();
         } else if (convName=="sharpen2D") {
           double rho = myConversion.get("rho").getDbl();
           int kSize = myConversion.get("kSize").getInt();
           images["result"] = result.sharpen2D(rho,kSize);
           responseBody = images["result"].toHTML();
         } else if (convName=="selectHSVChannel") {
           int channel = myConversion.get("channel").getInt();
           images["result"] = result.selectHSVChannel(channel);
           responseBody = images["result"].toHTML();
         } else if (convName=="selectFromHistogram") {
           double min = myConversion.get("min").getDbl(),
               max = myConversion.get("max").getDbl();
           images["result"] = result.selectFromHistogram(min,max);
           responseBody = images["result"].toHTML();
         } else if (convName=="crop" || convName=="selectRegion") {
           int top  = myConversion.get("top").getInt(),
               left = myConversion.get("left").getInt(),
               height = myConversion.get("height").getInt(),
               width = myConversion.get("width").getInt();
           images["result"] = (convName=="crop") ? 
             result.crop(top,top+height,left,left+width) :
             result.selectRegion(top,top+height,left,left+width);
           responseBody = images["result"].toHTML();
         } else {
           responseCode = "400";
           responseBody = "conversion not found: " + convName;
         }
       } else {
         responseCode = "400";
         responseBody = "image not found: " + srcKey;
       }

       
     } catch (std::string s) {
        responseCode = "400";
        responseBody = "error: " + s;
     }
    } else {
      responseCode = "404";
      responseBody = "NOT FOUND: " + url;
      throw std::string("incorrect method: " + mName);
    }

  } catch (std::string s) {
    if (responseCode == "200") responseCode = "400";
    responseBody = "SERVICE ERROR: ";
    responseBody += s;
  }


  // -2- send response
  if (responseCode == "200") {
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
