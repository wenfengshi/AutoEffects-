#include <vector>
#include <stdio.h>
#include <cv_face.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

static cv_handle_t handle_verify = NULL;
static cv_handle_t handle_db = NULL;
static cv_handle_t handle_detect = NULL;

#define _MAX_PATH 260

cv_feature_t *extract_feature(const char *image_path) {
	Mat bgr_image = imread(image_path);  // CV_PIX_FMT_BGR888
	if (!bgr_image.data) {
		return NULL;
	}
	
	cv_feature_t *p_feature = NULL;
	cv_face_t *p_face = NULL;
	int face_count = 0;
	cv_result_t st_result = CV_OK;
	st_result = cv_face_detect(handle_detect, bgr_image.data, CV_PIX_FMT_BGR888,
				bgr_image.cols, bgr_image.rows, bgr_image.step,
				CV_FACE_UP, &p_face, &face_count);
	if (face_count >= 1) {
		st_result = cv_verify_get_feature(handle_verify,
						(unsigned char *)bgr_image.data, CV_PIX_FMT_BGR888,
						bgr_image.cols, bgr_image.rows, bgr_image.step,
						p_face, &p_feature, NULL);
		if (st_result != CV_OK) {
			fprintf(stderr, "cv_verify_get_feature failed, error code %d\n", st_result);
		}
	} else {
		fprintf(stderr, "can't find face in %s", image_path);
	}
	// release the memory of face
	cv_face_release_detector_result(p_face, face_count);
	return p_feature;
}

int db_add(const char *image_path) {
	cv_feature_t *p_feature = extract_feature(image_path);
	if (!p_feature) {
		return -1;
	}
	int idx;
	cv_result_t cv_result = cv_verify_add_face(handle_db, p_feature, &idx);
	if (cv_result != CV_OK) {
		fprintf(stderr, "cv_verify_add_face failed, error code %d\n", cv_result);
	}
	cv_verify_release_feature(p_feature);
	return idx;
}

bool db_del(int idx) {
	if (idx < 0) {
		fprintf(stderr, "invalid idx!\n");
		return false;
	}
	cv_result_t cv_result = CV_OK;
	cv_result = cv_verify_delete_face(handle_db, idx);
	if (cv_result != CV_OK) {
		fprintf(stderr, "cv_verify_delete_face failed, error code %d\n", cv_result);
	}
	else {
		fprintf(stderr, "delete succeed\n");
	}
}

bool db_save(char *db_path) {
	cv_result_t cv_result = CV_OK;
	cv_result = cv_verify_save_db(handle_db, db_path);
	if (cv_result != CV_OK) {
		fprintf(stderr, "cv_verify_save_db failed, error code %d\n", cv_result);
		return false;
	}
	else {
		fprintf(stderr, "save done!\n");
	}

	return true;
}

bool db_load(char *db_path) {
	cv_result_t cv_result = CV_OK;
	cv_result = cv_verify_load_db(handle_db, db_path);
	if (cv_result != CV_OK) {
		fprintf(stderr, "cv_verify_load_db failed, error code %d\n", cv_result);
		return false;
	}
	else {
		fprintf(stderr, "load done!\n");
	}

	return true;
}

bool db_gen(char *image_list, char *output_db_path) {
	bool bresult = true;
	FILE *fp_path = fopen(image_list, "r");
	if(!fp_path) {
		fprintf(stderr, "failed to load %s\n", image_list);
		return false;
	}
	std::vector<cv_feature_t *> list_feature;
	list_feature.clear();
	for (;;) {
		char image_path[1024];
		int num = fscanf(fp_path, "%s", image_path);
		if (num != 1) {
			bresult = false;
			break;
		}
		fprintf(stderr, "extracting %s\n", image_path);

		// get the face feature
		cv_feature_t *p_feature = extract_feature(image_path);
		if (!p_feature) {
			fprintf(stderr, "failed to extract image: %s\n", image_path);
			continue;
		}
		list_feature.push_back(p_feature);
	}
	fclose(fp_path);
	cv_verify_destroy_db(handle_db);
	cv_result_t cv_result = CV_OK;
	cv_verify_create_db(&handle_db);
	cv_result = cv_verify_build_db(handle_db, &list_feature[0], list_feature.size());
	if (cv_result != CV_OK) {
		fprintf(stderr, "cv_verify_build_db failed, error code %d\n", cv_result);
		bresult = false;
	}
	cv_verify_save_db(handle_db, output_db_path);
	for (int i = 0; i < list_feature.size(); i++) {
		cv_verify_release_feature(list_feature[i]);
	}
	cout << "db gen done!" << endl;

	return bresult;
}

