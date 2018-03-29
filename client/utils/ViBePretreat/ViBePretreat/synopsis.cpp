
#include "synopsis.h"
#include <map>

//#include <unistd.h>


//static Ptr<BackgroundSubtractorMOG2> m_GMM = createBackgroundSubtractorMOG2();
//static Ptr<BackgroundSubtractorKNN> m_KNN = createBackgroundSubtractorKNN(100, 200);//history = 500, dist2Thres = 500

/**
*背景建模
*/
void bgModeling(const char * videoFilePath, const char* outBgImgPath, const int frame_num_used, IplImage ** bgImg, \
	int& fps, int& frame_number, CvSize& size, int& code, const int size1, const int size2, const int sigma1, const int sigma2) {
	//声明
	IplImage * frame = NULL;
	CvMat * frameMat = NULL;
	CvMat * bgMat = NULL;
	CvCapture* pCapture = NULL;
	IplImage * bgColorImg = NULL;
	int frame_no = 0;
	//函数自己初始化，避免 不必要的Bug，值得学习。
	//直接指针处理避免不必要的传值&多余操作。
	pCapture = cvCaptureFromFile(videoFilePath);//自己选取一段avi视频


	if (!pCapture) {
		printf("Unable to open video file for background modeling!\n");
		return;
	}

	if (*bgImg != NULL) {//非空需先清空*bgImg指向的内存
		cvReleaseImage(bgImg);
	}

	fps = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);
	frame_number = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT);
	code = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FOURCC);
	size.width = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	size.height = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);

	printf("Background Modeling...\n");
	//CvVideoWriter * writer = cvCreateVideoWriter(outFilePath, CV_FOURCC('h', '2', '6', '4'), fps, size, 1);

	//逐帧读取视频
	while (frame_no < frame_num_used) {
		frame = cvQueryFrame(pCapture);
		frame_no += 1;
		//cvWriteFrame(writer, frame); //写帧
		cout << "a" << endl;
		if (frame_no == 1) {
			cout << "zzzzzzzzzzzzzzz" << endl;
			//初始化
			bgColorImg = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, frame->nChannels);
			cvCopy(frame, bgColorImg);
			frameMat = cvCreateMat(frame->height, frame->width, CV_32FC3);
			bgMat = cvCreateMat((bgColorImg)->height, (bgColorImg)->width, CV_32FC3);
			cvConvert(frame, frameMat);
			cvConvert(bgColorImg, bgMat);
			continue;
		}
		//视频帧IplImage转CvMat
		cout << "b" << endl;
		try{
			cvConvert(frame, frameMat);
			cout << "c" << endl;
			
		}
		catch (Exception e2){
			break;
		}
		//高斯滤波先，以平滑图像
		cvSmooth(frame, frame, CV_GAUSSIAN, size1, size2, sigma1, sigma2);
		cout << "d" << endl;
		//滑动平均更新背景(求平均)
		cvRunningAvg(frameMat, bgMat, (double)1 / frame_num_used);
		cout << "e" << endl;

	}
	cout << "f" << endl;
	getchar();
	cvConvert(bgMat, bgColorImg);
	//cvShowImage("bgColorImg", bgColorImg);
	waitKey(10);
	cvSaveImage("C:/Users/chenteng/Desktop/bgColorImg.jpg", bgColorImg);
	printf("Background Model has been achieved!\n");
	/*
	*bgImg = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvCvtColor(bgColorImg, *bgImg, CV_BGR2GRAY); //帧图像彩色转灰度
	*/

	*bgImg = bgColorImg;
	//释放内存
	cvReleaseCapture(&pCapture);
	cvReleaseMat(&frameMat);
	cvReleaseMat(&bgMat);

	//cvReleaseVideoWriter(&writer);
}


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
*提取前景图像 - 二值图像
*/
void getFgImage(const IplImage * frame, IplImage * fgImg, const IplImage * bgImg, const int threshold) {
	if (frame == NULL || fgImg == NULL || bgImg == NULL) {
		printf("Fail: There exists NULL input. Fail to get Foreground Image!\n");
		return;
	}

	if (frame->nChannels != 3 || fgImg->nChannels != 3 || bgImg->nChannels != 3) {
		printf("Fail: All input image should be color!\nframe channel:%d\nfgImg channel:%d\nbgImg channel:%d\n", \
			frame->nChannels, fgImg->nChannels, bgImg->nChannels);
		return;
	}

	CvMat * frameMat = cvCreateMat(frame->height, frame->width, CV_32FC3);
	CvMat * fgMat = cvCreateMat(fgImg->height, fgImg->width, CV_32FC3);
	CvMat * bgMat = cvCreateMat(bgImg->height, bgImg->width, CV_32FC3);

	cvConvert(frame, frameMat);
	cvConvert(fgImg, fgMat);
	cvConvert(bgImg, bgMat);

	cvSmooth(frameMat, frameMat, CV_GAUSSIAN, 5, 5, 4, 4); //高斯滤波先，以平滑图像

	cvAbsDiff(frameMat, bgMat, fgMat); //当前帧跟背景图相减(求背景差并取绝对值)

	cvThreshold(fgMat, fgImg, threshold, 255, CV_THRESH_BINARY); //二值化前景图(这里采用特定阈值进行二值化)

	//进行形态学滤波，去掉噪音
	cvErode(fgImg, fgImg, 0, 1);
	cvDilate(fgImg, fgImg, 0, 1);

	//释放矩阵内存
	cvReleaseMat(&frameMat);
	cvReleaseMat(&fgMat);
	cvReleaseMat(&bgMat);
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
*前景位置矩形块合并 - 减少单元数
*/
void mergeRects(list<CvRect> & rects) {
	int x = 0, y = 0, width = 0, height = 0;//临时变量
	for (list<CvRect>::iterator i = rects.begin(); i != rects.end();) {
		bool merged = false;//多引入一个变量判断i是否被merge非常有用！
		list<CvRect>::iterator j = i;
		for (j++; j != rects.end(); j++) {
			if (isOverlap(*i, *j)) {
				if (i->x < j->x) {
					x = i->x;
					width = max(j->x - i->x + j->width, i->width);
				}
				else {
					x = j->x;
					width = max(i->x - j->x + i->width, j->width);
				}

				if (i->y < j->y) {
					y = i->y;
					height = max(j->y - i->y + j->height, i->height);
				}
				else {
					y = j->y;
					height = max(i->y - j->y + i->height, j->height);
				}

				//合并
				j->x = x;
				j->y = y;
				j->width = width;
				j->height = height;

				i = rects.erase(i);//删除被合并项。注意：删除前者（i）更新后者（j）！
				merged = true;
			}
		}
		if (!merged)
			i++;
	}
}


/**
*判断是否属于同一跟踪目标
*/
bool isSameObj(const CvRect & a, const CvRect & b, const float category_factor) {
	const CvRect * l_rect = &a,
		*r_rect = &b;
	if (a.x > b.x) {
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	int area = 0;//记录重叠区域面积
	if (l_rect->width <= r_rect->x - l_rect->x)
		return false;
	else if (l_rect->y <= r_rect->y && l_rect->height > r_rect->y - l_rect->y) {
		area = (l_rect->x + l_rect->width - r_rect->x) * (l_rect->y + l_rect->height - r_rect->y);
		if (area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else
			return false;
	}
	else if (l_rect->y >= r_rect->y && r_rect->height > l_rect->y - r_rect->y) {
		area = (l_rect->x + l_rect->width - r_rect->x) * (r_rect->y + r_rect->height - l_rect->y);
		if (area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else
			return false;
	}
	else
		return false;
}


/**
*前景跟踪并建立数据库结构 - Kernel
*/
void buildTrackDB(const char * videoFilePath, const IplImage * bgImg, list< list<tube *> > & database, \
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

	CvCapture * capture = cvCaptureFromFile(videoFilePath);
	if (!capture) {
		printf("Fail: Unable to open video file!\n");
		return;
	}
	int fps = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS),
		frame_count = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT),
		frame_id = 0;
	CvSize size = cvSize((int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
		(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT));

	IplImage * frame = NULL,
		*frame_gray = cvCreateImage(size, IPL_DEPTH_8U, 1),
		*fgImg = cvCreateImage(cvSize(bgImg->width, bgImg->height), bgImg->depth, bgImg->nChannels);

	//以下变量用于轮廓查找函数
	CvMemStorage * storage = cvCreateMemStorage();
	CvSeq * first_contour = NULL;

	int find_num = 0,
		extend_w = 0,
		extend_h = 0;
	list<CvRect> rects; //存放找到的前景块位置
	char time[10]; //存放时间
	int m = 0, s = 0; //分、秒

	printf("Build track database...\n");
	while (frame = cvQueryFrame(capture)) {
		IplImage * tmp = cvCreateImage(size, IPL_DEPTH_8U, 1);
		//cvCvtColor(frame, frame_gray, CV_BGR2GRAY); //帧图像彩色转灰度
		getFgImage(frame, fgImg, bgImg, threshold); //帧差法获取前景图像
		//cvShowImage("fg", fgImg);
		//cvWaitKey();
		cvCvtColor(fgImg, tmp, CV_BGR2GRAY); //帧图像彩色转灰度
		//cvCopy(fgImg, tmp);
		find_num = cvFindContours(tmp, storage, &first_contour, sizeof(CvContour), CV_RETR_CCOMP); //检测前景轮廓

		/**!!!Be careful with the param below!!!**/
		if (find_num > (obj_num * 10) || first_contour == NULL || frame == NULL) {
			cvClearMemStorage(storage);
			cvReleaseImage(&tmp);
			continue;
		}

		printf("frame:%d\tfind contour num:%d\n", frame_id, find_num);
		//contour analysis
		for (CvSeq* c = first_contour; c != NULL; c = c->h_next) {
			int left = 0x7fffffff, right = 0, top = 0x7fffffff, bottom = 0;
			int contourWidth = 0,
				contourHeight = 0;
			for (int j = 0; j < c->total; j++) {
				CvPoint * pt = (CvPoint *)cvGetSeqElem(c, j);
				if (pt->x < left)
					left = pt->x;
				if (pt->x > right)
					right = pt->x;
				if (pt->y < top)
					top = pt->y;
				if (pt->y >bottom)
					bottom = pt->y;
			}
			contourWidth = right - left + 1;
			contourHeight = bottom - top + 1;
			if (contourWidth * contourHeight < min_area)
				continue;
			extend_w = extend_factor * contourWidth;
			extend_h = extend_factor * contourHeight;
			left = (left >= extend_w ? left - extend_w : 0);
			top = (top >= extend_h ? top - extend_h : 0);
			right = ((right + extend_w) < size.width ? right + extend_w : size.width - 1);
			bottom = ((bottom + extend_h) < size.height ? bottom + extend_h : size.height - 1);
			rects.push_back(cvRect(left, top, right - left + 1, bottom - top + 1));
		}

		mergeRects(rects); //merge rects进一步筛选

		for (list<CvRect>::iterator iter = rects.begin(); iter != rects.end(); iter++) {
			//cvRectangle(frame, cvPoint(iter->x, iter->y), cvPoint(iter->x + iter->width, iter->y + iter->height),\
																		cvScalar(0,255,0), 1);//在frame中标示出最后保存的方框区域
			m = (frame_id / fps) / 60;
			s = (frame_id / fps) % 60;
			sprintf(time, "%02d:%02d", m, s);
			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5, 1, 2, 8);
			cvPutText(frame, time, cvPoint(iter->x + iter->width / 3, iter->y + iter->height / 3), \
				&font, cvScalar(0, 0, 255));//在frame中标记时间
			cvSetImageROI(frame, *iter);
			IplImage * target = cvCreateImage(cvSize((*iter).width, (*iter).height), frame->depth, frame->nChannels);
			cvCopy(frame, target);

			bool added = false;//判断通过下面的循环是否将tube加入到database
			for (list< list<tube *> >::iterator i = database.begin(); i != database.end(); i++) {
				list<tube *>::iterator j = i->end();//如果 *i 为空则应该到 database 末端了，循环应已终止
				j--;//找到list<tube *>最后一个元素，及该目标的最新跟踪数据
				if (isSameObj(*iter, (*j)->position, category_factor)) {//判断是同一跟踪目标
					i->push_back(new tube(*iter, frame_id / fps, target));
					added = true;
					break;
				}
			}
			if (!added)//扫描现有database未发现匹配项则新建跟踪目标链表
				database.push_back(list<tube *>(1, new tube(*iter, frame_id / fps, target)));

			cvResetImageROI(frame);
			cvReleaseImage(&target);
		}

		//The following 3 lines are for debuging, comment them when debug is done!
		//cvShowImage("frame", frame);
		//cvShowImage("fgImg", fgImg);
		//cvWaitKey(40);

		frame_id += 1;
		rects.clear();
		cvClearMemStorage(storage);
		cvReleaseImage(&tmp);
	}
	printf("Track DB has been saved! Total frame num:%d\tValid frame:%d\n", frame_count, frame_id);


	//释放内存
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&frame_gray);
}

