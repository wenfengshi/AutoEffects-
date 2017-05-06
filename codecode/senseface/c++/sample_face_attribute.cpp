#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "time_helper.h"
#include "cv_face.h"


using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "test_sample_face_attribute [input image] [threshold(0-100)]\n");
		fprintf(stderr, "for example: \"test_sample_face_attribute face_01.jpg 50\"\n");
		return -1;
	}
	int main_return = -1;
	char* image_path = argv[1];
	int threshold = 50;
	if (argc >= 3) {
		threshold = atoi(argv[2]);
	}

	//加载授权证书
	cv_result_t cv_result = cv_face_public_init_license("license.lic", "license");
	if(cv_result != CV_OK){
		fprintf(stderr, "cv_face_public_init_license error %d\n", cv_result);
		return -1;
	}

	// load image
	Mat bgr_image = imread(image_path);     // CV_PIX_FMT_BGR888
	if (!bgr_image.data) {
		fprintf(stderr, "fail to read %s\n", image_path);
		return -1;
	}

	cv_handle_t handle_detect = NULL;
	cv_handle_t handle_attribute = NULL;
	cv_face_t *p_face = NULL;
	int face_count = 0;

	do {
		// init detect handle
		cv_result = cv_face_create_detector(&handle_detect, NULL, CV_DETECT_ENABLE_ALIGN_21);
		if (cv_result != CV_OK) {
			fprintf(stderr, "fail to init detect handle, error code %d\n", cv_result);
			break;
		}
		// detect
		cv_result = cv_face_detect(handle_detect, bgr_image.data, CV_PIX_FMT_BGR888,
			bgr_image.cols, bgr_image.rows, bgr_image.step,
			CV_FACE_UP, &p_face, &face_count);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect error %d\n", cv_result);
			break;
		}

		// init attribute handle
		cv_result = cv_face_create_attribute_detector(&handle_attribute, "../../models/attribute.model");
		if (cv_result != CV_OK) {
			fprintf(stderr, "fail to init attribute handle, error code %d\n", cv_result);
			break;
		}
		if (face_count > 0) {
			for (int i = 0; i < face_count; i++) {
				int p_attribute_feature_result[CV_FEATURE_LENGTH] = { 0 };
				int p_attribute_emotion_result[CV_EMOTION_LENGTH] = { 0 };
				__TIC__();
				cv_result = cv_face_attribute_detect(handle_attribute, bgr_image.data, CV_PIX_FMT_BGR888,
					bgr_image.cols, bgr_image.rows, bgr_image.step,
					&(p_face[i]), p_attribute_feature_result, p_attribute_emotion_result);
				__TOC__();
				if (cv_result != CV_OK) {
					fprintf(stderr, "cv_face_attribute_detect error %d\n", cv_result);
					break;
				}
				else {
					// feature
					fprintf(stderr, "age: %d\n", p_attribute_feature_result[0]);
					fprintf(stderr, "gender: %s\n", p_attribute_feature_result[1] > threshold ? "male" : "female");
					fprintf(stderr, "attractive: %d\n", p_attribute_feature_result[2]);
					fprintf(stderr, "eyeglass: %s\n", p_attribute_feature_result[3] > threshold ? "yes" : "no");
					fprintf(stderr, "sunglass: %s\n", p_attribute_feature_result[4] > threshold ? "yes" : "no");
					fprintf(stderr, "smile: %s\n", p_attribute_feature_result[5] > threshold ? "yes" : "no");
					fprintf(stderr, "mask: %s\n", p_attribute_feature_result[6] > threshold ? "yes" : "no");
					switch (p_attribute_feature_result[7]){
					case 0:
						fprintf(stderr, "race: yellow\n");
						break;
					case 1:
						fprintf(stderr, "race: black\n");
						break;
					case 2:
						fprintf(stderr, "race: white\n");
						break;
					}
					fprintf(stderr, "eye open: %s\n", p_attribute_feature_result[8] > threshold ? "yes" : "no");
					fprintf(stderr, "mouth open: %s\n", p_attribute_feature_result[9] > threshold ? "yes" : "no");
					fprintf(stderr, "beard: %s\n", p_attribute_feature_result[10] > threshold ? "yes" : "no");

					// emotion
					int emotion_index = 0, max_score = 0;
					char* emotion[] = { "angry", "calm", "confused", "disgust", "happy", "sad", "scared", "surprised", "squint", "scream" };
					int emotion_size = sizeof(emotion) / sizeof(emotion[0]);
					for (int i = 0; i < emotion_size; i++) {
						if (p_attribute_emotion_result[i] > max_score){
							max_score = p_attribute_emotion_result[i];
							emotion_index = i;
						}
					}
					fprintf(stderr, "emotion : %s\n", emotion[emotion_index]);
				}
				main_return = 0;
			}
		}
		else {
			fprintf(stderr, "can't find face in %s", image_path);
		}
	} while (0);


	// release the memory of face
	cv_face_release_detector_result(p_face, face_count);
	// destroy attribute handle
	cv_face_destroy_attribute_detector(handle_attribute);
	// destroy detec handle
	cv_face_destroy_detector(handle_detect);

	fprintf(stderr, "test finish!\n");
	return main_return;
}

