#include <iostream>
#include "RelativeDetector.h"

#include <opencv_workbench/utils/ColorMaps.h>
#include <opencv_workbench/wb/WB.h>

using std::cout;
using std::endl;

RelativeDetector::RelativeDetector()
{
     cout << "RelativeDetector Constructor" << endl;
     thresh_value_ = 255;
     grad_thresh_value_ = 255;
}

RelativeDetector::~RelativeDetector()
{
     cout << "RelativeDetector Destructor" << endl;
}

void RelativeDetector::print()
{
     cout << "I am the Relative Detector" << endl;
}

int RelativeDetector::set_frame(int frame_number, const cv::Mat &original)
{         
     cv::Mat original_w_tracks;          
     original_w_tracks = original;
     
     //cv::Mat bad_gray;
     ////cv::applyColorMap(original, bad_gray, CV_BGR2GRAY);
     //cv::cvtColor(original, bad_gray, CV_BGR2GRAY);
     //cv::imshow("bad", bad_gray);
     
     cv::Mat gray;
     Jet2Gray_matlab(original,gray);
     cv::imshow("gray",gray);        
     
     // Threshold
     cv::Mat thresh;
     //cv::threshold(gray, thresh, 150, 255, cv::THRESH_TOZERO);
     wb::adaptive_threshold(gray, thresh, thresh_value_, 0.001, 0.002, 10, 5);
     cv::imshow("thresh",thresh);     
     
     wb::cluster_points(thresh, 0, 15);     
     
     //// Compute median
     //cv::Mat median;
     //cv::medianBlur(gray,median,5);
     //cv::imshow("median", median);          
     //
     //// Compute estimated gradient
     //cv::Mat grad;     
     //syllo::gradient_sobel(median, grad);
     //cv::imshow("gradient", grad);
     //
     //// Only select gradients above a certain threshold
     //cv::Mat grad_thresh;
     //syllo::adaptive_threshold(grad, grad_thresh, grad_thresh_value_, 0.001, 0.002, 10, 5);
     //cv::imshow("gradient_thresh", grad_thresh);
     
     
     
     //////////////////////////////////////////////////////////////
     /// Tracking     
     tracks_.clear(); // clear out the tracks from previous loop
     
     ///////////////////////////////////////////////////
     // Display images
     ///////////////////////////////////////////////////
     if (!hide_windows_) {          
     }
     
     return 0;
}

extern "C" {
     Detector *maker(){
          return new RelativeDetector;
     }

     class proxy {
     public:
          proxy(){
               // register the maker with the factory
               // causes static initialization error plugin_manager_.factory["blank_detector"] = maker;
               plugin_manager_.factory["relative_detector"] = maker;
          }
     };
     // our one instance of the proxy
     proxy p;
}


/// Color map tests
///cv::Mat grad;
///create_gradient(grad, 100, 255);
///cv::imshow("gradient", grad);
///
///cv::Mat jet_matlab;
///Gray2Jet_matlab(grad,jet_matlab);
///cv::imshow("jet matlab", jet_matlab);
///
///cv::Mat gray_2;
///Jet2Gray_matlab(jet_matlab,gray_2);
///cv::imshow("gray_2",gray_2);
///
///cv::Mat diff = gray_2 - grad;
///cv::imshow("diff",diff);
///
///if (!equal(gray_2, grad)) {
///     cout << "Grays are unequal" << endl;
///}     