#include "momoImg.hpp"

MomoImg::MomoImg(){
  centerRow = centerCol = 0;
}


MomoImg::MomoImg(Mat m) {
  img = m; // macht dies ein Verweis oder Kopie ??? noch mal lesen
           // und dann in tutorial aufnehmen ...
  centerRow = centerCol = 0; // TODO: wie geht das mit mehreren Konstruktoren?
}

void MomoImg::saveImage(std::string fp) {
  imwrite(fp, img);
}

std::string MomoImg::toHTML() {

  // encode image as jpg then as base64:
  // see: http://stackoverflow.com/questions/801199/opencv-to-use-in-memory-buffers-or-file-pointers
  std::vector<uchar> buf;
  cv::imencode(".jpg", img, buf, std::vector<int>() );
  std::string base64 = encodeBase64((unsigned char*) &buf[0], buf.size());
//  return encodeBase64((unsigned char*) &buf[0], buf.size());

  return std::string("<img src=\"data:jpeg;base64," + base64 + "\">");

}

void MomoImg::checkImage(std::string t, std::string fn, std::string suffix) {

  Mat x;
  int fontType =  FONT_HERSHEY_COMPLEX;
  Scalar fontColor = Scalar(255,255,255);

  // -1- create image, insert the text t into this image
  resize(img,x,Size(600,400));
  if (t.length()>0) putText(img, t, Point(50,50),fontType,2,fontColor,1);

  // -2- construct the filename, e.g. xyz_CENTER.jpg  for xyz.jpg
  if (fn.find_last_of(".")==std::string::npos) {
    std::cout << "unexpected filename without '.jpg': " << fn 
              << "\n\t cannot write check image" << std::endl;
    return;
  }
  fn.insert(fn.find_last_of("."),suffix);

  // -3- write the check file
  MYLOG(DEBUG,"writing check image: " << fn );
  imwrite(fn,img);
}

MomoImg MomoImg::drawSquare(const Point topleft, const Point bottomright) {

  Point a = Point(topleft.x, bottomright.y),
        b = Point(bottomright.x, topleft.y);

  line(img,topleft,a, Scalar(255,255,255),4,8);
  line(img,topleft,b, 255,4,8);
  line(img,bottomright,a, 255,4,8);
  line(img,bottomright,b, 255,4,8);

  return (*this);
}

MomoImg MomoImg::drawArrow(const Point start, const Point arrow) {

  Point normal = Point(arrow.y, -1*arrow.x);

  line(img,start,start + arrow,255,4,8);
  line(img,start + 0.7*arrow + 0.2*normal, start+arrow, 255,4,8); 
  line(img,start + 0.7*arrow - 0.2*normal, start+arrow, 255,4,8); 
  
  return (*this);
}

MomoImg MomoImg::label(const Point x, std::string text) {

  int fontType =  FONT_HERSHEY_PLAIN;
  Scalar fontColor = Scalar(255,255,255);

  putText(img, text, x,fontType,5,fontColor,4);

  return (*this);
}


MomoImg MomoImg::readImage(std::string fileName) {

  img = imread(fileName);
  if (!img.data) throw "could not read image from: " + fileName;
  return (*this);
}


MomoImg MomoImg::clone() {

  MomoImg myClone(img.clone());

  return myClone;
}


bool MomoImg::isValidPoint(Point p) {
  return (p.y<img.rows && p.x<img.cols);
}

long MomoImg::getNumOnes() {

  int nCols=img.cols, 
      nRows=img.rows;
  long nPix = 0;

  for (int i=0; i<nRows; i++) { 
    uchar* data = img.ptr<uchar>(i);
    for (int j=0; j<nCols; j++) 
      if (data[j]) {
        nPix++;
    }
  }
 return nPix;
}