/**
*前景跟踪并建立数据库结构 - GMM
*/
void buildTrackDB_GMM(const char * videoFilePath, const IplImage * bgImg, list< list<tube *> > & database, \
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
	CvCapture * capture = cvCreateCameraCapture(0);//打开本地视频
	if (!capture) {
		printf("Fail: Unable to open video file!\n");
		return;
	}
	int fps = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS),
		//frame_count = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT),
		frame_id = 0;
	CvSize size;// = cvSize((int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT));
	size.width = 640; size.height = 480;
	IplImage * frame = NULL,
		*frame_gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
	//  *fgImg = cvCreateImage(cvSize(bgImg->width, bgImg->height), bgImg->depth, bgImg->nChannels);

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

		//for(int i=0; i<centers.size(); i++)
		//{
		//    circle(frame_mat,centers[i],3,Scalar(0,255,0),1,CV_AA);
		// }
		/*
		mergeRects(rects); //merge rects进一步筛选

		for (list<CvRect>::iterator iter = rects.begin(); iter != rects.end(); iter++) {
		cvRectangle(frame, cvPoint(iter->x, iter->y), cvPoint(iter->x + iter->width, iter->y + iter->height), \
		cvScalar(0, 255, 0), 1);//在frame中标示出最后保存的方框区域
		m = (frame_id / fps) / 60;
		s = (frame_id / fps) % 60;
		sprintf(time, "%02d:%02d", m, s);
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5, 1, 2, 8);
		cvPutText(frame, time, cvPoint(iter->x + iter->width / 3, iter->y + iter->height / 3), \
		&font, cvScalar(0, 0, 255));//在frame中标记时间
		cvSetImageROI(frame, *iter);
		IplImage * target = cvCreateImage(cvSize((*iter).width, (*iter).height), frame->depth, frame->nChannels);
		cvCopy(frame, target);

		bool added = false;//判断通过下面的循环是否将tube加入到database

		for (list< list<tube *> >::iterator i = database.begin(); i != database.end(); i++) {
		list<tube *>::iterator j = i->end();//如果 *i 为空则应该到 database 末端了，循环应已终止
		j--;//找到list<tube *>最后一个元素，及该目标的最新跟踪数据

		if (isSameObj(*iter, (*j)->position, category_factor)) {//判断是同一跟踪目标
		i->push_back(new tube(*iter, frame_id / fps, target));
		added = true;
		break;
		}

		}
		if (!added){
		database.push_back(list<tube *>(1, new tube(*iter, frame_id / fps, target)));
		}//扫描现有database未发现匹配项则新建跟踪目标链表


		cvResetImageROI(frame);
		cvReleaseImage(&target);
		}
		*/
		//The following 3 lines are for debuging, comment them when debug is done!
		cvShowImage("frame", frame);
		//cvShowImage("fgImg", fgImg);
		cvWaitKey(10);

		frame_id += 1;
		rects.clear();
		//cvClearMemStorage(storage);
		//cvReleaseMemStorage(&storage);
		//cvReleaseImage(&tmp);

	}
	//	printf("Track DB has been saved! Total frame num:%d\tValid frame:%d\n", frame_count, frame_id);



	//释放内存
	//cvReleaseMemStorage(&storage);

	cvReleaseImage(&frame_gray);
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
	/*
	vector<vector<Point>> contours;
	vector<Vec4i> hierarcy;
	findContours(diff, contours, hierarcy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));//查找轮廓
	vector<vector<Point>>contours_poly(contours.size());
	vector<Rect> boundRect(contours.size()); //定义外接矩形集合
	//drawContours(img2, contours, -1, Scalar(0, 0, 255), 1, 8);  //绘制轮廓
	int x0 = 0, y0 = 0, w0 = 0, h0 = 0;
	for (int i = 0; i<contours.size(); i++)
	{
	approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);//对图像轮廓点进行多边形拟合：轮廓点组成的点集，输出的多边形点集，精度（即两个轮廓点之间的距离），输出多边形是否封闭
	boundRect[i] = boundingRect(Mat(contours_poly[i]));
	if (boundRect[i].width>55 && boundRect[i].width<180 && boundRect[i].height>55 && boundRect[i].height<180) {//轮廓筛选
	x0 = boundRect[i].x;
	y0 = boundRect[i].y;
	w0 = boundRect[i].width;
	h0 = boundRect[i].height;

	rectangle(result, Point(x0, y0), Point(x0 + w0, y0 + h0), Scalar(0, 255, 0), 2, 8, 0);
	if ((y0 + h0 / 2 + 1) >= 138 && (y0 + h0 / 2 - 1) <= 142) {//经过这条线（区间），车辆数量+1
	CarNum++;

	}

	}
	line(result, Point(0, 140), Point(568, 140), Scalar(0, 0, 255), 1, 8);//画红线
	Point org(0, 35);
	//putText(result, "CarNum=" + intToString(CarNum), org, CV_FONT_HERSHEY_SIMPLEX, 0.8f, Scalar(0, 255, 0), 2);
	}
	*/
	return result;

}








