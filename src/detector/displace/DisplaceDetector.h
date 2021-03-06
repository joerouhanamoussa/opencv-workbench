#ifndef DISPLACEDETECTOR_H_
#define DISPLACEDETECTOR_H_
/// ---------------------------------------------------------------------------
/// @file DisplaceDetector.h
/// @author Kevin DeMarco <kevin.demarco@gmail.com>
///
/// Time-stamp: <2015-07-14 17:39:11 syllogismrxs>
///
/// @version 1.0
/// Created: 04 Feb 2015
///
/// ---------------------------------------------------------------------------
/// @section LICENSE
///
/// The MIT License (MIT)
/// Copyright (c) 2012 Kevin DeMarco
///
/// Permission is hereby granted, free of charge, to any person obtaining a
/// copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation
/// the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
/// ---------------------------------------------------------------------------
/// @section DESCRIPTION
///
/// The BlankDetector class ...
///
/// ---------------------------------------------------------------------------
#include <opencv_workbench/detector/Detector.h>
#include <opencv_workbench/syllo/syllo.h>

class DisplaceDetector : public Detector{
public:
     DisplaceDetector();
     ~DisplaceDetector();
     void print();

     int set_frame(int frame_number, const cv::Mat &original);     
     
protected:

     cv::Mat erosionConfig_;
     cv::Mat dilationConfig_;
     
     std::map<int,syllo::Blob> prev_blobs_;
     std::map<int,syllo::Cluster> prev_clusters_;
     std::map<int,syllo::Cluster> all_clusters_;
     std::map<int,bool> track_ids_;
 
     // Running Gaussian Frames
     cv::Mat rg_;
     cv::Mat avgs_;
     cv::Mat variance_;     

private:
};

extern "C" {
    Detector *maker();
}

#endif
