#include "synopsis.h"
//#include "video_synopsis.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	const char file_path[] = "C:/Users/chenteng/Desktop/VideoTest.avi"; //"E:/Qt/car.mp4";
	const char file_demo_video[] = "E:/Qt/car.mp4";
	cout << "input 0 is open camera now, input 1 is  local video"<<endl;
	int select;
	cin >> select;
	if (select == 0){
		VideoCapture capture(0);//如果是笔记本，0打开的是自带的摄像头，1 打开外接的相机
		double rate = 25.0;//视频的帧率
		Size videoSize(640, 480);
		VideoWriter writer(file_path, CV_FOURCC('M', 'J', 'P', 'G'), rate, videoSize);
		Mat frame;
		cout << "camera is openning， store video now..." << endl;
		cout << "you can press  esc to close camera ..." << endl;
		while (capture.isOpened()){
			capture >> frame;
			writer << frame;
			imshow("video", frame);
			//27是键盘摁下esc时，计算机接收到的ascii码值
			if (waitKey(20) == 27) {
				capture.release();
				destroyAllWindows();
				break;
			}
		}
		cout << "camera is close，  video has store" << endl;
	}
	else if (select == 1){

	}
	
	const char file_bgimage_path[] = "C:/Users/chenteng/Desktop/bgColorImg.jpg"; //"C:/Users/chenteng/Desktop/bgColorImg.jpg";
	int fps, frame_number;
	CvSize size;
	IplImage* bgImage = NULL;
	const int frame_num_used = 500;
	list< list<tube *> > database;
	int code = (int)CV_FOURCC('d', 'i', 'v', '3');//MPEG-4.3 codec
	cout << "begin  generate background" << endl;
	if (select == 0){
		bgModeling(file_path, file_bgimage_path, frame_num_used, &bgImage, fps, frame_number, size, code);

	}else if (select == 1){
		bgModeling(file_demo_video, file_bgimage_path, frame_num_used, &bgImage, fps, frame_number, size, code);
	}
	cvReleaseImage(&bgImage);
	cout << "background  has store! the path is：  " << file_bgimage_path << endl;
	getchar();
	
	return 0;
}