#include <vector>
#include <stdio.h>
#include <cv_face.h>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "time_helper.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
  const int threshold = 50;
  // const std::vector<int> facemark = {52,53, 54,55, 56, 57,72, 73,74, 58,59,60,61,62,63,75,76,77,46};
  const std::vector<int> facemark = {74, 77, 52, 55, 58, 61, 46, 72, 75, 73, 76};
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

  int64_t nFrame = capture.get(CV_CAP_PROP_FRAME_COUNT);
  if (framelater) endframe = nFrame;

  // st_result_t ret = ST_OK;
  // if (argc < 2) {
  //   fprintf(stderr, "test_sample_face_track [alignment point size(21 or 106)] [detect face cont limit]\n");
  //   fprintf(stderr, "for example: \"test_sample_face_track 21 1\"\n");
  //   return -1;
  // }

  //加载授权证书
  cv_result_t cv_result = cv_face_public_init_license("license.lic", "license");
  if(cv_result != CV_OK){
    fprintf(stderr, "cv_face_public_init_license error %d\n", cv_result);
    return -1;
  }

  // cv::VideoCapture capture("./videos/eyes.mp4");
  // // capture.open(0);         // open the camera
  // if (!capture.isOpened()) {
  //   fprintf(stderr, "can not open camera!\n");
  //   return -1;
  // }
  //namedWindow("TrackingTest");
  int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

  int config = CV_DETECT_ENABLE_ALIGN_106;

  // int point_size = atoi(argv[1]);
  // int config;
  // if (point_size == 21) {
  //   config = CV_DETECT_ENABLE_ALIGN_21;
  // }
  // else if (point_size == 106) {
  //   config = CV_DETECT_ENABLE_ALIGN_106;
  // }
  // else {
  //   fprintf(stderr, "alignment point size must be 21 or 106\n");
  //   return -1;
  // }

  cv_handle_t handle_track = NULL;
  // cv_handle_t handle_attribute = NULL;

  do {
    // init handle
    cv_result = cv_face_create_tracker(&handle_track, NULL, config | CV_FACE_TRACKING_TWO_THREAD);
    if (cv_result != CV_OK) {
      fprintf(stderr, "cv_face_create_tracker failed, error code %d\n", cv_result);
      break;
    }

    // if (argc == 3) {
    //   int detect_face_cnt_limit = atoi(argv[2]);
    //   if (detect_face_cnt_limit < -1) {
    //     detect_face_cnt_limit = -1;
    //   }
    //   int val = 0;
    //   cv_result = cv_face_track_set_detect_face_cnt_limit(handle_track, detect_face_cnt_limit, &val);
    //   if (cv_result != CV_OK) {
    //     fprintf(stderr, "cv_face_track_set_detect_face_cnt_limit failed, error : %d\n", cv_result);
    //     break;
    //   } else {
    //     fprintf(stderr, "detect face count limit : %d\n", val);
    //   }
    // }

    Mat bgr_frame;
    cv_face_t *p_face = NULL;
    int face_count = 0;
    int nframe = 0;
    while (capture.read(bgr_frame)) {       // CV_PIX_FMT_BGR888
      ofs << nframe << std::endl;
      resize(bgr_frame, bgr_frame, Size(frame_width, frame_height), 0, 0,
        INTER_LINEAR);
      // realtime track
      face_count = 0;
      cv_result = cv_face_track(handle_track, bgr_frame.data, CV_PIX_FMT_BGR888,
        bgr_frame.cols, bgr_frame.rows, bgr_frame.step,
        CV_FACE_UP, &p_face, &face_count);
      if (cv_result != CV_OK) {
        fprintf(stderr, "cv_face_track failed, error : %d\n", cv_result);
        cv_face_release_tracker_result(p_face, face_count);
        break;
      }

      // cv_result = cv_face_create_attribute_detector(&handle_attribute, "../../models/attribute.model");
      // if (cv_result != CV_OK) {
      //  fprintf(stderr, "fail to init attribute handle, error code %d\n", cv_result);
      //  break;
      // }


      for (int i = 0; i < face_count; i++) {
        fprintf(stderr, "face: %d-----[%d, %d, %d, %d]-----id: %d\n", i,
          p_face[i].rect.left, p_face[i].rect.top,
          p_face[i].rect.right, p_face[i].rect.bottom, p_face[i].ID);
        fprintf(stderr, "face pose: [yaw: %.2f, pitch: %.2f, roll: %.2f, eye distance: %.2f]\n",
          p_face[i].yaw,
          p_face[i].pitch, p_face[i].roll, p_face[i].eye_dist);

        // draw the video
        //Scalar scalar_color = CV_RGB(p_face[i].ID * 53 % 256,
        //  p_face[i].ID * 93 % 256,
        //  p_face[i].ID * 143 % 256);
        //rectangle(bgr_frame, Point2f(static_cast<float>(p_face[i].rect.left),
        //  static_cast<float>(p_face[i].rect.top)),
        //  Point2f(static_cast<float>(p_face[i].rect.right),
        //  static_cast<float>(p_face[i].rect.bottom)), scalar_color, 2);
        // std::cout << "this" << p_face[i].points_count << std::endl;
        for (auto j = facemark.begin(); j != facemark.end(); j++){
        // for (int j = 0; j < p_face[i].points_count; j++) {
         // circle(bgr_frame, Point2f(p_face[i].points_array[*j].x,
          //  p_face[i].points_array[*j].y), 1, Scalar(0, 255, 0));
          ofs << p_face[i].points_array[*j].x << " " << p_face[i].points_array[*j].y << std::endl;
        }


        // int p_attribute_feature_result[CV_FEATURE_LENGTH] = { 0 };
        // int p_attribute_emotion_result[CV_EMOTION_LENGTH] = { 0 };
        // __TIC__();
        // cv_result = cv_face_attribute_detect(handle_attribute, bgr_frame.data, CV_PIX_FMT_BGR888,
        //  bgr_frame.cols, bgr_frame.rows, bgr_frame.step,
        //  &(p_face[i]), p_attribute_feature_result, p_attribute_emotion_result);
        // __TOC__();
        // if (cv_result != CV_OK) {
        //  fprintf(stderr, "cv_face_attribute_detect error %d\n", cv_result);
        //  break;
        // }
        // else {
        //  // feature
        //  fprintf(stderr, "age: %d\n", p_attribute_feature_result[0]);
        //  fprintf(stderr, "gender: %s\n", p_attribute_feature_result[1] > threshold ? "male" : "female");
        //  fprintf(stderr, "attractive: %d\n", p_attribute_feature_result[2]);
        //  fprintf(stderr, "eyeglass: %s\n", p_attribute_feature_result[3] > threshold ? "yes" : "no");
        //  fprintf(stderr, "sunglass: %s\n", p_attribute_feature_result[4] > threshold ? "yes" : "no");
        //  fprintf(stderr, "smile: %s\n", p_attribute_feature_result[5] > threshold ? "yes" : "no");
        //  fprintf(stderr, "mask: %s\n", p_attribute_feature_result[6] > threshold ? "yes" : "no");
        //  switch (p_attribute_feature_result[7]){
        //  case 0:
        //    fprintf(stderr, "race: yellow\n");
        //    break;
        //  case 1:
        //    fprintf(stderr, "race: black\n");
        //    break;
        //  case 2:
        //    fprintf(stderr, "race: white\n");
        //    break;
        //  }
        //  fprintf(stderr, "eye open: %s\n", p_attribute_feature_result[8] > threshold ? "yes" : "no");
        //  fprintf(stderr, "mouth open: %s\n", p_attribute_feature_result[9] > threshold ? "yes" : "no");
        //  fprintf(stderr, "beard: %s\n", p_attribute_feature_result[10] > threshold ? "yes" : "no");

        //  // emotion
        //  int emotion_index = 0, max_score = 0;
        //  char* emotion[] = { "angry", "calm", "confused", "disgust", "happy", "sad", "scared", "surprised", "squint", "scream" };
        //  int emotion_size = sizeof(emotion) / sizeof(emotion[0]);
        //  for (int i = 0; i < emotion_size; i++) {
        //    if (p_attribute_emotion_result[i] > max_score){
        //      max_score = p_attribute_emotion_result[i];
        //      emotion_index = i;
        //    }
        //  }
        //  fprintf(stderr, "emotion : %s\n", emotion[emotion_index]);
        // }
        


        
      }
      ofs << std::endl;
      // cv_face_create_attribute_detector()
      // release the memory of face
      cv_face_release_tracker_result(p_face, face_count);
      //imshow("TrackingTest", bgr_frame);
      if (waitKey(1) != -1)
        break;
      nframe++;
    }

  } while (0);

  // destroy track handle
  cv_face_destroy_tracker(handle_track);

  // cv_face_destroy_detector(handle_attribute);

  ofs.close();
  
  return 0;

}