int MomoImg::getMedian(long median, bool countRows) {
  
  int nCols=img.cols, 
      nRows=img.rows;
  long nPix = 0;

  if (countRows) {
    for (int row=0; row<nRows; row++) { 
      uchar* data = img.ptr<uchar>(row);
      for (int col=0; col<nCols; col++) {
        if (data[col]) nPix++;
      }
      if (nPix>median) return row;
    }
  } else {
    for (int col=0; col<nCols; col++) {
      for (int row=0; row<nRows; row++) { 
        if (img.at<uchar>(row,col)) nPix++;
      }
      if (nPix>median) return col;
    }
  }

  return -1;
}

Point MomoImg::getCenter() {

  int nCols=img.cols, 
      nRows=img.rows;
  long row_, col_,nPixel;

  // -1- ENTER and warn if not binary picture
  nPixel = 0;
  MYLOG(DEBUG,"ENTERING, nRows:" << nRows << " nCols:" << nCols << " type: " << img.type());
  // TODO: warn if not binary

  // -2- Loop and calculate Sum row*f(row) sum col*f(col)
  row_ = col_ = 0;
  for (int i=0; i<nRows; i++) { 
    uchar* data = img.ptr<uchar>(i);
    for (int j=0; j<nCols; j++) 
      if (data[j]) {
        row_ += i;
        col_ += j;
        nPixel++;
    }
  }

  // -3- determine center of mass from above 
  //     warn if nPixel too low
  if (nPixel == 0) throw "cannot determine center from zero Pixels\n";
  centerRow =  (row_ / nPixel);
  centerCol =  (col_ / nPixel);
  if (centerRow>nRows) throw "from MomoImg::getCenter invalid result for row";
  if (centerCol>nCols) throw "from MomoImg::getCenter invalid result for col";

  // -4- Exit
  MYLOG(DEBUG,"EXITING: center row=" << centerRow << " col: " << centerCol << " nPix: " << nPixel);
  return Point(centerCol, centerRow);
}

// parameter horizontal should true if the structure is close to horizontally
double MomoImg::getAngle(bool horizontal) {

  int nRows=img.rows,
      nCols=img.cols;
  double weight, totalWeight =0,
         angle = 0;

  // -0- ENTER
  MYLOG(DEBUG, "ENTERING center row:" << centerRow << " col:" << centerCol);
  getCenter();

  // -1- Loop through pixel, add angle for each
  //     weighted with distance squared
  for (int i=0; i<nRows; i++) { 
    uchar* data = img.ptr<uchar>(i);
    weight = (i-centerRow);
    weight *= weight; // no-op for horizontal==true;
    for (int j=0; j<nCols; j++) {
      if (j==centerCol) continue;
      if (data[j]) {
        if (horizontal) {
          weight = (j-centerCol); 
          weight *= weight;
        }
        totalWeight += weight; 
        angle += weight*(i-centerRow)/(j-centerCol);
      }
    }
  }
  
  // -2- normalize and return
  angle = atan(angle/totalWeight);
  MYLOG(DEBUG,"EXITING w. angle: " << angle);
  return angle; 
}

MomoImg MomoImg::selectCannyLines( unsigned int lowThreshold, unsigned int highThreshold) {

  cv::Canny(img,img,lowThreshold,highThreshold);
  return (*this);
}

MomoImg MomoImg::selectHSVChannel(unsigned int i) {
  std::vector<Mat> channels;
  cvtColor(img, img, CV_BGR2HSV);
  split(img, channels);
  img = channels[i%3]; 

  return (*this);
}

MomoImg MomoImg::selectContrast(unsigned int nPix, bool horizontal) {

  Mat vMask, hMask,
      vBox(nPix,1,CV_8U,Scalar(1)),
      hBox(1,nPix,CV_8U,Scalar(1));

  // -1- determine contrast as difference between two masks,
  //     on grown in horizontal, the other in vertical direction:
  erode(img, hMask, hBox);
  erode(img, vMask, vBox);

   // -2- selection
  if (horizontal) {
    img = hMask - vMask;
    selectShape(1,nPix,3);
  } else {
    img = vMask - hMask;
    selectShape(nPix,1,3);
  }

  // -3- return
  return (*this);

}

MomoImg MomoImg::selectShape(int hSize, int vSize, int repeat) {

  Mat box(hSize,vSize,CV_8U, Scalar(1));
  for (int i=0; i<repeat; i++) erode(img, img, box);
  return (*this);
}

