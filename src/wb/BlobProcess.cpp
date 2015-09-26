#include <iostream>

#include <opencv_workbench/wb/Point.h>
#include <opencv_workbench/track/hungarian.h>

#include "BlobProcess.h"

using std::cout;
using std::endl;

BlobProcess::BlobProcess()
{
     min_blob_size_ = 30;
     next_id_ = 0;
}

int BlobProcess::next_available_id()
{
     return next_id_++;
}

uchar BlobProcess::valueAt(cv::Mat &img, int row, int col)
{
     if (col >= 0 && col < img.cols) {
	  if (row >= 0 && row < img.rows) {
	       return img.at<uchar>(row,col);
	  }
     } 
     return 0;
}
     
uchar BlobProcess::findMin(uchar NE, uchar N, uchar NW, uchar W)
{
     int minChamp = 9999;

     if (NE == 0 && N == 0 && NW == 0 && W == 0) {
	  return 0;
     }

     if(NE != 0 && NE < minChamp) {
	  minChamp = NE;
     }

     if(N != 0 && N < minChamp) {
	  minChamp = N;
     }

     if(NW != 0 && NW < minChamp) {
	  minChamp = NW;
     }

     if(W != 0 && W < minChamp) {
	  minChamp = W;
     }

     return minChamp;
}

void BlobProcess::labelNeighbors(cv::Mat &img, std::vector<uchar> &labelTable, 
                                 uchar label, int i, int j)
{
     uchar value;

     value = valueAt(img,i-1,j+1);
     if (value != 0) {
	  labelTable[value] = label;
	  img.at<uchar>(i-1,j+1) = label;
     }

     value = valueAt(img,i-1,j);
     if (value != 0) {
	  labelTable[value] = label;
	  img.at<uchar>(i-1,j) = label;
     }

     value = valueAt(img,i-1,j-1);
     if (value != 0) {
	  labelTable[value] = label;
	  img.at<uchar>(i-1,j-1) = label;
     }

     value = valueAt(img,i,j-1);
     if (value != 0) {
	  labelTable[value] = label;
	  img.at<uchar>(i,j-1) = label;
     }		    
}

int** array_to_matrix(int* m, int rows, int cols) {
     int i,j;
     int** r;
     r = (int**)calloc(rows,sizeof(int*));
     for(i=0;i<rows;i++)
     {
          r[i] = (int*)calloc(cols,sizeof(int));
          for(j=0;j<cols;j++)
               r[i][j] = m[i*cols+j];
     }
     return r;
}

void delete_matrix(int **array, int rows, int cols)
{
     for (int r = 0; r < rows; r++) {
          free (array[r]);
     }
     free (array);
}

