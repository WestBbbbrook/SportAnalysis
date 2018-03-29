//
// Created by xin on 18-1-19.
//

#include "synopsis.h"
#include <map>

/**
*基本数据单元构造函数
*/
tube::tube(CvRect rect, int t, IplImage * img) :position(rect), t_sec(t) {
	target = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
	cvCopy(img, target);
}

/**
*基本数据单元析构函数
*/
tube::~tube() {
	cvReleaseImage(&target);
}

/**
*判断两矩形是否重叠
*/
bool isOverlap(const CvRect & a, const CvRect & b) {
	const CvRect * l_rect = &a,
		*r_rect = &b;
	if (a.x > b.x) {
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	if (l_rect->width < r_rect->x - l_rect->x)
		return false;
	else if (l_rect->y <= r_rect->y && l_rect->height >= r_rect->y - l_rect->y)
		return true;
	else if (l_rect->y > r_rect->y && r_rect->height >= l_rect->y - r_rect->y)
		return true;
	else
		return false;
}



/**
*前景跟踪并建立数据库结构 - GMM
*/
void buildTrackDB_GMM(CvCapture * capture, const IplImage * bgImg, list< list<tube *> > & database, \
	const int threshold, const int min_area, const int obj_num, const float extend_factor, \
	const float category_factor, const bool save_mode) {
	if (bgImg == NULL) {
		printf("Fail: NULL background image!\n");
		return;
	}

	if (bgImg->nChannels != 3) {
		printf("Fail: Background image should be grayscale!\n");
		return;
	}

	if (!database.empty()) {
		printf("Fail: Database is not empty!\n");
		return;
	}

	if (extend_factor < 0 || extend_factor > 1 || category_factor < 0 || category_factor > 1) {
		printf("Fail: Invalid use of params!\n0 <= extend_factor <= 1\n0 <= category_factor <= 1\n");
		return;
	}

	//CvCapture * capture = cvCaptureFromFile(videoFilePath);
	//CvCapture * capture = cvCreateCameraCapture(0);//打开本地视频

	if (!capture) {
		printf("Fail: Unable to open video file!\n");
		return;
	}
	int frame_id = 0;
	CvSize size;// = cvSize((int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT));
	size.width = 640; size.height = 480;
	IplImage * frame = NULL;

	//以下变量用于轮廓查找函数
	//CvMemStorage * storage = cvCreateMemStorage();
	CvSeq * first_contour = NULL;

	int find_num = 0,
		extend_w = 0,
		extend_h = 0;
	list<CvRect> rects; //存放找到的前景块位置
	char time[10]; //存放时间
	int m = 0, s = 0; //分、秒
	int last_find_num = 0;

	printf("Build track database...\n");
	while (frame = cvQueryFrame(capture)) {
		Mat frame_mat;
		//<CvRect> rects; //存放找到的前景块位置
		vector<Point2d> centers; //存放找到的前景块中心
		//vector<vector<Point> > contours;
		//vector<Vec4i> hierarchy;

		vector<Mat> contours;
		Mat hierarchy;

		//frame_mat = cvarrToMat(tmp,true);
		//cvAbsDiff(bgImg, frame, frame1);

		frame_mat = cvarrToMat(frame, true);
		Mat frame_mat1 = cvarrToMat(bgImg, true);

		Mat edges;
		if (frame == NULL)
		{
			break;
		}
		//detectEdge_KNN(frame, m_KNN, edges);
		//detectEdge_KNN2(frame_mat, frame_mat1, edges);
		edges = MoveDetect(frame_mat, frame_mat1);

		findContours(edges, contours, hierarchy, CV_RETR_CCOMP, CHAIN_APPROX_NONE);
		last_find_num = find_num;
		find_num = contours.size();

		imshow("KNN前景", edges);
		/**!!!Be careful with the param below!!!探测数量上限**/
		if (find_num > (obj_num * 3) || frame == NULL) {
			continue;
		}

		printf("frame:%d\tfind contour num:%d\n", frame_id, find_num);

		//contour analysis
		if (contours.size()>0)
		{
			for (int i = 0; i < contours.size(); i++)
			{
				Rect r = boundingRect(contours[i]);
				if (r.height * r.width > 2000) {
					rects.push_back(r);
				}
				//centers.push_back((r.br()+r.tl())*0.5);
			}
		}
		cout << "contours.size" << contours.size();

		cvShowImage("frame", frame);
		//cvShowImage("fgImg", fgImg);
		cvWaitKey(10);
		frame_id += 1;
		rects.clear();
		//cvClearMemStorage(storage);
		//cvReleaseMemStorage(&storage);
		//cvReleaseImage(&tmp);

	}
	//释放内存
	//cvReleaseMemStorage(&storage);
}



//帧差法
int CarNum = 0;

Mat MoveDetect(Mat frame1, Mat frame2) {
	Mat result = frame2.clone();
	Mat gray1, gray2;
	cvtColor(frame1, gray1, CV_BGR2GRAY);
	cvtColor(frame2, gray2, CV_BGR2GRAY);

	Mat diff;
	absdiff(gray1, gray2, diff);
	//imshow("absdiss", diff);
	threshold(diff, diff, 25, 255, CV_THRESH_BINARY);
	imshow("threshold", diff);

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element2 = getStructuringElement(MORPH_RECT, Size(19, 19));
	erode(diff, diff, element);
	//imshow("erode", dst);
	medianBlur(diff, diff, 3);
	imshow("medianBlur", diff);
	dilate(diff, diff, element2);
	//imshow("dilate", diff);
	result = diff;
	return result;

}

