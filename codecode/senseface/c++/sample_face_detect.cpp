#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "time_helper.h"
#include "cv_face.h"


using namespace std;
using namespace cv;


int main(int argc, char *argv[]) {
	if (argc < 4) {
		fprintf(stderr, "test_sample_face_detect [input image] [out image] [alignment points(21 or 106)]\n");
		fprintf(stderr, "for example: \"test_sample_face_detect face_01.jpg out_face_detect.jpg  21\n");
		return -1;
	}

	char* input_image_path = argv[1];
	char* output_image_path = argv[2];
	int points_size = atoi(argv[3]);
	int config;
	if (points_size == 21) {
		config = CV_DETECT_ENABLE_ALIGN_21;
	}
	else if (points_size == 106) {
		config = CV_DETECT_ENABLE_ALIGN_106;
	}
	else {
		fprintf(stderr, "alignment point size error, must be 21 or 106\n");
		return -1;
	}
	//加载授权证书
	cv_result_t cv_result = cv_face_public_init_license("license.lic", "license");
	if(cv_result != CV_OK){
		fprintf(stderr, "cv_face_public_init_license error %d\n", cv_result);
		return -1;
	}

	// load image
	Mat bgr_image = imread(input_image_path);             // CV_PIX_FMT_BGR888
	if (!bgr_image.data) {
		fprintf(stderr, "fail to read %s\n", input_image_path);
		return -1;
	}
                   

	// init detect handle
	cv_handle_t handle_detect = NULL;
	cv_face_t* p_face = NULL;
	int face_count = 0;
	do 
	{
		cv_result = cv_face_create_detector(&handle_detect, NULL, config);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_create_detector failed, error code %d\n", cv_result);
			break;
		}

		/*
		 * test get and set threshold
		 */
		float default_threshold;
		cv_result = cv_face_detect_get_threshold(handle_detect, &default_threshold);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect_get_threshold failed, error code %d\n", cv_result);
			break;
		}
		fprintf(stderr, "default threshold : %f\n", default_threshold);

		cv_result = cv_face_detect_set_threshold(handle_detect, default_threshold);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect_set_threshold failed, error code %d\n", cv_result);
			break;
		}
		fprintf(stderr, "threshold set : %f\n", default_threshold);


		// detect
		__TIC__();
		cv_result = cv_face_detect(handle_detect, bgr_image.data, CV_PIX_FMT_BGR888,
			bgr_image.cols, bgr_image.rows, bgr_image.step,
			CV_FACE_UP, &p_face, &face_count);
		__TOC__();
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect error %d\n", cv_result);
			break;
		}

		if (face_count > 0) {
			// draw result
			for (int i = 0; i < face_count; i++) {
				rectangle(bgr_image, Point(p_face[i].rect.left, p_face[i].rect.top),
					Point(p_face[i].rect.right, p_face[i].rect.bottom),
					Scalar(0, 255, 0), 2, 8, 0);
				fprintf(stderr, "face number: %d\n", i + 1);
				fprintf(stderr, "face rect: [%d, %d, %d, %d]\n", p_face[i].rect.top,
					p_face[i].rect.left,
					p_face[i].rect.right, p_face[i].rect.bottom);
				fprintf(stderr, "score: %f\n", p_face[i].score);
				fprintf(stderr, "face pose: [yaw: %.2f, pitch: %.2f, roll: %.2f, eye distance: %.2f]\n",
					p_face[i].yaw,
					p_face[i].pitch, p_face[i].roll, p_face[i].eye_dist);
				fprintf(stderr, "face algin:\n");
				for (unsigned int j = 0; j < p_face[i].points_count; j++) {
					float x = p_face[i].points_array[j].x;
					float y = p_face[i].points_array[j].y;
					fprintf(stderr, "(%.2f, %.2f)\n", x, y);
					circle(bgr_image, Point2f(x, y), 2, Scalar(0, 0, 255), -1);
				}
				fprintf(stderr, "\n");
			}
			// save image
			imwrite(output_image_path, bgr_image);
		}
		else {
			fprintf(stderr, "can't find face in %s", input_image_path);
		}

	} while (0);
	

	// release the memory of face
	cv_face_release_detector_result(p_face, face_count);
	// destroy detect handle
	cv_face_destroy_detector(handle_detect);

	fprintf(stderr, "test finish!\n");
	return 0;
}

