#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <string>
#include "st_motion.h"


using namespace std;
using namespace cv;

int main(int argc, char **argv) {
  const int threshold = 50;
  const std::vector<int> facemark = {1, 5, 7, 2, 6, 8, 0};
  std::ofstream ofs;

  std::string videofile;
  std::string binfile;
  bool framelater = false;
  int64_t beginframe = 0;
  int64_t endframe = 0;


  if (argc == 3){
    videofile = argv[1];
    binfile = argv[2];
    framelater = true;
  } else if (argc == 5){
    videofile = argv[1];
    binfile = argv[2];
    beginframe = atoi(argv[3]);
    endframe = atoi(argv[4]);
  } else {
    fprintf(stderr, "Invalid input!\n");
    return -1;
  }

  ofs.open(binfile, std::ios::out);

  cv::VideoCapture capture(videofile);
  // capture.open(0);         // open the camera
  if (!capture.isOpened()) {
    fprintf(stderr, "Track can not read!\n");
    return -1;
  }
  // capture.set(CV_CAP_PROP_FRAME_WIDTH, 1080);
  // capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

  int64_t nFrame = capture.get(CV_CAP_PROP_FRAME_COUNT);
  if (framelater) endframe = nFrame;

  st_result_t ret = ST_OK;
  st_handle_t body_handle;
  
  //加载授权证书
  ret = st_motion_public_init_license("license.lic", "license");
  if(ret != ST_OK){
    fprintf(stderr, "st_motion_public_init_license error %d\n", ret);
    return -1;
  }

  // create the body track handle
  ret = st_motion_body_create_tracker(&body_handle, "../../models/Body_Track.model");
  if (ret != ST_OK) {
    fprintf(stderr, "st_motion_body_create_tracker failed, error code: %d\n", ret);
    return -1;
  }
  // limit the track bodies count, -1 means no limit
  ret = st_motion_body_set_track_body_cnt_limit(body_handle, -1, NULL);
  if (ret != ST_OK) {
    st_motion_body_destroy_tracker(body_handle);
    fprintf(stderr, "st_motion_body_set_track_body_cnt_limit failed, error code: %d\n", ret);
    return -1;
  }

  int nframe = 0;
  Mat read_frame;
  while (capture.read(read_frame)) {
    if ((nframe>=beginframe) && (nframe<=endframe))
    {
      st_motion_body_t *p_bodies = nullptr;
        int bodies_count = 0;
        // body track
        ret = st_motion_body_track(body_handle,
          read_frame.data, ST_PIX_FMT_BGR888, read_frame.cols, read_frame.rows, &p_bodies, &bodies_count);
        if (ret != ST_OK) {
          fprintf(stderr, "st_motion_body_track failed, error code: %d\n", ret);
          break;
        }
    
        Scalar colors[] = { CV_RGB(255, 0, 0), CV_RGB(0, 255, 0), CV_RGB(255, 0, 255), \
          CV_RGB(0, 0, 255), CV_RGB(0, 255, 255), CV_RGB(255, 0, 0), \
          CV_RGB(255, 255, 255), CV_RGB(255, 255, 0), CV_RGB(0, 0, 255) };
    
        ofs << nframe <<std::endl;
        for (int i = 0; i < bodies_count; i++) {
          for (auto j = facemark.begin(); j != facemark.end(); j++){
          // for (int j = 0; j < p_bodies[i].keypoints_count; j++) {
            int coloridx = i % 9;
            circle(read_frame, Point2f(p_bodies[i].keypoints[*j].x, p_bodies[i].keypoints[*j].y), 5, colors[coloridx], 2);
            ofs << p_bodies[i].keypoints[*j].x << " " << p_bodies[i].keypoints[*j].y <<std::endl;
          }
    
          
    
          // draw line with the body key points
          vector<pair<int, int>> limbs;
          if (p_bodies[0].keypoints_count == 9) {
            limbs.push_back(make_pair(1, 5));
            limbs.push_back(make_pair(5, 7));
            limbs.push_back(make_pair(2, 6));
            limbs.push_back(make_pair(6, 8));
            limbs.push_back(make_pair(3, 4));
          }
          for (int j = 0; j < limbs.size(); j++) {
            int ja = limbs[j].first;
            int jb = limbs[j].second;
            line(read_frame, Point2f(p_bodies[0].keypoints[ja].x, p_bodies[0].keypoints[ja].y),
              Point2f(p_bodies[0].keypoints[jb].x, p_bodies[0].keypoints[jb].y), CV_RGB(255, 0, 0), 2);
          }
        }
        ofs << std::endl;
    
 
        st_motion_body_release_track_result(p_bodies, bodies_count);   // release the memory of body track result
    }
           flip(read_frame, read_frame, 1);
        imshow("BodyKeypointsDetect", read_frame);
    if (waitKey(1) != -1) {
      return 0;
    }
    nframe ++;
  }

  st_motion_body_destroy_tracker(body_handle);   // release the memory of body track handle

  ofs.close();

  return 0;
}
