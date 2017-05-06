#include <stdio.h>
#include <deque>
#include <opencv2/opencv.hpp>

#include "st_motion.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	VideoCapture capture;
	// VideoCapture capture("./videos/love.mp4");
	capture.open(0);
	if (!capture.isOpened()) {
		fprintf(stderr, "Hand tracker can not open camera!\n");
		return -1;
	}
	// set the frame size
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 1080);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

	st_result_t ret = ST_OK;
	st_handle_t hand_fingertip_handle = NULL;
	//加载授权证书
	ret = st_motion_public_init_license("license.lic", "license");
	if(ret != ST_OK){
		fprintf(stderr, "st_motion_public_init_license error %d\n", ret);
		return -1;
	}

	// create the fingertip track handle
	ret = st_motion_fingertip_create_tracker(&hand_fingertip_handle, "../../models/Fingertip_Track.model", frame_height, frame_width);
	if (ret != ST_OK) {
		fprintf(stderr, "create handle failed, ret = %d\n", ret);
		return -1;
	}

	deque<Point> recent_points;    // save the passed fingertip point for draw the motion trail
	Mat read_frame;
	while (capture.read(read_frame)) {
		st_motion_fingertip_t fingertip_info;
		ret = st_motion_fingertip_track(hand_fingertip_handle,
			read_frame.data, ST_PIX_FMT_BGR888, read_frame.cols, read_frame.rows, &fingertip_info);
		if (ret != ST_OK) {
			fprintf(stderr, "tracking failed, ret = %d\n", ret);
			break;
		}


		if (fingertip_info.is_valid) {
			recent_points.push_back(Point(fingertip_info.fingertip_point.x, fingertip_info.fingertip_point.y));
			if (recent_points.size() >= 100) {
				recent_points.pop_front();
			}

			rectangle(read_frame, Point(fingertip_info.hand_rect.left, fingertip_info.hand_rect.top),
				Point(fingertip_info.hand_rect.right, fingertip_info.hand_rect.bottom),
				Scalar(255, 0, 0), 3);
			deque<Point>::iterator it = recent_points.begin();
			Point start = *it;
			++it;
			while (it != recent_points.end()) {
				line(read_frame, start, *it, Scalar(0, 255, 0), 3);
				start = *it;
				++it;
			}
		}
		else {
			recent_points.clear();
		}

		flip(read_frame, read_frame, 1);
		imshow("TrackingTest", read_frame);
		if (waitKey(1) != -1)
			break;
	}

	st_motion_fingertip_destroy_tracker(hand_fingertip_handle);
	return 0;
}
