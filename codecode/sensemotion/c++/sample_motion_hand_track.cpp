#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>

#include "st_motion.h"

using namespace cv;

#define HAND_TYPE_SIZE 8
// single thread tracker
#define SYNC_TRACK_OPTION "--sync"
// two tread tracker
#define ASYNC_TRACK_OPTION "--async"


#define TEST_RESET_OPTION "--test_reset"

static void print_cmd_help(const char *name){
	fprintf(stderr, "%s [%s, %s] [%s]\n", name,
		SYNC_TRACK_OPTION, ASYNC_TRACK_OPTION,
		TEST_RESET_OPTION);
	fprintf(stderr,
		"for example: \"%s %s %s\"\n", name, SYNC_TRACK_OPTION, TEST_RESET_OPTION);
}


int main(int argc, char **argv)
{
	if (argc < 2) {
		print_cmd_help(argv[0]);
		return -1;
	}
	char *p_model_path = "../../models/Hand_Track.model";

	bool test_reset = ((argc >= 3) && (0 == strcmp(argv[2], TEST_RESET_OPTION)));

	VideoCapture capture;
	if (!capture.open(0)) {
		fprintf(stderr, "Tack can not open camera!\n");
		return -1;
	}
	
	st_result_t ret = ST_OK;
	//加载授权证书
	ret = st_motion_public_init_license("license.lic", "license");
	if(ret != ST_OK){
		fprintf(stderr, "st_motion_public_init_license error %d\n", ret);
		return -1;
	}


	// set the frame size
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 1080);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

	// read the hand type images
	vector<Mat> vec_hand_images;
	vec_hand_images.resize(HAND_TYPE_SIZE);
	for (int i = 0; i < HAND_TYPE_SIZE; ++i) {
		char path[256];
		sprintf(path, "../test-images/%d.png", i + 1);
		Mat img = imread(path);
		if (img.empty()) {
			fprintf(stderr, "Can't load hand gesture images!\n");
			return -1;
		}
		vec_hand_images[i] = img.clone();
	}

	st_handle_t hand_track_handle = NULL;


	if (0 == strcmp(argv[1], SYNC_TRACK_OPTION)) {
		ret = st_motion_hand_create_tracker(&hand_track_handle, p_model_path, frame_width, frame_height, 0);
		if (ret != ST_OK) {
			fprintf(stderr, "Track create track handle failed, ret = %d\n", ret);
			return -1;
		}
	}
	else if (0 == strcmp(argv[1], ASYNC_TRACK_OPTION)) {
		ret = st_motion_hand_create_tracker(&hand_track_handle, p_model_path,
			frame_height, frame_width, ST_MOTION_TRACKING_ASYNC);
		if (ret != ST_OK) {
			fprintf(stderr, "Track create track handle failed, ret = %d\n", ret);
			return -1;
		}
	}
	else {
		print_cmd_help(argv[0]);
		return -1;
	}

	int frame_id = 0;
	Mat read_frame;

	while (capture.read(read_frame)) {
		frame_id++;
		if (test_reset && frame_id % 102 == 0) {
			frame_id = 0;
			printf("test_rest is enabled, now reset\n");
			st_motion_hand_reset_tracker(hand_track_handle);   // rest the hand track handle
		}

		st_motion_hand_t hand_info;
		ret = st_motion_hand_track(hand_track_handle,
			read_frame.data, ST_PIX_FMT_BGR888, read_frame.cols, read_frame.rows, &hand_info);
		if (ret != ST_OK) {
			fprintf(stderr, "st_motion_hand_track error : %d\n", ret);
			return -1;
		}

		if (hand_info.hand_type > -1 && hand_info.hand_type < HAND_TYPE_SIZE) {
			rectangle(read_frame, Point(hand_info.hand_rect.left, hand_info.hand_rect.top),
				Point(hand_info.hand_rect.right, hand_info.hand_rect.bottom), Scalar(0, 255, 0), 2);
			fprintf(stderr, "hand_type=%d\n", hand_info.hand_type);
			Mat &type_image = vec_hand_images[hand_info.hand_type];
			type_image.copyTo(read_frame(Rect(0, 0, type_image.cols, type_image.rows)));
		}

		flip(read_frame, read_frame, 1);
		imshow("Hand Tracking", read_frame);
		if (waitKey(1) != -1)
			break;
	}

	st_motion_hand_destroy_tracker(hand_track_handle);
}