int BlobProcess::process_frame(cv::Mat &input)
{
     blobs_.clear();
     
     std::vector<uchar> labelTable;
     labelTable.push_back(0);

     cv::Mat img;
     input.copyTo(img);
     
     uchar label = 1;

     uchar NE, N, NW, W;

     for( int i = 0; i < img.rows; ++i) {
          for( int j = 0; j < img.cols; ++j ) {
               if (img.at<uchar>(i,j) != 0) {
                    NE = valueAt(img,i-1,j+1);
                    N  = valueAt(img,i-1,j);
                    NW = valueAt(img,i-1,j-1);
                    W  = valueAt(img,i,j-1);
                    
                    uchar value = findMin(NE,N,NW,W);

                    if (value == 0) {
                         img.at<uchar>(i,j) = label;
                         labelTable.push_back(label);
                         label++;
                    } else {
                         img.at<uchar>(i,j) = value;
                         labelNeighbors(img, labelTable, value, i, j);
                    }
               }
          }
     }
          
     // Second pass to fix connected components that have different labels
     std::map<int,wb::Blob> blobs_temp;
     for( int i = 0; i < img.rows; ++i) {
          for( int j = 0; j < img.cols; ++j ) {
               if (img.at<uchar>(i,j) != 0) {
                    int id = labelTable[img.at<uchar>(i,j)];
                    //img.at<uchar>(i,j) = id;

                    // If the ID is new, add it to the blob map
                    if (blobs_temp.count(id) == 0) {
                         wb::Blob blob(id);
                         blobs_temp[id] = blob;
                    }
                    wb::Point p;
                    p.set_position(cv::Point(j,i));
                    blobs_temp[id].add_point(p);
               }
          }
     }     

     //////////////////////////////////////////////////////////////////////////
     // Remove small blobs / convert map to vector
     //////////////////////////////////////////////////////////////////////////
     std::map<int,wb::Blob>::iterator it_temp = blobs_temp.begin();
     std::vector<wb::Blob> new_blobs;
     for(; it_temp != blobs_temp.end(); it_temp++) {
          // Only copy over blobs that are of a minimum size
          if (it_temp->second.size() >= min_blob_size_) {
               it_temp->second.compute_metrics();
               new_blobs.push_back(it_temp->second);
          }
     }    

     //////////////////////////////////////////////////////////////////////////
     // Use the Hungarian Method to match new blob measurements with previous
     // blob tracks
     //////////////////////////////////////////////////////////////////////////

     // Create cost matrix using the euclidean distance between previous and 
     // current blob centroids
     int blob_count = new_blobs.size();
     int prev_blob_count = prev_blobs_.size();

     // Determine max of blob_count and prev_blob_count
     int rows = -1, cols = -1;
          
     int r_start = 0, r_end = 0;
     int c_start = 0, c_end = 0;

     if (blob_count == prev_blob_count) {
          // Equal number of tracks and measurements
          rows = cols = blob_count;          
     } else if (blob_count > prev_blob_count) {
          // More measurements than tracks
          rows = cols = blob_count;
          
          r_start = 0;
          r_end = rows;
          c_start = prev_blob_count;
          c_end = blob_count;
     } else {
          // More tracks than measurements
          rows = cols = prev_blob_count;
          
          r_start = blob_count;
          r_end = prev_blob_count;
          c_start = 0;
          c_end = cols;
     }

     int * cost = new int[rows*cols];
          
     // New blob measurements are along the Y-axis (left hand side)
     // Old Blob tracks are along x-axis (top-side)
     std::vector<wb::Blob>::iterator it = new_blobs.begin();
     int r = 0;
     int max_cost = -1e3;
     for(; it != new_blobs.end(); it++) {          
          std::vector<wb::Blob>::iterator it_prev = prev_blobs_.begin();
          int c = 0;
          for (; it_prev != prev_blobs_.end(); it_prev++) {
               cv::Point p1 = it->centroid();
               cv::Point p2 = it_prev->centroid();
               int dist = round(pow(p1.x-p2.x,2) + pow(p1.y-p2.y,2));
               
               cost[r*cols + c] = dist;

               if (dist > max_cost) {
                    max_cost = dist;
               }
               
               c++;
          }
          r++;
     }

     // If necessary, pad cost matrix with appropriate number of dummy rows or
     // columns          
     for (int r = r_start; r < r_end; r++) {
          for (int c = c_start; c < c_end; c++) {
               cost[r*cols + c] = max_cost;
          }
     }
          
     int** m = array_to_matrix(cost,rows, cols);
     delete[] cost;
     
     hungarian_problem_t p;
     int matrix_size = hungarian_init(&p, m, rows, cols, HUNGARIAN_MODE_MINIMIZE_COST);
     hungarian_solve(&p);

     // Get assignment matrix;
     int * assignment = new int[rows*cols];
     hungarian_get_assignment(&p, assignment);
         
     // Use the assignment to update the old tracks with new blob measurement     
     r = 0;
     it = new_blobs.begin();
     for(int r = 0; r < rows; r++) {          
          std::vector<wb::Blob>::iterator it_prev = prev_blobs_.begin();
          for (int c = 0; c < cols; c++) {
               if (assignment[r*cols + c] == 1) {
                    if (r < blob_count && c < prev_blob_count) {
                         // Found an assignment. Update the new measurement
                         // with the track ID and age of older track. Add
                         // to blobs_ vector
                         it->set_id(it_prev->id());
                         it->set_age(it_prev->age()+1);
                         blobs_.push_back(*it);
                    } else if (r >= blob_count) {
                         // Possible missed track (dec age and copy over)
                         it_prev->dec_age();
                         blobs_.push_back(*it_prev);
                         // TODO
                    } else if (c >= prev_blob_count) {
                         // Possible new track
                         it->set_id(next_available_id());
                         it->set_age(1);
                         blobs_.push_back(*it);
                    }
                    break; // There is only one assignment per row
               }
               if (c < prev_blob_count-1) {
                    it_prev++;
               }
          }
          if (r < blob_count-1) {
               it++;
          }
     }     
     
     hungarian_free(&p);
     free(m);               
     delete[] assignment;
     
     //std::map<int,syllo::Blob>::iterator it;
     //for(it=blobs_.begin();it!=blobs_.end();it++)
     //{
     //     printf("id: %d \t size:%d\n",it->first,it->second.getSize());
     //}

     //output = reduced;
     prev_blobs_ = blobs_;
     
     return blobs_.size();
}

void BlobProcess::overlay_blobs(cv::Mat &src, cv::Mat &dst)
{
     cv::Mat color;
     cv::cvtColor(src, color, CV_GRAY2BGR);     
     dst = color;
     
     std::vector<wb::Blob>::iterator it = blobs_.begin();
     for(; it != blobs_.end(); it++) {
          // Draw all blob points in the image
          std::vector<wb::Point> points = it->points();
          std::vector<wb::Point>::iterator it_points = points.begin();
          for(; it_points != points.end(); it_points++) {                    
               dst.at<cv::Vec3b>(it_points->y(), it_points->x()) = cv::Vec3b(20,255,57);
          }

          cv::Point centroid_point = it->centroid();
          cv::Rect rect = it->rectangle();
               
          std::ostringstream convert;
          convert << it->id();               
          const std::string& text = convert.str();
          
          cv::circle(dst, centroid_point, 1, cv::Scalar(255,255,255), -1, 8, 0);
          cv::rectangle(dst, rect, cv::Scalar(255,255,255), 1, 8, 0);
          cv::putText(dst, text, cv::Point(rect.x-3,rect.y-3), cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(255,255,255), 1, 8, false);
     }
}
