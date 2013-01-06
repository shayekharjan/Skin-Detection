#include "StdAfx.h"
#include <iostream>
#include <vector>
#include<math.h>
#include <stdlib.h>
#include "cv.h"
#include<highgui.h>
#include<conio.h>
#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>
#include <cvblob.h>
using namespace std;
using namespace cv;
#include "core\core_c.h"
using namespace cvb;

Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

/// Function header
void thresh_callback(int, void* );

/**
 * @function main
 */
int main( int argc, char** argv )
{
  /// Load source image and convert it to gray
  src = imread( "19054d.PNG", 1 );
  //src = imread( "skintest.jpg", 1 );

  /// Convert image to gray and blur it
  cvtColor( src, src_gray, CV_BGR2GRAY );
  blur( src_gray, src_gray, Size(3,3) );

  /// Create Window
  char* source_window = "Source";
  namedWindow( source_window, CV_WINDOW_AUTOSIZE );
  imshow( source_window, src );

  //createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
  thresh_callback( 0, 0 );

  waitKey(0);
  return(0);
}

/**
 * @function thresh_callback
 */
void thresh_callback(int, void* )
{
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using canny
  Canny( src_gray, canny_output, thresh, thresh*2, 3 );
  /// Find contours  
  findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Get the moments
  vector<Moments> mu(contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mu[i] = moments( contours[i], false ); }

  ///  Get the mass centers: 
  vector<Point2f> mc( contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

  /// Draw contours
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     { 
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() ); 
       circle( drawing, mc[i], 4, color, -1, 8, 0 );
     }

  /// Show in a window
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );

  /// Calculate the area with the moments 00 and compare with the result of the OpenCV function
  printf("\t Info: Area and Contour Length \n");
  for( int i = 0; i< contours.size(); i++ )
     {
       printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n", i, mu[i].m00, contourArea(contours[i]), arcLength( contours[i], true ) );  
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() ); 
       circle( drawing, mc[i], 4, color, -1, 8, 0 );
     }

 
Mat drawing_new = Mat::zeros( canny_output.size(), CV_8UC3 );
Mat drawing_new2 = Mat::zeros( canny_output.size(), CV_8UC3 );
Mat drawing_new3 = Mat::zeros( canny_output.size(), CV_8UC3 );
Mat drawing_new4 = Mat::zeros( canny_output.size(), CV_8UC3 );
  
Scalar color = Scalar( rng.uniform(255, 255), rng.uniform(255,255), rng.uniform(255,255) );

 /// Find the rotated rectangles and ellipses for each contour
  vector<RotatedRect> minRect( contours.size() );
  vector<RotatedRect> minEllipse( contours.size() );

  for( int i = 0; i < contours.size(); i++ )
     { minRect[i] = minAreaRect( Mat(contours[i]) );
       if( contours[i].size() > 5 )
         { minEllipse[i] = fitEllipse( Mat(contours[i]) ); }
     }

  /// Draw contours + rotated rects + ellipses
  Mat drawing4 = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       // contour
       drawContours( drawing_new, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
       // ellipse
       ellipse( drawing_new, minEllipse[i], color, 2, 8 );
	   //ellipse( drawing_new, minEllipse[L3i], color, 2, 8 );
	   //ellipse( drawing_new, minEllipse[L5i], color, 2, 8 );
       // rotated rectangle
       Point2f rect_points[4]; 
	   minRect[i].points( rect_points );
       //minRect[L3i].points( rect_points );
       //minRect[L5i].points( rect_points );
       
	   for( int j = 0; j < 4; j++ )
          line( drawing_new, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
     }


//drawContours( drawing_new, contours, L1i, color, 2, 8, hierarchy, 0, Point() ); 
//circle( drawing_new, mc[L1i], 4, color, -1, 8, 0 );

//Scalar color2 = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//drawContours( drawing_new, contours, L2i, color, 2, 8, hierarchy, 0, Point() ); 
//circle( drawing_new, mc[L2i], 4, color, -1, 8, 0 );

//Scalar color3 = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//drawContours( drawing_new3, contours, L3i, color, 2, 8, hierarchy, 0, Point() ); 

//drawContours( drawing_new2, contours, L5i, color, 2, 8, hierarchy, 0, Point() ); 
//drawContours( drawing_new3, contours, 13, color, 2, 8, hierarchy, 0, Point() ); 

//circle( drawing, mc[L3i], 4, color, -1, 8, 0 );
       
//Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() ); 
//       circle( drawing, mc[i], 4, color, -1, 8, 0 );
//IplImage *final_skin = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U,1);




//for(int i = 0 ; i < contours.size(); i++ )
//{
	//cvDrawContours( final_skin, contour[i], 255, 255, 0, CV_FILLED );
//	drawContours( drawing_new, contours, L1i, color, -1, 8, hierarchy, 0, Point() ); 
//	drawContours( drawing_new, contours, L3i, color, -1, 8, hierarchy, 0, Point() ); 
//    drawContours( drawing_new, contours, L5i, color, -1, 8, hierarchy, 0, Point() ); 



//		drawContours( drawing_new4, contours, L1i, color, -1, 8, hierarchy, 0, Point() ); 
//	drawContours( drawing_new4, contours, L3i, color, -1, 8, hierarchy, 0, Point() ); 
//    drawContours( drawing_new4, contours, L5i, color, -1, 8, hierarchy, 0, Point() ); 

//}



namedWindow( "Contours_new", CV_WINDOW_AUTOSIZE );
imshow( "Contours_new", drawing_new );


//namedWindow( "Contours_new2", CV_WINDOW_AUTOSIZE );
//imshow( "Contours_new2", drawing_new2 );

//namedWindow( "Contours_new3", CV_WINDOW_AUTOSIZE );
//imshow( "Contours_new3", drawing_new3 );


//namedWindow( "Contours_new4", CV_WINDOW_AUTOSIZE );
//imshow( "Contours_new4", drawing_new4 );

}
