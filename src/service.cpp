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


// to be moved to schuelerlabor
std::vector<int> analyzeSlots(const std::vector<double> pixPerCol, int& left, int& width) {

  double sumA_, sumB_;
  bool countingA = false,   // count peaks from -1 to +1
       countingB = false;   // count peaks from -2 to +0
  int startA, startB = -1;
  std::vector<int> lanesWithSignal,  // slot-index of used slots
                   deviation;        // w.r.t. the expected position

  for (unsigned int col=0; col<pixPerCol.size(); col++) {

    if (pixPerCol[col]>-1) {
      if (countingA) {
        if (pixPerCol[col]>0) sumA_ += pixPerCol[col];
      } else if (col>1 && pixPerCol[col-1]<0) {
        startA = col;
        countingA=true;
      }
    } else {
      if (countingA) {
        int width_ = col - startA;
        if ((width_>20) && (width_ < width) && (sumA_/width_)>1.1) {
          int pos = (startA+col)/2,
              slot = (pos - left)/width;
          std::cout << "A peak at slot: " << slot << "soll: " 
                    << left + (slot+0.5)*width << " pos:" << pos << std::endl ;
          deviation.push_back(pos - (left + (slot+0.5)*width));
          lanesWithSignal.push_back(slot);
          countingB=false;
          sumB_ = 0;
          startB = -1;
        }
        sumA_ = 0;
        countingA = false;
        startA = -1;
      }
    }

    if (pixPerCol[col]>-2) {
      if (countingB) {
        if (pixPerCol[col]>0) sumB_ += pixPerCol[col];
      } else if (col>1 && pixPerCol[col-1]<0) {
        startB = col;
        countingB=true;
      }
    } else {
      if (countingB) {
        int width_ = col - startB;
        if ((width_>20) && width_<width && (sumB_/width_)>0.1) {
          int pos = (startB+col)/2,
              slot = (pos - left)/width;
          std::cout << "B peak at slot: " << slot << "soll: " 
                    << left + (slot+0.5)*width << " pos:" << pos << std::endl ;
          deviation.push_back(pos - (left + (slot+0.5)*width));
          lanesWithSignal.push_back(slot);
          countingA=false;
          sumA_ = 0;
          startA = -1;
        }
        sumB_ = 0;
        countingB = false;
        startB = -1;
      }
    }

  }

  // -2- korrigiere left und width wenn ein minimaler Anzahl Slots gefunden 
  double correction=0;
  if (lanesWithSignal.size()>3) {
    for (unsigned int i=0; i<lanesWithSignal.size(); i++) {
       correction += deviation[i]; 
    }
  }
  std::cout << "changing left from: " << left ;
  left += correction/lanesWithSignal.size();
  std::cout << " to: " << left << std::endl ;

  // -3-
  return lanesWithSignal;

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
       jsObject myImage = parseBody(base64ToString(request.body));
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
       jsObject myConversion = parseBody(base64ToString(request.body));
       std::string srcKey = myConversion.get("sourceImage").getString();
       convName = myConversion.get("name").getString();

       if (images.count(srcKey)>0) {
         MomoImg srcImage = images[srcKey],
                 result = srcImage.clone();
         if (convName=="test") {

           // to be moved to schuelerlabor 
           int param = myConversion.get("param").getInt(),  // not used
               leftBottom, rightBottom,          // two points in the bottom border
               topLeft, bottomLeft,              // two points in the left border
               topRight, bottomRight,            // two points in the right border
               numRows,                          // num Rows in reduced image
               numCols, 
               arrLen = result.img.rows*0.03;     // length of check-arrow

           MomoImg analysis,   // grayscale for analyzing purposes
                   totalMask,  // 0/1-mask indicating value>100
                   mask;       // different working 0/1-submasks 


           // -1- reduce, rotate and improve image 
           result = result.resize(0.5, 0.5)
             .rotate(180)
             .blurMaskFilter(0.9, 99)
             .blurMaskFilter(0.9, 99);

           // -2- find left,top and right borders with the help of getMedian, which returns the column (row)
           //     to which x% of all pixels are located left/above and (100-x) pixels are located right/below 
           //     better results with two calls
           analysis = result.clone().selectHSVChannel(2); 
           numRows = analysis.img.rows, 
           numCols = analysis.img.cols, 
           totalMask = analysis.clone().selectFromHistogram(80,100); // select 20% pixel with highest value
//           totalMask = result.clone().threshold(2,50,true); // alt: select pixel with hue>50

           mask  = totalMask.clone().selectRegion(numRows*0.7,numRows*0.95,numCols*0.2, numCols*0.4);
           leftBottom = mask.getMedian(mask.getNumOnes()*0.9, true);
           mask = totalMask.clone().selectRegion(numRows*0.7,numRows*0.95,numCols*0.6, numCols*0.8);
           rightBottom = mask.getMedian(mask.getNumOnes()*0.9, true);
           mask = totalMask.clone().selectRegion(numRows*0.2,numRows*0.4,numCols*0.1, numCols*0.5);
           topLeft = mask.getMedian(mask.getNumOnes()*0.1, false);
           mask = totalMask.clone().selectRegion(numRows*0.2,numRows*0.4,numCols*0.1, topLeft + numCols/20);
           topLeft = mask.getMedian(mask.getNumOnes()*0.1, false);
           mask = totalMask.clone().selectRegion(numRows*0.2,numRows*0.4,numCols*0.5, numCols*0.9);
           topRight = mask.getMedian(mask.getNumOnes()*0.9, false);
           mask = totalMask.clone().selectRegion(numRows*0.2,numRows*0.4,topRight - numCols/20, topRight+numCols/20);
           topRight = mask.getMedian(mask.getNumOnes()*0.9, false);
           mask = totalMask.clone().selectRegion(numRows*0.6,numRows*0.8,numCols*0.1, numCols*0.5);
           bottomLeft = mask.getMedian(mask.getNumOnes()*0.05, false);
           mask = totalMask.clone().selectRegion(numRows*0.6,numRows*0.8,bottomLeft-numCols/20, bottomLeft+numCols/20);
           bottomLeft = mask.getMedian(mask.getNumOnes()*0.05, false);
           mask = totalMask.clone().selectRegion(numRows*0.6,numRows*0.8,numCols*0.5, numCols*0.9);
           bottomRight = mask.getMedian(mask.getNumOnes()*0.95, false);
           mask = totalMask.clone().selectRegion(numRows*0.6,numRows*0.8,bottomRight-numCols/20, numCols*0.9); 
           bottomRight = mask.getMedian(mask.getNumOnes()*0.95, false);
           std::cout << "lt/rt: " << leftBottom << "/" << rightBottom 
                      << "\ntl/bl: " << topLeft << "/" << bottomLeft 
                      << "\ntr/br: " << topRight << "/" << bottomRight 
                      << std::endl;

           
           // -3- if needed rotate and or skew

           // -4- determine region to search for lanes and calculate columnsums
           int left = (topLeft + bottomLeft)/2 ,
               width = (topRight + bottomRight)/2 - left ,
               upperTop = (leftBottom + rightBottom)/2 - width*0.81,
               lowerTop = upperTop + width*0.43 ;

           // central dimension from which all others derive:
           left += width/30;
           width *= 0.95;        // actual value ~ 900
           
           MomoImg upperPanel = analysis.clone().crop(upperTop,upperTop+width/3,left, left+width),
               lowerPanel = analysis.clone().crop(lowerTop,lowerTop+width/3,left, left+width);

           analysis.drawSquare(Point(left,upperTop), Point(width, width/3));
           analysis.drawSquare(Point(left,lowerTop), Point(width, width/3));
           images["upperPanel"] = upperPanel; 
           images["lowerPanel"] = lowerPanel;

           images["new_" + srcKey] = result;

           // -4B- determine lanes in  upperPanel and lowerPanel
           std::vector<double> pixPerColUpper = (Mat) upperPanel.reduce(0,1),
                               pixPerColLower = (Mat) lowerPanel.reduce(0,1),
                               baseLine;

           // band filter:
           cv::blur(pixPerColUpper, pixPerColUpper,Size(3,1));
           cv::blur(pixPerColUpper, baseLine,Size(80,1));
           pixPerColUpper = (Mat) (((Mat) pixPerColUpper) - ((Mat) baseLine));
           cv::blur(pixPerColLower, pixPerColLower,Size(3,1));
           cv::blur(pixPerColLower, baseLine,Size(80,1));
           pixPerColLower = (Mat) (((Mat) pixPerColLower) - ((Mat) baseLine));

//           jsValue x(pixPerColUpper);
//           jsBody.add("plotData", x);


           // draw determined lanes into analysis
           // analysing pixPerCol

           int lowerOffset = 0.012*width,
               upperOffset = lowerOffset,
               lowerLaneWidth = width/20.3,
               upperLaneWidth = lowerLaneWidth;
           // determine used lanes *and* calculate correction factors
           std::vector<int> upperLanes = analyzeSlots(pixPerColUpper, upperOffset, upperLaneWidth),
                            lowerLanes = analyzeSlots(pixPerColLower, lowerOffset, lowerLaneWidth);
            
           for (unsigned int lane=0; lane<20; lane++) {

             Point lt1_ = Point(left + upperOffset + (lane+0.1)*upperLaneWidth, upperTop + 0.3*upperLaneWidth),
                   lt2_ = Point(left + lowerOffset + (lane+0.1)*lowerLaneWidth, lowerTop + 0.3*lowerLaneWidth),
                   diag = Point(0.04*width, width/3.5);

             analysis.drawSquare(lt1_, diag);
             analysis.drawSquare(lt2_, diag);
           }
           

           // -5- draw top/left/right checks as arrows into analysis
           analysis.drawArrow(Point(numCols*0.15, leftBottom+50), Point(arrLen,-50));
           analysis.drawArrow(Point(numCols*0.65, rightBottom+50), Point(arrLen,-50));
           analysis.drawArrow(Point(topLeft-50, numRows*0.2), Point(50,arrLen));
           analysis.drawArrow(Point(topRight+50, numRows*0.2), Point(-50,arrLen));
           analysis.drawArrow(Point(bottomLeft-50, numRows*0.6), Point(50,arrLen));
           analysis.drawArrow(Point(bottomRight+50, numRows*0.6), Point(-50,arrLen));

           // -5A- draw arrows in each Lane with detected content
           for (unsigned int i=0; i<upperLanes.size(); i++) {
             analysis.drawArrow(
               Point(left + upperOffset + upperLaneWidth*(upperLanes[i]+0.5), upperTop + width/3),
               Point(0,-arrLen/2));
           }
           for (unsigned int i=0; i<lowerLanes.size(); i++) {
             analysis.drawArrow(
               Point(left + lowerOffset + lowerLaneWidth*(lowerLanes[i]+0.5), lowerTop + width/3),
               Point(0,-arrLen/2));
           }


           // -6- save result in new_image and analysis in result
           images["new_" + srcKey] = result;
           images["result"] = analysis;


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
