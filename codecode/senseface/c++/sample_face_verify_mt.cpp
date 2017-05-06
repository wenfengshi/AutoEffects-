#include <stdio.h>
#include <vector>
#include "time_helper.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <cv_face.h>
#include <opencv2/opencv.hpp>
#define DEFAULT_THRESHOLD (0.5)

using namespace std;
using namespace cv;

struct THREAD_INFO {
	int thread_id;
	#ifdef _MSC_VER
	HANDLE task;
	#else
	pthread_t task;
	#endif
	cv_handle_t handle;
	char *image_path;
	cv_feature_t **feature;
};

static const int thread_pair_num = 1;

// thread function
#ifdef _MSC_VER
static DWORD WINAPI get_feature(LPVOID parame)
#else
void *get_feature(void *parame)
#endif
{
	struct THREAD_INFO *thread_info = (struct THREAD_INFO *)parame;
	cv_handle_t handle_detect = NULL;
	cv_face_t *p_face = NULL;
	int face_count = 0;
	cv_handle_t handle_verify = thread_info->handle;
	cv_feature_t *p_feature = NULL;
	unsigned int feature_length = 0;
	cv_result_t cv_result = CV_OK;

	do {
		// load image
		Mat bgr_image = imread(thread_info->image_path);
		if (!bgr_image.data) {
			fprintf(stderr, "fail to read %s\n", thread_info->image_path);
			break;
		}
		// init detector handle
		cv_result = cv_face_create_detector(&handle_detect, NULL, CV_DETECT_ENABLE_ALIGN_21);
		if (cv_result != CV_OK) {
			fprintf(stderr, "fail to init detect handle, error code %d thread %d\n", cv_result, thread_info->thread_id);
			break;
		}

		// face detector
		fprintf(stderr, "thread %d detector\n", thread_info->thread_id);
		cv_result = cv_face_detect(handle_detect, bgr_image.data, CV_PIX_FMT_BGR888,
			bgr_image.cols, bgr_image.rows, bgr_image.step,
			CV_FACE_UP, &p_face, &face_count);
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_face_detect error : %d\n", cv_result);
			break;
		}
		if (face_count == 0) {
			fprintf(stderr, "cv_face_detect : can not detect face in %s\n", thread_info->image_path);
			break;
		}

		
		// get feature
		fprintf(stderr, "thread %d get feature\n", thread_info->thread_id);
		__TIC__();
		cv_result = cv_verify_get_feature(handle_verify, bgr_image.data, CV_PIX_FMT_BGR888,
			bgr_image.cols,
			bgr_image.rows, bgr_image.step, &p_face[0], thread_info->feature,
			&feature_length);
		__TOC__();
		if (cv_result != CV_OK) {
			fprintf(stderr, "cv_verify_get_feature error : %d\n", cv_result);
		}
	} while (0);
	

	// release the memory of face
	cv_face_release_detector_result(p_face, face_count);
	// destroy detect handle
	cv_face_destroy_detector(handle_detect);
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "test_sample_face_verify_mt [input image1] [input image2]\n");
		fprintf(stderr,
			"for example: \"test_sample_face_verify_mt face_01.jpg face_02.jpg\"\n");
		return -1;
	}

	//加载授权证书
	cv_result_t cv_result = cv_face_public_init_license("license.lic", "license");
	if(cv_result != CV_OK){
		fprintf(stderr, "cv_face_public_init_license error %d\n", cv_result);
		return -1;
	}

	// init verify handle
	cv_handle_t handle_verify = NULL;
	std::vector<cv_handle_t> vec_handle_verify;
	cv_result = cv_verify_create_handle(&handle_verify, "../../models/verify.model");
	if (cv_result != CV_OK) {
		fprintf(stderr, "fail to init verify handle, error code %d\n", cv_result);
		return -1;
	}
	int model_version = cv_verify_get_version(handle_verify);
	fprintf(stderr, "verify model version : %d\n", model_version);

	const int thread_num = thread_pair_num * 2;
	THREAD_INFO thread_info[thread_num];
	cv_feature_t *feature[thread_num];
	for (int i = 0; i < thread_num; i++) {
		printf("start thread %d\n", i);
		// duplicate verify handle
		cv_handle_t handle_verify_new = NULL;
		cv_result = cv_verify_duplicate_handle(handle_verify, &handle_verify_new);
		if (cv_result != CV_OK) {
			fprintf(stderr, "fail to duplicate verify handle, error code %d\n", cv_result);
			break;
		}
		vec_handle_verify.push_back(handle_verify_new);
		thread_info[i].thread_id = i;
		thread_info[i].handle = handle_verify_new;
		thread_info[i].image_path = argv[(i % 2) + 1];
		thread_info[i].feature = &feature[i];
		#ifdef _MSC_VER
		thread_info[i].task = CreateThread(NULL, 0, get_feature,
						&thread_info[i], 0, NULL);
		#else
		pthread_create(&(thread_info[i].task), 0, get_feature, &thread_info[i]);
		#endif

	}
	for (int i = 0; i < thread_num; i++) {
		#ifdef _MSC_VER
		WaitForSingleObject(thread_info[i].task, INFINITE);
		CloseHandle(thread_info[i].task);
		#else
		pthread_join(thread_info[i].task, 0);
		#endif

		if ((i%2) == 0) continue;

		// verify
		if (feature[i - 1] && feature[i]) {
			fprintf(stderr, "feature verify\n");
			cv_result_t cv_result = CV_OK;
			float score = 0;
			cv_result = cv_verify_compare_feature(handle_verify, feature[i - 1], feature[i], &score);
			if (cv_result == CV_OK) {
				fprintf(stderr, "score: %f\n", score);
				// comapre score with DEFAULT_THRESHOLD
				// > DEFAULT_THRESHOLD => the same person
				// < DEFAULT_THRESHOLD => different people
				if (score > DEFAULT_THRESHOLD) {
					fprintf(stderr, "the same person.\n");
				} else {
					fprintf(stderr, "different people.\n");
				}
			} else {
				fprintf(stderr, "cv_verify_compare_feature error : %d\n", cv_result);
			}
		}
	}

	// release the memory of feature
	for (int i = 0; i < thread_num; i++) {
		cv_verify_release_feature(feature[i]);
	}
	// destroy verify handle
	for (int i = 0; i < vec_handle_verify.size(); ++i) {
		cv_verify_destroy_handle(vec_handle_verify[i]);
	}
	cv_verify_destroy_handle(handle_verify);
	fprintf(stderr, "test finish\n");
	return 0;
}

