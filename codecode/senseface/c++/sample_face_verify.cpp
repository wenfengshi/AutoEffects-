#include <stdio.h>
#include <cv_face.h>
#include "time_helper.h"

#include <opencv2/opencv.hpp>

#define DEFAULT_THRESHOLD (0.5)

using namespace std;
using namespace cv;


int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "test_sample_face_verify [input image1] [input image2]\n");
		fprintf(stderr,
			"for example: \"test_sample_face_verify face_01.jpg face_02.jpg\"\n");
		return -1;
	}
	//加载授权证书
	cv_result_t cv_result = cv_face_public_init_license("license.lic", "license");
	if(cv_result != CV_OK){
		fprintf(stderr, "cv_face_public_init_license error %d\n", cv_result);
		return -1;
	}
	
	// load image
	Mat bgr_image_1, bgr_image_2;
	bgr_image_1 = imread(argv[1]);      // CV_PIX_FMT_BGR888
	if (!bgr_image_1.data) {
		fprintf(stderr, "fail to read %s\n", argv[1]);
		return -1;
	}

	bgr_image_2 = imread(argv[2]);
	if (!bgr_image_2.data) {
		fprintf(stderr, "fail to read %s\n", argv[2]);
		return -1;
	}

	int main_return = -1;
	cv_handle_t handle_detect = NULL;
	cv_handle_t handle_verify = NULL;
	cv_face_t *p_face_1 = NULL;
	cv_face_t *p_face_2 = NULL;
	int face_count_1 = 0;
	int face_count_2 = 0;
	
	do {
		// init detect handle
		cv_result = cv_face_create_detector(&handle_detect, NULL, CV_DETECT_ENABLE_ALIGN_21);
		if (cv_result != CV_OK) {
			fprintf(stderr, "fail to init detect handle, error code %d\n", cv_result);
			break;
		}
		// detect
		cv_result = cv_face_detect(handle_detect, bgr_image_1.data, CV_PIX_FMT_BGR888,
			bgr_image_1.cols, bgr_image_1.rows, bgr_image_1.step,
			CV_FACE_UP, &p_face_1, &face_count_1);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect failed, error code : %d\n", cv_result);
			break;
		}
		cv_result = cv_face_detect(handle_detect, bgr_image_2.data, CV_PIX_FMT_BGR888,
			bgr_image_2.cols, bgr_image_2.rows, bgr_image_2.step,
			CV_FACE_UP, &p_face_2, &face_count_2);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect failed, error code : %d\n", cv_result);
			break;
		}

		// verify the first face
		if (face_count_1 > 0 && face_count_2 > 0) {
			// init verify handle
			cv_result = cv_verify_create_handle(&handle_verify, "../../models/verify.model");
			if (cv_result != CV_OK)
			{
				fprintf(stderr, "fail to init verify handle, error code %d\n", cv_result);
				break;
			}
			if (handle_verify) {
				int model_version = cv_verify_get_version(handle_verify);
				fprintf(stderr, "verify model version : %d\n", model_version);
				int feature_length = cv_verify_get_feature_length(handle_verify);
				fprintf(stderr, "verify model feature length : %d\n", feature_length);

				cv_feature_t *p_feature_1 = NULL, *p_feature_2 = NULL;
				float score;
				unsigned int feature_length_1 = 0, feature_length_2 = 0;
				// get feature
				__TIC__();
				cv_result = cv_verify_get_feature(handle_verify, bgr_image_1.data, CV_PIX_FMT_BGR888,
					bgr_image_1.cols, bgr_image_1.rows, bgr_image_1.step, p_face_1,
					&p_feature_1, &feature_length_1);
				__TOC__();
				if (cv_result != CV_OK) {
					fprintf(stderr, "cv_verify_get_feature failed, error code %d\n", cv_result);
					break;
				}
				cv_result = cv_verify_get_feature(handle_verify, bgr_image_2.data, CV_PIX_FMT_BGR888,
					bgr_image_2.cols, bgr_image_2.rows, bgr_image_2.step, p_face_2, 
					&p_feature_2, &feature_length_2);
				if (cv_result != CV_OK) {
					fprintf(stderr, "cv_verify_get_feature failed, error code %d\n", cv_result);
					break;
				}

				if (feature_length_1 > 0 && feature_length_2 > 0) {
					cv_feature_header_t *p_feature_header = CV_FEATURE_HEADER(p_feature_1);
					fprintf(stderr, "Feature information:\n");
					fprintf(stderr, "    ver:\t0x%08x\n", p_feature_header->ver);
					fprintf(stderr, "    length:\t%d bytes\n", p_feature_header->len);

					// compare feature
					cv_result = cv_verify_compare_feature(handle_verify, p_feature_1,
						p_feature_2, &score);
					if (cv_result == CV_OK) {
						fprintf(stderr, "score: %f\n", score);
						// comapre score with DEFAULT_THRESHOLD
						// > DEFAULT_THRESHOLD => the same person
						// < DEFAULT_THRESHOLD => different people
						if (score > DEFAULT_THRESHOLD)
							fprintf(stderr, "the same person.\n");
						else
							fprintf(stderr, "different people.\n");

						main_return = 0;  // success
					}
					else {
						fprintf(stderr, "cv_verify_compare_feature failed, error code : %d\n", cv_result);
					}

					// test serial and deserial
					char *string_feature_1 = new char[CV_ENCODE_FEATURE_SIZE(p_feature_1)];
					cv_verify_serialize_feature(p_feature_1, string_feature_1);
					cv_feature_t *p_feature_new_1 = cv_verify_deserialize_feature(string_feature_1);
					delete[]string_feature_1;
					char *string_feature_2 = new char[CV_ENCODE_FEATURE_SIZE(p_feature_2)];
					cv_verify_serialize_feature(p_feature_2, string_feature_2);
					cv_feature_t *p_feature_new_2 = cv_verify_deserialize_feature(string_feature_2);
					delete[]string_feature_2;
					score = 0.0;
					cv_result = cv_verify_compare_feature(handle_verify, p_feature_1,
						p_feature_2, &score);
					fprintf(stderr, "after serial and deserial the feature compare score is %f  \n", score);
					cv_verify_release_feature(p_feature_new_1);
					cv_verify_release_feature(p_feature_new_2);
				}
				else {
					fprintf(stderr, "error, the feature length is 0!\n");
				}
				// release the memory of feature
				cv_verify_release_feature(p_feature_1);
				cv_verify_release_feature(p_feature_2);
				
			}
		}
		else {
			if (face_count_1 == 0) {
				fprintf(stderr, "can't find face in %s\n", argv[1]);
			}
			if (face_count_2 == 0) {
				fprintf(stderr, "can't find face in %s\n", argv[2]);
			}
		}
	} while (0);


	// release the memory of face
	cv_face_release_detector_result(p_face_1, face_count_1);
	cv_face_release_detector_result(p_face_2, face_count_2);
	// destroy detect handle
	cv_face_destroy_detector(handle_detect);
	// destroy verify handle
	cv_verify_destroy_handle(handle_verify);
	fprintf(stderr, "test finish!\n");
	return main_return;
}