MomoImg MomoImg::selectShapeColor(int hSize, int vSize, int channel ) {

  Mat result,hsvimg
      , erosionBox(hSize, vSize, CV_8U, Scalar(1));;
  std::vector<Mat> channels;

  // -1- select "adaptively filtered" horiz/vertic-component
  //     of hsv imags: subtract a picture that has been smoothed in direction d
  cvtColor(img, hsvimg, CV_BGR2HSV);
  split(hsvimg, channels);
  boxFilter(channels[channel], result, CV_8U, Size(hSize*hSize, vSize*vSize));
  result = channels[channel] - result;

  // -2- convert to binary, then filter again for vertical/horizontal
  threshold(result, result, 15, 255, cv::THRESH_BINARY); 
  erode(result, img, erosionBox);

  // return
  return (*this);
  
}

MomoImg MomoImg::dilateShape(int hSize, int vSize, int repeat) {

  Mat box(hSize,vSize,CV_8U, Scalar(1));
  for (int i=0; i<repeat; i++) dilate(img, img, box);
  return (*this);
}

MomoImg MomoImg::setSize(unsigned int rows, unsigned int cols) {

  MYLOG(DEBUG,"Enter, rows:" << img.rows << " cols:" << img.cols);
  try {
    resize(img,img,Size(rows,cols));
  } catch (cv::Exception& e) {
    throw " from MomoImg::setSize: " + e.err;
  }
  return (*this);
}

/*************
 * add image m at location (x,y)
 * by subselecting part of this that starts at (x,y)
 * and then call addImage(m) 
 */
MomoImg MomoImg::addImage(Mat m, unsigned int atRow, unsigned int atCol) {

  Mat origImg = img,  // origImg and img are different pointers to (regions of) the same Image
      mask;

  // -0- check arguments
  if (atRow>img.rows || atCol >img.cols) {
    throw "from addImage: cannot add at "
        + std::to_string(atRow) +":" + std::to_string(atCol)
        + " in image of size " 
        + std::to_string(img.rows) +":" + std::to_string(img.cols);
  }

  // set img to the part starting at (atRow, atCol), then call addImage()
  img = img(Range(atRow,origImg.rows),Range(atCol, origImg.cols));
  addImage(m);

  // reset to complete image
  img = origImg;
 
  return (*this);
}


/********************************************
 * add image m at location (0,0) = top left
 * crop m to my size if needed
 * dont copy black pixels in m
 */
MomoImg MomoImg::addImage(Mat m) {

  // -0- Enter, TODO: check dimensions ??
  if (m.cols== img.cols && m.rows==img.rows) {
    MYLOG(DEBUG,"ENTERING, added image fits: " << m.rows << ":" << m.cols );
  } else {
    MYLOG(WARNING,"ENTERING, adding non-fittig image: " << m.rows << ":" << m.cols 
           << " to me:" << img.rows << ":" << img.cols);
  }

  // -1- crop m if too large:
  int minRows = (m.rows<img.rows) ? m.rows : img.rows,
      minCols = (m.cols<img.cols) ? m.cols : img.cols;
  MomoImg cropped_m(m);
  cropped_m.crop(0,minRows,0,minCols);

  // -2- construct a mask where m != 0
  Mat mask = cropped_m.clone().selectHSVChannel(2).img > 1;

  // -3- add (to upper left part if m smaller than my image)
  cropped_m.img.copyTo(img(Range(0,minRows),Range(0, minCols)),mask);

  // -2- exit
  MYLOG(DEBUG,"EXITING");
  return (*this);
  
}