bool search_db(char *image_path) {
	FILE *fp_path = fopen(image_path, "r");
	if (fp_path == NULL) {
		fprintf(stderr, "invalid path !\n");
		return false;
	}
	fclose(fp_path);

	cv_feature_t *p_feature = extract_feature(image_path);
	if (p_feature == NULL) {
		fprintf(stderr, "extract failed !\n");
		return false;
	}

	int top_k = 10;
	int *top_idxs = new int[top_k];
	float *top_scores = new float[top_k];
	unsigned int result_length = 0;
	cv_result_t cv_result = cv_verify_search_face(handle_verify, handle_db,
		p_feature, top_k,
		top_idxs, top_scores, &result_length);
	if (cv_result == CV_OK) {
		for (unsigned int t = 0; t < result_length; t++) {
			// const cv_feature_t item = result[t].item;
			fprintf(stderr, "%d\t", top_idxs[t]);
			fprintf(stderr, "%0.2f\n", top_scores[t]);
		}
	}
	else {
		fprintf(stderr, "cv_verify_search_face failed, error code %d\n", cv_result);
	}
	if (top_idxs) {
		delete[]top_idxs;
	}
	if (top_scores) {
		delete[]top_scores;
	}
	cv_verify_release_feature(p_feature);

	return true;
}

bool search_list(char *image_path, char *list_path) {
	cv_feature_t *p_feature = extract_feature(image_path);
	if (p_feature == NULL) {
		fprintf(stderr, "failed to extract image: %s\n", image_path);
		return false;
	}

	FILE *fp_path = fopen(list_path, "r");
	if(!fp_path) {
		fprintf(stderr, "failed to load %s\n", list_path);
		return false;
	}
	std::vector<cv_feature_t *> list_feature;
	list_feature.clear();
	for (;;) {
		char image_path[_MAX_PATH];
		int num = fscanf(fp_path, "%s", image_path);
		if (num != 1) {
			break;
		}
		fprintf(stderr, "extracting %s\n", image_path);

		// get the face feature
		cv_feature_t *p_feature = extract_feature(image_path);
		if (!p_feature) {
			fprintf(stderr, "failed to extract image: %s\n", image_path);
			continue;
		}
		list_feature.push_back(p_feature);
	}
	fclose(fp_path);


	const int top_k = 10;
	int top_idxs[top_k];
	float top_scores[top_k];
	unsigned int result_length = 0;
	cv_result_t cv_result = cv_verify_search_face_from_list(handle_verify,
				&list_feature[0], list_feature.size(),
				p_feature, top_k,
				top_idxs, top_scores, &result_length);

	if (cv_result == CV_OK) {
		for (unsigned int t = 0; t < result_length; t++) {
			fprintf(stderr, "%d\t", top_idxs[t]);
			fprintf(stderr, "%0.2f\n", top_scores[t]);
		}
	} else {
		fprintf(stderr, "search face failed");
	}
	cv_verify_release_feature(p_feature);

	for (int i = 0; i < list_feature.size(); i++) {
		cv_verify_release_feature(list_feature[i]);
	}
	fprintf(stderr, "list search done!\n");

	return true;
}