/**
*融合跟踪数据库，输出视频摘要，同时释放Database内存
*/
void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg) {

	/**
	*Tube seq database重组优化
	*/

	//从这里开始
	if (database.empty()) {
		printf("Fail: Database is empty, cannot merge!\n");
		return;
	}

	if (videoFilePath == NULL) {
		printf("Fail: NULL output file path/name!\n");
		return;
	}

	CvVideoWriter * writer = cvCreateVideoWriter(videoFilePath, CV_FOURCC('M', 'J', 'P', 'G'), fps, size, 1);
	if (writer == NULL) {
		printf("Fail: Invalid video file path/name, cannot build a video writer!\n");
		return;
	}

	IplImage * frame = cvCreateImage(size, IPL_DEPTH_8U, 3); //输出帧
	tube * tmp = NULL; //临时tube指针
	vector<CvRect> rects; //保存每帧被填充的位置
	vector<CvRect> rects_last;
	vector<CvRect> temp;
	int frame_id = 0;

	printf("Merging track database...\n");


	while (!database.empty()) {

		cvCopy(bgImg, frame); //每次开始用背景图片刷新输出帧
		//list< list<tube *> >::iterator j;
		for (size_t t = 0; t != temp.size(); t++) { //检查是否与已经填充的有重叠
			rects_last.push_back(temp[t]);
		}
		temp.clear();
		for (list< list<tube *> >::iterator i = database.begin(); i != database.end();) { //遍历级联链表每个子链表首项
			if (i->empty()) {
				i = database.erase(i);
				continue;
			}
			bool canAdd = true;
			tmp = *(i->begin());

			for (size_t t = 0; t != rects.size(); t++) { //检查是否与已经填充的有重叠
				if (isOverlap(rects[t], tmp->position)) {
					canAdd = false;
					break;
				}
			}
			for (size_t t = 0; t != rects_last.size(); t++) { //检查是否与已经填充的有重叠
				if (!isSameObj(rects_last[t], tmp->position, 0.5) && isOverlap(rects_last[t], tmp->position)) {
					canAdd = false;
					break;
				}
			}


			if (canAdd) { //如果可以加入则将tube写入输出帧
				cvSetImageROI(frame, tmp->position);
				cvCopy(tmp->target, frame);
				cvResetImageROI(frame);
				rects.push_back(tmp->position); //将填充图块的位置保存
				temp.push_back(tmp->position);
				delete tmp; //释放tmp指向的tube的图像内存
				i->pop_front();
				i++;
			}
			else
				i++;



		}
		cvWriteFrame(writer, frame); //写帧
		rects.clear();//清空rects
		rects_last.clear();
		printf("frame:%d written.\n", frame_id);
		frame_id += 1;

	}

	printf("Database has been merged and written to file.\n");

	//释放内存
	cvReleaseImage(&frame);
	cvReleaseVideoWriter(&writer);
}
/**
*释放Database内存函数 - 辅助
*/
void freeDB(list< list<tube *> > & database) {
	for (list< list<tube *> >::iterator iter1 = database.begin(); iter1 != database.end(); iter1++)
	for (list<tube *>::iterator iter2 = iter1->begin(); iter2 != iter1->end();) {
		delete *iter2;
		iter2 = (*iter1).erase(iter2);
	}
}