MomoImg MomoImg::rotate(double phi) {

  Mat rotMatrix(2,3, CV_32FC1);
  Point center = Point(img.cols/2, img.rows/2);

  rotMatrix = getRotationMatrix2D(center, phi, 1.0 );
  warpAffine(img, img, rotMatrix, img.size());

  return (*this);


/* old code: 

  Mat srcX(img.rows, img.cols, CV_32F);
  Mat srcY(img.rows, img.cols, CV_32F);

  // -0- ENTER
  MYLOG(DEBUG,"ENTERING");

  // -1- create mapping
  for (int i=0; i<img.rows; i++)
    for (int j=0; j<img.cols; j++) {
      srcX.at<float>(i,j) = j*cos(phi) - i*sin(phi);
      srcY.at<float>(i,j) = j*sin(phi) + i*cos(phi);
    }

  // -2- map
  remap(img, img, srcX, srcY, cv::INTER_LINEAR);

  // -3- return
  MYLOG(DEBUG, "EXITING");
 
*/

}

MomoImg MomoImg::selectHSV( int hMin, int hMax, int sMin, int sMax, int vMin, int vMax) {

  Mat hsvImage, mask,
      totalMask(img.rows, img.cols, CV_8U, 255); // all pixel init to one ?
  std::vector<Mat> channels;
  int filter[] = {hMin, hMax, sMin, sMax, vMin, vMax};

  // -0- ENTER
  MYLOG(DEBUG,"ENTERING for img mit rows: " <<img.rows << " cols: " << img.cols );


  // -1- convert image to hsv, then split into channels
  cvtColor(img, hsvImage, CV_BGR2HSV);
  split(hsvImage, channels);

  // -2- apply filters
  for (unsigned int i=0; i<6; i++) {

    bool lowLimit = (i%2==0);

    if ((lowLimit && filter[i]>0) || (!lowLimit && filter[i]<255)) {
       threshold(channels[i/2], mask, filter[i], 255 
                 , (lowLimit) ? THRESH_BINARY : THRESH_BINARY_INV); 
       totalMask = totalMask & mask;
    }
  }

  // -3- return
  // TODO: set nPixel to totalMask.numberSelectedPixel
  MYLOG(DEBUG,"EXITING" );
  img = totalMask;
  return (*this);

}



// set all bits outside selected region to zero
MomoImg MomoImg::selectRegion(unsigned int rowMin,unsigned int rowMax
                          ,unsigned int colMin,unsigned int colMax) {
  unsigned int nRows = img.rows,
               nCols = img.cols;

  // -0- ENTER
  MYLOG(DEBUG, "Enter: " << rowMin << ":" << rowMax << ":" << colMin << ":" << colMax);
  if (rowMax>nRows) rowMax = nRows;
  if (colMax>nCols) colMax = nCols;

  // -1- deselect all rows < rowMin
  for (unsigned int i=0; i<rowMin; i++) { 
    uchar* data = img.ptr<uchar>(i);
    for (unsigned int j=0; j<nCols; j++) data[j] =0;
  }

  // -2- for rowMin < row < rowMax:
  // deselect j<colMin and j>colMax
  for (unsigned int i=rowMin; i<rowMax; i++) { 
    uchar* data = img.ptr<uchar>(i);
    for (unsigned int j=0; j<colMin; j++) data[j] =0;
    for (unsigned int j=colMax; j<nCols; j++) data[j] =0;
  }

  // -3- deselect all rows > rowMax
  for (unsigned int i=rowMax; i<nRows; i++) { 
    uchar* data = img.ptr<uchar>(i);
    for (unsigned int j=0; j<nCols; j++) data[j] =0;
  }

  // -4- return
  return (*this);

}

MomoImg MomoImg::crop(unsigned int rowMin,unsigned int rowMax
                  ,unsigned int colMin,unsigned int colMax) {

  unsigned int nRows = img.rows,
               nCols = img.cols;

  // -0- ENTER
  MYLOG(DEBUG, "ENTERING: " << rowMin << ":" << rowMax << ":" << colMin << ":" << colMax);
  if (rowMax>nRows) rowMax = nRows;
  if (colMax>nCols) colMax = nCols;

  // -1- crop
  Rect region(colMin, rowMin, colMax - colMin, rowMax - rowMin);
  Mat croppedImage(img, region);
  img = croppedImage;

  // -2- exit
  MYLOG(DEBUG,"EXITING");
  return (*this);
}