void get_help() {
	fprintf(stderr, "Usage: help | Get cmd list\n");
	fprintf(stderr, "Usage: search p_image_colorpath | Search image in db\n");
	fprintf(stderr, "Usage: add p_image_colorpath | Add image in db, return idx in db, if idx < 0 means failed\n");
	fprintf(stderr, "Usage: del idx | Delete image in db\n");
	fprintf(stderr, "Usage: save db_file | Save current db in db_file\n");
	fprintf(stderr, "Usage: load db_file | Load db in db_file\n");
	fprintf(stderr, "Usage: gen p_image_colorlist db_file | Gen images in p_image_colorlist and save in db_file\n");
	fprintf(stderr, "Usage: exit | Exit the program\n");
}
int main(int argc, char *argv[]) {
	//加载授权证书
	cv_result_t cv_result = cv_face_public_init_license("license.lic", "license");
	if(cv_result != CV_OK){
		fprintf(stderr, "cv_face_public_init_license error %d\n", cv_result);
		return -1;
	}

    cv_result = cv_face_create_detector(&handle_detect, NULL, CV_DETECT_ENABLE_ALIGN_21);
	if (cv_result != CV_OK){
		fprintf(stderr, "create detect handle failed, error code %d\n", cv_result);
	}
	cv_result = cv_verify_create_handle(&handle_verify, "../../models/verify.model");
	if (cv_result != CV_OK){
		fprintf(stderr, "create verify handle failed, error code %d\n", cv_result);
	}
	cv_result = cv_verify_create_db(&handle_db);
	if (cv_result != CV_OK){
		fprintf(stderr, "create db handle failed, error code %d\n", cv_result);
	}
	
	if (handle_detect != NULL && handle_verify != NULL && handle_db != NULL) {
		// db_gen("list.txt","out.db");
		fprintf(stderr, "Database is empty at the beginning\n");
		fprintf(stderr, "Please input 'help' to get the cmd list\n");
		char input_code[256];
		char image_path[_MAX_PATH];
		char db_path[_MAX_PATH];
		char command[256];
		input_code[0] = 0;
		while (1) {
			fprintf(stderr, ">>");
			if (!fgets(input_code, 256, stdin)) {
				fprintf(stderr, "read nothing\n");
				continue;
			}
			int input_length = strlen(input_code);
			if (input_length > 0 && input_code[input_length - 1] == '\n') {
				input_code[--input_length] = 0;
			}
			if (input_length == 0) {
				continue;
			}
			std::string str_input_code(input_code);
			if (strcmp(str_input_code.c_str(), "help") == 0) {
				get_help();
			}
			else if (strcmp(str_input_code.substr(0, 3).c_str(), "add") == 0) {
				int input_number = sscanf(input_code, "%s%s", command, image_path);
				if (input_number != 2) {
					fprintf(stderr, "invalid! Usage: add p_image_colorpath\n");
					continue;
				}
				int idx = db_add(image_path);
				cout << "idx = " << idx << endl;
			}
			else if (strcmp(str_input_code.substr(0, 3).c_str(), "del") == 0) {
				int idx = -1;
				int input_number = sscanf(input_code, "%s%d", image_path, &idx);
				if (input_number != 2) {
					fprintf(stderr, "invalid! Usage: del idx(unsigned int\n");
					continue;
				}
				db_del(idx);
			}
			else if (strcmp(str_input_code.substr(0, 4).c_str(), "save") == 0) {
				int input_number = sscanf(input_code, "%s%s", command, image_path);
				if (input_number != 2) {
					fprintf(stderr, "invalid! Usage: save db_file\n");
					continue;
				}
				db_save(image_path);
			}
			else if (strcmp(str_input_code.substr(0, 4).c_str(), "load") == 0) {
				int input_number = sscanf(input_code, "%s%s", command, image_path);
				if (input_number != 2) {
					fprintf(stderr, "invalid! Usage: load db_file\n");
					continue;
				}
				db_load(image_path);
			}
			else if (strcmp(str_input_code.c_str(), "exit") == 0) {
				break;
			}
			else if (strcmp(str_input_code.substr(0, 3).c_str(), "gen") == 0) {
				int input_number = sscanf(input_code, "%s%s%s", command, image_path, db_path);
				if (input_number != 3) {
					fprintf(stderr, "invalid! Usage: gen p_image_colorlist_file db_file\n");
					continue;
				}
				db_gen(image_path, db_path);
			}
			else if (strcmp(str_input_code.substr(0, 6).c_str(), "search") == 0) {
				int input_number = sscanf(input_code, "%s%s", command, image_path);
				if (input_number != 2) {
					fprintf(stderr, "invalid! Usage: search p_image_colorpath\n");
					continue;
				}
				search_db(image_path);
			}
			else if (strcmp(str_input_code.substr(0, 10).c_str(), "listsearch") == 0) {
				char search_path[_MAX_PATH];
				int input_number = sscanf(input_code, "%s%s%s", command, search_path,
					image_path);
				if (input_number != 3) {
					fprintf(stderr, "invalid! Usage: listsearch p_image_colorsrcpath p_image_colorlistpath\n");
					continue;
				}
				search_list(search_path, image_path);
			}
			else {
				fprintf(stderr, "invalid cmd, please input 'help' to get the cmd list\n");
			}
		}
	}

	cv_face_destroy_detector(handle_detect);
	cv_verify_destroy_db(handle_db);
	cv_verify_destroy_handle(handle_verify);
	return 0;
}

