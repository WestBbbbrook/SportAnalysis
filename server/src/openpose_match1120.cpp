// ------------------------- OpenPose Library Tutorial - Pose - Example 1 - Extract from Image -------------------------
// This first example shows the user how to:
// 1. Load an image (`filestream` module)
// 2. Extract the pose of that image (`pose` module)
// 3. Render the pose on a resized copy of the input image (`pose` module)
// 4. Display the rendered pose (`gui` module)
// In addition to the previous OpenPose modules, we also need to use:
// 1. `core` module: for the Array<float> class that the `pose` module needs
// 2. `utilities` module: for the error & logging functions, i.e. op::error & op::log respectively

// 3rdparty dependencies
#include <gflags/gflags.h> // DEFINE_bool, DEFINE_int32, DEFINE_int64, DEFINE_uint64, DEFINE_double, DEFINE_string
#include <glog/logging.h> // google::InitGoogleLogging
// OpenPose dependencies
#include <openpose/core/headers.hpp>
#include <openpose/filestream/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/pose/headers.hpp>
#include <openpose/utilities/headers.hpp>

#include <string>
#include <dirent.h>
#include <unistd.h>
#include <vector>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

#include <time.h>

#include <sys/time.h>

using namespace std;
using namespace cv;

// See all the available parameter options withe the `--help` flag. E.g. `./build/examples/openpose/openpose.bin --help`.
// Note: This command will show you flags for other unnecessary 3rdparty files. Check only the flags for the OpenPose
// executable. E.g. for `openpose.bin`, look for `Flags from examples/openpose/openpose.cpp:`.
// Debugging
DEFINE_int32(logging_level,             3,              "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
			 " 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
			 " low priority messages and 4 for important ones.");
// Producer
DEFINE_string(image_path,               "examples/media/COCO_val2014_000000000192.jpg",     "Process the desired image.");
DEFINE_string(image_path2,               "examples/media/Pic20170912.bmp",     "Process the desired image.");
DEFINE_string(video_file1,               "examples/media/test-1.mp4",     "Process the desired image.");
DEFINE_string(video_file,               "examples/videos/",     "Process the desired image.");
DEFINE_string(out_file,               "examples/activity_101117/activity/txt/",     "Process the desired image.");
// OpenPose
DEFINE_string(model_pose,               "COCO",         "Model to be used. E.g. `COCO` (18 keypoints), `MPI` (15 keypoints, ~10% faster), "
			  "`MPI_4_layers` (15 keypoints, even faster but less accurate).");
DEFINE_string(model_folder,             "models/",      "Folder path (absolute or relative) where the models (pose, face, ...) are located.");
DEFINE_string(net_resolution,           "656x368",      "Multiples of 16. If it is increased, the accuracy potentially increases. If it is decreased,"
			  " the speed increases. For maximum speed-accuracy balance, it should keep the closest aspect"
			  " ratio possible to the images or videos to be processed. E.g. the default `656x368` is"
			  " optimal for 16:9 videos, e.g. full HD (1980x1080) and HD (1280x720) videos.");
DEFINE_string(resolution,               "656x368",     "The image resolution (display and output). Use \"-1x-1\" to force the program to use the"
			  " default images resolution.");
DEFINE_int32(num_gpu_start,             0,              "GPU device start number.");
DEFINE_double(scale_gap,                0.3,            "Scale gap between scales. No effect unless scale_number > 1. Initial scale is always 1."
			  " If you want to change the initial scale, you actually want to multiply the"
			  " `net_resolution` by your desired initial scale.");
DEFINE_int32(scale_number,              1,              "Number of scales to average.");
// OpenPose Rendering
DEFINE_bool(disable_blending,           false,          "If blending is enabled, it will merge the results with the original frame. If disabled, it"
			" will only display the results on a black background.");
DEFINE_double(render_threshold,         0.05,           "Only estimated keypoints whose score confidences are higher than this threshold will be"
			  " rendered. Generally, a high threshold (> 0.5) will only render very clear body parts;"
			  " while small thresholds (~0.1) will also output guessed and occluded keypoints, but also"
			  " more false positives (i.e. wrong detections).");
DEFINE_double(alpha_pose,               0.6,            "Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will"
			  " hide it. Only valid for GPU rendering.");

#define PACKAGE_NUM 32

#define IMG_WIDTH 656//560 //1280
#define IMG_HEIGHT 368//315 //720

#define BLOCKSIZE IMG_WIDTH*IMG_HEIGHT*3/PACKAGE_NUM

struct recvBuf
{
	char buf[BLOCKSIZE];
	int flag;
};


class SocketMatTransmissionServer
{
public:
	SocketMatTransmissionServer(void);
	~SocketMatTransmissionServer(void);
	int sockConn;
private:
	struct recvBuf data;

	int needRecv;
	int count;

public:

	// 打开socket连接
	// params :	PORT	传输端口
	// return : -1		连接失败
	//			1		连接成功
	int socketConnect(int PORT);


	// 传输图像
	// params : image	待接收图像
	//		image	待接收图像
	// return : -1		接收失败
	//			1		接收成功
	int receive(cv::Mat &image);

	int receiveDouble(double &num);

	int sendDouble(double num);

	int receiveString(string &str);

	int sendString(string str);



	// 断开socket连接
	void socketDisconnect(void);
};

SocketMatTransmissionServer::SocketMatTransmissionServer(void)
{
}


SocketMatTransmissionServer::~SocketMatTransmissionServer(void)
{
}


int SocketMatTransmissionServer::socketConnect(int PORT)
{
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int on=1;
	//setsockopt( server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
	{
		perror("bind");
		return -1;
	}

	if(listen(server_sockfd, 5) == -1)
	{
		perror("listen");
		return -1;
	}

	struct sockaddr_in client_addr;
	socklen_t length = sizeof(client_addr);

	sockConn = accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
	if(sockConn < 0)
	{
		perror("connect");
		return -1;
	}
	else
	{
		printf("connect successful!\n");
		return 1;
	}

	close(server_sockfd);
}


void SocketMatTransmissionServer::socketDisconnect(void)
{
	close(sockConn);
}

int SocketMatTransmissionServer::receive(cv::Mat &image)
{
	int returnflag = 0;
	cv::Mat img(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, cv::Scalar(0));
	needRecv = sizeof(recvBuf);
	count = 0;
	memset(&data, 0, sizeof(data));

	for (int i = 0; i < PACKAGE_NUM; i++)
	{
		int pos = 0;
		int len0 = 0;

		while (pos < needRecv)
		{
			len0 = recv(sockConn, (char *)(&data) + pos, needRecv - pos, 0);
			if (len0 < 0)
			{
				printf("Server Recieve Data Failed!\n");
				break;
			}
			pos += len0;
		}

		count = count + data.flag;

		int num1 = IMG_HEIGHT / PACKAGE_NUM * i;
		for (int j = 0; j < IMG_HEIGHT / PACKAGE_NUM; j++)
		{
			int num2 = j * IMG_WIDTH * 3;
			uchar *ucdata = img.ptr<uchar>(j + num1);
			for (int k = 0; k < IMG_WIDTH * 3; k++)
			{
				ucdata[k] = data.buf[num2 + k];
			}
		}

		if (data.flag == 2)
		{
			if (count == PACKAGE_NUM + 1)
			{
				image = img;
				returnflag = 1;
				count = 0;
			}
			else
			{
				count = 0;
				i = 0;
			}
		}
	}
	if(returnflag == 1)
		return 1;
	else
		return -1;
}

int SocketMatTransmissionServer::receiveDouble(double &num)
{

	char recvBuf[100] = "";

	if(recv(sockConn, recvBuf, 100, 0) < 0)
	{
		printf("Server Recieve Data Failed!\n");
		return -1;
	}
	//printf("%s\n",recvBuf);
	num = atof(recvBuf);
	//std::cout<<num<<std::endl;
	return 1;
}

int SocketMatTransmissionServer::sendDouble(double num)
{
	if (num < 0.0)
	{
		printf("double num < 0\n\n");
		return -1;
	}

	char doublechars[100] = "";

	sprintf(doublechars, "%f", num);

	if (send(sockConn, doublechars, strlen(doublechars) + 1, 0) < 0)
	{
		printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
	return 0;
}

int SocketMatTransmissionServer::receiveString(string &str)
{

	char recvBuf[1000] = "";

	if(recv(sockConn, recvBuf, 1000, 0) < 0)
	{
		printf("Server Recieve Data Failed!\n");
		return -1;
	}
	//printf("%s\n",recvBuf);
	str = recvBuf;
	//std::cout<<num<<std::endl;
	return 1;
}

int SocketMatTransmissionServer::sendString(string str)
{
	const char *stringchars = str.data();
	if (send(sockConn, stringchars, strlen(stringchars) + 1, 0) < 0)
	{
		printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
	return 0;
}


//向量p1逆时针旋转到p2的角度
double getRotateAngle(double x1, double y1, double x2, double y2)
{
	const double epsilon = 1.0e-6;
	const double nyPI = acos(-1.0);
	double dist, dot, degree, angle;

	// normalize
	dist = sqrt( x1 * x1 + y1 * y1 );
	x1 /= dist;
	y1 /= dist;
	dist = sqrt( x2 * x2 + y2 * y2 );
	x2 /= dist;
	y2 /= dist;
	// dot product
	dot = x1 * x2 + y1 * y2;
	if ( fabs(dot - 1.0) <= epsilon )
		angle = 0.0;
	else if ( fabs(dot + 1.0) <= epsilon )
		angle = nyPI;
	else
	{
		double cross;

		angle = acos(dot);
		//cross product
		cross = x1 * y2 - x2 * y1;
		// vector p2 is clockwise from vector p1
		// with respect to the origin (0.0)
		if (cross < 0 )
		{
			angle = 2 * nyPI - angle;
		}
	}
	degree = angle *  180.0 / nyPI;
	return degree;
}

//求中心点
Point getCenterPoint(vector<Point> points)
{
	int size = points.size();
	if(size == 0)return Point();
	int x = 0;
	int y = 0;
	for(int i = 0; i < size; ++i)
	{
		x += points[i].x;
		y += points[i].y;
	}
	return Point(x / size, y / size);
}


//已知三个点，求fromp1-center-top2逆时针角度
double getAngle(Point fromp1, Point center, Point top2)
{
	double x1 = fromp1.x - center.x;
	double y1 = fromp1.y - center.y;
	double x2 = top2.x - center.x;
	double y2 = top2.y - center.y;
	double res = getRotateAngle(x1, y1, x2, y2);
	if(std::isnan(res))return 0.0;
	return res;
}

//将角度存到容器 轮廓法
vector<double> getAngles(vector<Point> points, Point head)
{
	vector<double> res;
	Point center = getCenterPoint(points);
	for(int i = 0; i < points.size(); ++i)
	{
		if(points[i] != center && points[i] != head)
		{
			double angle = getAngle(points[i], center, head);
			res.push_back(angle);
		}
	}
	sort(res.begin(), res.end());
	return res;
}

//将角度存到容器 openpose
vector<double> getAngles(vector<Point> points)
{
	vector<double> res;
	/*
	0-1-2
	2-1-8
	8-1-11
	11-1-5
	5-1-0
	3-2-1
	4-3-2
	1-5-6
	5-6-7
	9-8-1
	10-9-8
	1-11-12
	11-12-13
	*/
	res.push_back(getAngle(points[0], points[1], points[2]));
	res.push_back(getAngle(points[2], points[1], points[8]));
	res.push_back(getAngle(points[8], points[1], points[11]));
	res.push_back(getAngle(points[11], points[1], points[5]));
	res.push_back(getAngle(points[5], points[1], points[0]));
	res.push_back(getAngle(points[3], points[2], points[1]));
	res.push_back(getAngle(points[4], points[3], points[2]));
	res.push_back(getAngle(points[1], points[5], points[6]));
	res.push_back(getAngle(points[5], points[6], points[7]));
	res.push_back(getAngle(points[9], points[8], points[1]));
	res.push_back(getAngle(points[10], points[9], points[8]));
	res.push_back(getAngle(points[1], points[11], points[12]));
	res.push_back(getAngle(points[11], points[12], points[13]));
	return res;
}

//求加权夹角 每刻相似度 1 - sigma(w * delta(alpha)) / (size(alpha)) * 180)
double getSimilarity(vector<double> v1, vector<double> v2, vector<double> weight)
{
	cout<<"v1 v2 w "<<v1.size()<<" "<<v2.size()<<" "<<weight.size()<<endl;
	if(v1.size() != v2.size() || v1.size() != weight.size())
		return 0.0;

	double res = 0.0;
	for(int i = 0; i < v1.size(); ++i)
	{
		res += weight[i] * abs(v1[i] - v2[i]);
		cout<<"res "<<res<<" "<<v1[i]<<" "<<v2[i]<<endl;
	}
	cout <<"ssss"<<1 - res/(v1.size()*360)<<endl;
	res = 1 - res / (v1.size() * 180);
	if(std::isnan(res))return 0;
	return res;
}

//求加权夹角 综合相似度 sigma(w * score) / (size(frame))
double getSimilarity(vector<double> score, vector<double> weight)
{
	cout<<"s w "<<score.size()<<" "<<weight.size()<<endl;
	if(score.size() != weight.size())
		return 0.0;

	double res = 0.0;
	for(int i = 0; i < score.size(); ++i)
	{
		res += weight[i] * score[i];
	}
	res = res / score.size();
	if(std::isnan(res))return 0;
	return res;
}




vector<string> listFiles(const char *basePath)
{
	vector<string> files;
	DIR *dir;
	struct dirent *ptr;
	//char base[1000];

	if ((dir = opendir(basePath)) == NULL)
	{
		std::cout << "Open dir error..." << std::endl;
		return files;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8)    ///file
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			//files.push_back(string(basePath) + "/" + ptr->d_name);
			files.push_back(ptr->d_name);
		//else if(ptr->d_type == 10)    ///link file
		//printf("d_name:%s/%s\n",basePath,ptr->d_name);
		//else if(ptr->d_type == 4)    ///dir
		//{
		//    memset(base,'\0',sizeof(base));
		//    strcpy(base,basePath);
		//    strcat(base,"/");
		//    strcat(base,ptr->d_name);
		//    readFileList(base);
		//}
	}
	closedir(dir);
	return files;
}

void SplitString(const string &s, vector<string> &v, const string &c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while(string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if(pos1 != s.length())
		v.push_back(s.substr(pos1));
}

vector<vector<Point> > readDBPoints(string txtfilename)
{
	ifstream in(txtfilename);
	string txtline;
	vector<vector<Point> > pointsframes;

	int step = 0;

	while (getline(in, txtline))//逐行读取数据并存于s中，直至数据全部读取
	{
		if((step++) % 6 != 0){
			continue;
		}
		//std::cout << txtline.c_str() << std::endl;
		vector<string> splitwords;
		SplitString(txtline, splitwords, ":");
		//std::cout << splitwords.size() << std::endl;
		vector<Point> points;
		int vindex = 1;
		for(int i = 0; i < 18; ++i)
		{
			double x = stod(splitwords[vindex++]);
			double y = stod(splitwords[vindex++]);
			points.push_back(Point(x, y));
			std::cout << x << " " << y << endl;
		}
		pointsframes.push_back(points);
		//std::cout << stod(splitwords[1])<<" "<<stod(splitwords[2]);
		std::cout << std::endl;
	}

	in.close();
	return pointsframes;
}

double bbOverlap(Rect box1, Rect box2)
{
	if (box1.x > box2.x + box2.width)
	{
		return 0.0;
	}
	if (box1.y > box2.y + box2.height)
	{
		return 0.0;
	}
	if (box1.x + box1.width < box2.x)
	{
		return 0.0;
	}
	if (box1.y + box1.height < box2.y)
	{
		return 0.0;
	}
	double colInt =  min(box1.x + box1.width, box2.x + box2.width) - max(box1.x, box2.x);
	double rowInt =  min(box1.y + box1.height, box2.y + box2.height) - max(box1.y, box2.y);
	double intersection = colInt * rowInt;
	double area1 = box1.width * box1.height;
	double area2 = box2.width * box2.height;
	return intersection / (area1 + area2 - intersection);
}

class CompressiveTracker
{
public:
	CompressiveTracker(void);
	~CompressiveTracker(void);

private:
	int featureMinNumRect;
	int featureMaxNumRect;
	int featureNum;
	vector<vector<Rect>> features;
	vector<vector<float>> featuresWeight;
	int rOuterPositive;
	vector<Rect> samplePositiveBox;
	vector<Rect> sampleNegativeBox;
	int rSearchWindow;
	Mat imageIntegral;
	Mat samplePositiveFeatureValue;
	Mat sampleNegativeFeatureValue;
	vector<float> muPositive;
	vector<float> sigmaPositive;
	vector<float> muNegative;
	vector<float> sigmaNegative;
	float learnRate;
	vector<Rect> detectBox;
	Mat detectFeatureValue;
	RNG rng;

private:
	void HaarFeature(Rect &_objectBox, int _numFeature);
	void sampleRect(Mat &_image, Rect &_objectBox, float _rInner, float _rOuter, int _maxSampleNum, vector<Rect> &_sampleBox);
	void sampleRect(Mat &_image, Rect &_objectBox, float _srw, vector<Rect> &_sampleBox);
	void getFeatureValue(Mat &_imageIntegral, vector<Rect> &_sampleBox, Mat &_sampleFeatureValue);
	void classifierUpdate(Mat &_sampleFeatureValue, vector<float> &_mu, vector<float> &_sigma, float _learnRate);
	void radioClassifier(vector<float> &_muPos, vector<float> &_sigmaPos, vector<float> &_muNeg, vector<float> &_sigmaNeg,
		Mat &_sampleFeatureValue, float &_radioMax, int &_radioMaxIndex);
public:
	void processFrame(Mat &_frame, Rect &_objectBox);
	void init(Mat &_frame, Rect &_objectBox);
};

//------------------------------------------------
CompressiveTracker::CompressiveTracker(void)
{
	featureMinNumRect = 2;
	featureMaxNumRect = 4;  // number of rectangle from 2 to 4
	featureNum = 50;    // number of all weaker classifiers, i.e,feature pool
	rOuterPositive = 4; // radical scope of positive samples
	rSearchWindow = 50;//25; // size of search window
	muPositive = vector<float>(featureNum, 0.0f);
	muNegative = vector<float>(featureNum, 0.0f);
	sigmaPositive = vector<float>(featureNum, 1.0f);
	sigmaNegative = vector<float>(featureNum, 1.0f);
	learnRate = 0.85f;  // Learning rate parameter
}

CompressiveTracker::~CompressiveTracker(void)
{
}


void CompressiveTracker::HaarFeature(Rect &_objectBox, int _numFeature)
	/*Description: compute Haar features
	Arguments:
	-_objectBox: [x y width height] object rectangle
	-_numFeature: total number of features.The default is 50.
	*/
{
	features = vector<vector<Rect>>(_numFeature, vector<Rect>());
	featuresWeight = vector<vector<float>>(_numFeature, vector<float>());

	int numRect;
	Rect rectTemp;
	float weightTemp;

	for (int i = 0; i < _numFeature; i++)
	{
		numRect = cvFloor(rng.uniform((double)featureMinNumRect, (double)featureMaxNumRect));

		for (int j = 0; j < numRect; j++)
		{

			rectTemp.x = cvFloor(rng.uniform(0.0, (double)(_objectBox.width - 3)));
			rectTemp.y = cvFloor(rng.uniform(0.0, (double)(_objectBox.height - 3)));
			rectTemp.width = cvCeil(rng.uniform(0.0, (double)(_objectBox.width - rectTemp.x - 2)));
			rectTemp.height = cvCeil(rng.uniform(0.0, (double)(_objectBox.height - rectTemp.y - 2)));
			features[i].push_back(rectTemp);

			weightTemp = (float)pow(-1.0, cvFloor(rng.uniform(0.0, 2.0))) / sqrt(float(numRect));
			featuresWeight[i].push_back(weightTemp);

		}
	}
}


void CompressiveTracker::sampleRect(Mat &_image, Rect &_objectBox, float _rInner, float _rOuter, int _maxSampleNum, vector<Rect> &_sampleBox)
	/* Description: compute the coordinate of positive and negative sample image templates
	Arguments:
	-_image:        processing frame
	-_objectBox:    recent object position
	-_rInner:       inner sampling radius
	-_rOuter:       Outer sampling radius
	-_maxSampleNum: maximal number of sampled images
	-_sampleBox:    Storing the rectangle coordinates of the sampled images.
	*/
{
	int rowsz = _image.rows - _objectBox.height - 1;
	int colsz = _image.cols - _objectBox.width - 1;
	float inradsq = _rInner * _rInner;
	float outradsq = _rOuter * _rOuter;


	int dist;

	int minrow = max(0, (int)_objectBox.y - (int)_rInner);
	int maxrow = min((int)rowsz - 1, (int)_objectBox.y + (int)_rInner);
	int mincol = max(0, (int)_objectBox.x - (int)_rInner);
	int maxcol = min((int)colsz - 1, (int)_objectBox.x + (int)_rInner);



	int i = 0;

	float prob = ((float)(_maxSampleNum)) / (maxrow - minrow + 1) / (maxcol - mincol + 1);

	int r;
	int c;

	_sampleBox.clear();//important
	Rect rec(0, 0, 0, 0);

	for( r = minrow; r <= (int)maxrow; r++ )
		for( c = mincol; c <= (int)maxcol; c++ )
		{
			dist = (_objectBox.y - r) * (_objectBox.y - r) + (_objectBox.x - c) * (_objectBox.x - c);

			if( rng.uniform(0., 1.) < prob && dist < inradsq && dist >= outradsq )
			{

				rec.x = c;
				rec.y = r;
				rec.width = _objectBox.width;
				rec.height = _objectBox.height;

				_sampleBox.push_back(rec);

				i++;
			}
		}

		_sampleBox.resize(i);

}

void CompressiveTracker::sampleRect(Mat &_image, Rect &_objectBox, float _srw, vector<Rect> &_sampleBox)
	/* Description: Compute the coordinate of samples when detecting the object.*/
{
	int rowsz = _image.rows - _objectBox.height - 1;
	int colsz = _image.cols - _objectBox.width - 1;
	float inradsq = _srw * _srw;


	int dist;

	int minrow = max(0, (int)_objectBox.y - (int)_srw);
	int maxrow = min((int)rowsz - 1, (int)_objectBox.y + (int)_srw);
	int mincol = max(0, (int)_objectBox.x - (int)_srw);
	int maxcol = min((int)colsz - 1, (int)_objectBox.x + (int)_srw);

	int i = 0;

	int r;
	int c;

	Rect rec(0, 0, 0, 0);
	_sampleBox.clear();//important

	for( r = minrow; r <= (int)maxrow; r++ )
		for( c = mincol; c <= (int)maxcol; c++ )
		{
			dist = (_objectBox.y - r) * (_objectBox.y - r) + (_objectBox.x - c) * (_objectBox.x - c);

			if( dist < inradsq )
			{

				rec.x = c;
				rec.y = r;
				rec.width = _objectBox.width;
				rec.height = _objectBox.height;

				_sampleBox.push_back(rec);

				i++;
			}
		}

		_sampleBox.resize(i);

}
// Compute the features of samples
void CompressiveTracker::getFeatureValue(Mat &_imageIntegral, vector<Rect> &_sampleBox, Mat &_sampleFeatureValue)
{
	int sampleBoxSize = _sampleBox.size();
	_sampleFeatureValue.create(featureNum, sampleBoxSize, CV_32F);
	float tempValue;
	int xMin;
	int xMax;
	int yMin;
	int yMax;

	for (int i = 0; i < featureNum; i++)
	{
		for (int j = 0; j < sampleBoxSize; j++)
		{
			tempValue = 0.0f;
			for (size_t k = 0; k < features[i].size(); k++)
			{
				xMin = _sampleBox[j].x + features[i][k].x;
				xMax = _sampleBox[j].x + features[i][k].x + features[i][k].width;
				yMin = _sampleBox[j].y + features[i][k].y;
				yMax = _sampleBox[j].y + features[i][k].y + features[i][k].height;
				tempValue += featuresWeight[i][k] *
					(_imageIntegral.at<float>(yMin, xMin) +
					_imageIntegral.at<float>(yMax, xMax) -
					_imageIntegral.at<float>(yMin, xMax) -
					_imageIntegral.at<float>(yMax, xMin));
			}
			_sampleFeatureValue.at<float>(i, j) = tempValue;
		}
	}
}

// Update the mean and variance of the gaussian classifier
void CompressiveTracker::classifierUpdate(Mat &_sampleFeatureValue, vector<float> &_mu, vector<float> &_sigma, float _learnRate)
{
	Scalar muTemp;
	Scalar sigmaTemp;

	for (int i = 0; i < featureNum; i++)
	{
		meanStdDev(_sampleFeatureValue.row(i), muTemp, sigmaTemp);

		_sigma[i] = (float)sqrt( _learnRate * _sigma[i] * _sigma[i] + (1.0f - _learnRate) * sigmaTemp.val[0] * sigmaTemp.val[0]
		+ _learnRate * (1.0f - _learnRate) * (_mu[i] - muTemp.val[0]) * (_mu[i] - muTemp.val[0])); // equation 6 in paper

		_mu[i] = _mu[i] * _learnRate + (1.0f - _learnRate) * muTemp.val[0]; // equation 6 in paper
	}
}

// Compute the ratio classifier
void CompressiveTracker::radioClassifier(vector<float> &_muPos, vector<float> &_sigmaPos, vector<float> &_muNeg, vector<float> &_sigmaNeg,
										 Mat &_sampleFeatureValue, float &_radioMax, int &_radioMaxIndex)
{
	float sumRadio;
	_radioMax = -FLT_MAX;
	_radioMaxIndex = 0;
	float pPos;
	float pNeg;
	int sampleBoxNum = _sampleFeatureValue.cols;

	for (int j = 0; j < sampleBoxNum; j++)
	{
		sumRadio = 0.0f;
		for (int i = 0; i < featureNum; i++)
		{
			pPos = exp( (_sampleFeatureValue.at<float>(i, j) - _muPos[i]) * (_sampleFeatureValue.at<float>(i, j) - _muPos[i]) / -(2.0f * _sigmaPos[i] * _sigmaPos[i] + 1e-30) ) / (_sigmaPos[i] + 1e-30);
			pNeg = exp( (_sampleFeatureValue.at<float>(i, j) - _muNeg[i]) * (_sampleFeatureValue.at<float>(i, j) - _muNeg[i]) / -(2.0f * _sigmaNeg[i] * _sigmaNeg[i] + 1e-30) ) / (_sigmaNeg[i] + 1e-30);
			sumRadio += log(pPos + 1e-30) - log(pNeg + 1e-30); // equation 4
		}
		if (_radioMax < sumRadio)
		{
			_radioMax = sumRadio;
			_radioMaxIndex = j;
		}
	}
}
void CompressiveTracker::init(Mat &_frame, Rect &_objectBox)
{
	// compute feature template
	HaarFeature(_objectBox, featureNum);

	// compute sample templates
	sampleRect(_frame, _objectBox, rOuterPositive, 0, 1000000, samplePositiveBox);
	sampleRect(_frame, _objectBox, rSearchWindow * 1.5, rOuterPositive + 4.0, 100, sampleNegativeBox);

	integral(_frame, imageIntegral, CV_32F);

	getFeatureValue(imageIntegral, samplePositiveBox, samplePositiveFeatureValue);
	getFeatureValue(imageIntegral, sampleNegativeBox, sampleNegativeFeatureValue);
	classifierUpdate(samplePositiveFeatureValue, muPositive, sigmaPositive, learnRate);
	classifierUpdate(sampleNegativeFeatureValue, muNegative, sigmaNegative, learnRate);
}
void CompressiveTracker::processFrame(Mat &_frame, Rect &_objectBox)
{
	// predict
	sampleRect(_frame, _objectBox, rSearchWindow, detectBox);
	integral(_frame, imageIntegral, CV_32F);
	getFeatureValue(imageIntegral, detectBox, detectFeatureValue);
	int radioMaxIndex;
	float radioMax;
	radioClassifier(muPositive, sigmaPositive, muNegative, sigmaNegative, detectFeatureValue, radioMax, radioMaxIndex);
	_objectBox = detectBox[radioMaxIndex];

	// update
	sampleRect(_frame, _objectBox, rOuterPositive, 0.0, 1000000, samplePositiveBox);
	sampleRect(_frame, _objectBox, rSearchWindow * 1.5, rOuterPositive + 4.0, 100, sampleNegativeBox);

	getFeatureValue(imageIntegral, samplePositiveBox, samplePositiveFeatureValue);
	getFeatureValue(imageIntegral, sampleNegativeBox, sampleNegativeFeatureValue);
	classifierUpdate(samplePositiveFeatureValue, muPositive, sigmaPositive, learnRate);
	classifierUpdate(sampleNegativeFeatureValue, muNegative, sigmaNegative, learnRate);
}

char getch()
{
	char c;
	system("stty -echo");
	system("stty -icanon");
	c = getchar();
	system("stty icanon");
	system("stty echo");
	return c;
}

void recvMat(int &clickX, int &clickY, bool &action_start)
{
	op::log("OpenPose Library .", op::Priority::High);
	// ------------------------- INITIALIZATION -------------------------
	// Step 1 - Set logging level
	// - 0 will output all the logging messages
	// - 255 will output nothing
	op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.", __LINE__, __FUNCTION__, __FILE__);
	op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
	op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
	// Step 2 - Read Google flags (user defined configuration)
	// outputSize
	const auto outputSize = op::flagsToPoint(FLAGS_resolution, "640x480");//"1280x720");
	// netInputSize
	const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "656x368");//"640x480");
	// netOutputSize
	const auto netOutputSize = netInputSize;
	// poseModel
	const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
	// Check no contradictory flags enabled
	if (FLAGS_alpha_pose < 0. || FLAGS_alpha_pose > 1.)
		op::error("Alpha value for blending must be in the range [0,1].", __LINE__, __FUNCTION__, __FILE__);
	if (FLAGS_scale_gap <= 0. && FLAGS_scale_number > 1)
		op::error("Incompatible flag configuration: scale_gap must be greater than 0 or scale_number = 1.", __LINE__, __FUNCTION__, __FILE__);
	// Logging
	op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
	// Step 3 - Initialize all required classes
	op::CvMatToOpInput cvMatToOpInput{netInputSize, FLAGS_scale_number, (float)FLAGS_scale_gap};
	//op::CvMatToOpOutput cvMatToOpOutput{outputSize};
	op::PoseExtractorCaffe poseExtractorCaffe{netInputSize, netOutputSize, outputSize, FLAGS_scale_number, poseModel,
		FLAGS_model_folder, FLAGS_num_gpu_start};
	//op::PoseRenderer poseRenderer{netOutputSize, outputSize, poseModel, nullptr, (float)FLAGS_render_threshold,
	//                              !FLAGS_disbale_blending, (float)FLAGS_alpha_pose};
	//op::OpOutputToCvMat opOutputToCvMat{outputSize};
	//const op::Point<int> windowedSize = outputSize;
	//op::FrameDisplayer frameDisplayer{windowedSize, "OpenPose Tutorial - Example 1"};

	// Step 4 - Initialize resources on desired thread (in this case single thread, i.e. we init resources here)
	poseExtractorCaffe.initializationOnThread();
	//poseRenderer.initializationOnThread();

	SocketMatTransmissionServer socketMat;
	if (socketMat.socketConnect(6666) < 0)
	{
		return;
	}



	vector<vector<Point> > pointsVideo1;
	vector<double> finalscores;

	double max_score = 0.0;
	int max_index = 0;

	//bool action_start = true;
	bool clicked = false;

	cv::Mat inputImage;
	// CT framework
	CompressiveTracker ct;


	Mat grayImg;
	Rect trackbox;

	vector<Point> pointsPerFrame1;
	string point_string = "points";
        string point1_string = "points1";
	string score_string = "scores";
	string bboxs_string = "bboxs";

	//read db txt file
	vector<string> files = listFiles(FLAGS_out_file.data());
	if(files.empty())
	{
		std::cout << "empty files" << std::endl;
		return;
	}
	sort(files.begin(), files.end());
	//cout<<files[0]<<"+++++++++++++++++"<<endl;

	vector<vector<vector<Point> > > pointsVideos2;
	for(int i = 0; i < files.size(); ++i)
	{
		pointsVideos2.push_back(readDBPoints(string(FLAGS_out_file.data()) + "/" + files[i]));					
	}


	while (1)
	{
		clock_t m_begin = clock();


		//per frame
		pointsPerFrame1.clear();
		point_string = "points";
                point1_string = "points1";
		score_string = "scores";
		bboxs_string = "bboxs";

		if(socketMat.receive(inputImage) > 0)
		{
			//cv::imshow("test",image);
			//cv::waitKey(30);
			if ( inputImage.empty() )continue; //break;
			//resize(inputImage,inputImage,Size(656,368));
			clock_t m_begin_op = clock();
			// Step 2 - Format input image to OpenPose input and output formats
			op::Array<float> netInputArray;
			std::vector<float> scaleRatios;
			std::tie(netInputArray, scaleRatios) = cvMatToOpInput.format(inputImage);
			//double scaleInputToOutput;
			//op::Array<float> outputArray;
			//std::tie(scaleInputToOutput, outputArray) = cvMatToOpOutput.format(inputImage);

			// Step 3 - Estimate poseKeypoints
			poseExtractorCaffe.forwardPass(netInputArray, {inputImage.cols, inputImage.rows}, scaleRatios);
			const auto poseKeypoints = poseExtractorCaffe.getPoseKeypoints();

			/////////////////////////
			// Common parameters needed
			const auto numberPeopleDetected = poseKeypoints.getSize(0);
			const auto numberBodyParts = poseKeypoints.getSize(1);

			std::cout<<"openpose detect done"<<std::endl;
			clock_t m_end_op = clock();
			cout <<"============openpose========================="<<  CLOCKS_PER_SEC/ (double)(m_end_op - m_begin_op) << endl;
			std::cout<<numberPeopleDetected<<std::endl;
			std::cout<<numberBodyParts<<std::endl;
			//continue;


			cout<<"action_start:"<<action_start<<endl;
			cout<<"pointsVideo1.size():"<<pointsVideo1.size()<<endl;

			if(action_start == false && !pointsVideo1.empty())
			{
				for(int i = 0; i < files.size(); ++i)
				{
					//pointsVideos2.push_back(readDBPoints(string(FLAGS_out_file.data()) + "/" + files[i]));

					//vector<vector<Point> > pointsVideo2 = readDBPoints(string(FLAGS_out_file.data()) + "/" + files[i]);
					vector<vector<Point> > pointsVideo2 = pointsVideos2[i];

					//compute
					vector<double> weightpf;
					vector<double> weightfinal;
					vector<double> score;
					//per frame
					cout<<pointsVideo1.size() <<"===== "<< pointsVideo2.size()<<endl;
					for(int index = 0; index < pointsVideo1.size() && index < pointsVideo2.size(); ++index)
					{
						vector<double> angles1 = getAngles(pointsVideo1[index]);
						vector<double> angles2 = getAngles(pointsVideo2[index]);
						//for(int i = 0; i < angles1.size(); ++i)
						//{
						//	weightpf.push_back(1.0);
						//}
							/*
	0-1-2
	2-1-8
	8-1-11
	11-1-5
	5-1-0
	3-2-1
	4-3-2
	1-5-6
	5-6-7
	9-8-1
	10-9-8
	1-11-12
	11-12-13
	*/
						for(int i = 0; i < 5;++i)
							weightpf.push_back(0.5);
						for(int i = 5; i < 7;++i)
							weightpf.push_back(3.75);
						for(int i = 7; i < 13;++i)
							weightpf.push_back(0.5);


						double simPf = getSimilarity(angles1, angles2, weightpf);
						weightpf.clear();
						cout << "simPf:" << simPf << endl;
						score.push_back(simPf);
						weightfinal.push_back(1.0);
					}
					double simfinal = getSimilarity(score, weightfinal);
					finalscores.push_back(simfinal);
					cout << "simfinal:" << simfinal << endl;
					//scores.push_back(simfinal);
					if(simfinal > max_score)
					{
						max_score = simfinal;
						max_index = i;
					}
					score.clear();
				}

				///////////////////////////////////
				for(int i = 0; i < files.size(); ++i)
				{
					std::cout << files[i] << " " << finalscores[i] << std::endl;
				}

				cout << "max score:" << max_score << "(" << max_index << ")" << files[max_index] << endl;

				//send
				for(int i = 0; i < finalscores.size(); ++i)
				{
					score_string += (":" + std::to_string(finalscores[i])  );
				}
				//
				//score_string = ("scores:" + std::to_string(0.95)  );
				//res_string += (":" + std::to_string(max_score) + ":" + std::to_string(max_index));
				socketMat.sendString(score_string);
				cout<<score_string<<endl;
				//clear
				pointsVideo1.clear();
				finalscores.clear();
				clicked = false;
				//mousePoint.clear();
				max_score = 0.0;
				max_index = 0;
				//*aciton_end=false;
			}



			if(numberPeopleDetected == 0){
				socketMat.sendString("zero");
				clock_t m_end = clock();
				cout <<"====================================="<<  CLOCKS_PER_SEC/ (double)(m_end - m_begin) << endl;
				continue;
			}

			if(numberPeopleDetected == 1 )//&& action_start == true)
			{
				cout<<"numberPeopleDetected "<<numberPeopleDetected<<endl;
				//cout<<action_start<<endl;
				if( action_start == true){
					int person = 0;
					for(int part = 0; part < numberBodyParts; ++part)
					{
						const auto baseIndex = poseKeypoints.getSize(2) * (person * numberBodyParts + part);
						const int x = poseKeypoints[baseIndex];
						const int y = poseKeypoints[baseIndex + 1];


						pointsPerFrame1.push_back(Point(x, y));
						//std::cout << "test::::(" << x << "," << y << "):::::::::::::::::::::::" << std::endl;
						point_string += (":" + std::to_string((int)x) + ":" + std::to_string((int)y));
					}
					pointsVideo1.push_back(pointsPerFrame1);
					cout<<point_string<<endl;
					socketMat.sendString(point_string);

				}else{
					int person = 0;
					for(int part = 0; part < numberBodyParts; ++part)
					{
						const auto baseIndex = poseKeypoints.getSize(2) * (person * numberBodyParts + part);
						const int x = poseKeypoints[baseIndex];
						const int y = poseKeypoints[baseIndex + 1];


						pointsPerFrame1.push_back(Point(x, y));
						//std::cout << "test::::(" << x << "," << y << "):::::::::::::::::::::::" << std::endl;
						point1_string += (":" + std::to_string((int)x) + ":" + std::to_string((int)y));
						circle(inputImage, Point(x,y), 5, cv::Scalar(0, 0, 255),-1);
					}
					//pointsVideo1.push_back(pointsPerFrame1);
					cout<<point1_string<<endl;
					socketMat.sendString(point1_string);
					//imshow("test",inputImage);
					//waitKey(10);
				}
				clock_t m_end = clock();
				cout <<"====================================="<<  CLOCKS_PER_SEC/ (double)(m_end - m_begin) << endl;
				continue;//next frame
			}




			vector<vector<Point> > bboxs;


			for(int person = 0; person < numberPeopleDetected; ++person)
			{
				//each person:
				int startX = 9999;
				int startY = 9999;
				int endX = 0;
				int endY = 0;
				int part0y = 0;
				int part1y = 0;

				//std::cout << "::::::" << person << "::::::::" << std::endl;
				vector<Point> box;
				for(int part = 0; part < numberBodyParts; ++part)
				{
					const auto baseIndex = poseKeypoints.getSize(2) * (person * numberBodyParts + part);
					const int x = poseKeypoints[baseIndex];
					const int y = poseKeypoints[baseIndex + 1];
					//const auto score_xy = poseKeypoints[baseIndex + 2];

					if(x == 0 && y == 0){
						continue;
					}
					if(part == 0)part0y = y;
					if(part == 1)part1y = y;

					if(x < startX)startX = x;
					if(y < startY)startY = y;
					if(x > endX)endX = x;
					if(y > endY)endY = y;

					//if person is we need {}
					//pointsPerFrame1.push_back(Point(x, y));

				}
				startY = startY - abs(part1y-part0y);

				box.push_back(Point(startX, startY));
				box.push_back(Point(endX, endY));
				bboxs.push_back(box);
				bboxs_string += (":" + std::to_string((int)startX) + ":" + std::to_string((int)startY) + ":" + std::to_string((int)endX) + ":" + std::to_string((int)endY));



				//rectangle(inputImage,Rect(startX, startY, endX-startX, endY-startY),Scalar(0,0,255));



				//std::cout << "===============" << std::endl;
			}
			

			if(clicked == false)
			{
				//send all boxs
				cout<<bboxs_string<<endl;
				socketMat.sendString(bboxs_string);

				if((clickX) > 0 && (clickY) > 0) //not 0
				{
					for(int bi = 0; bi < bboxs.size(); ++bi)
					{
						Rect rectbox(bboxs[bi][0].x, bboxs[bi][0].y, bboxs[bi][1].x - bboxs[bi][0].x, bboxs[bi][1].y - bboxs[bi][0].y);
						if(rectbox.contains(Point(clickX, clickY)))
						{
							cvtColor(inputImage, grayImg, CV_RGB2GRAY);
							ct.init(grayImg, rectbox);
							trackbox = rectbox;
							clicked = true;
						}
					}
					clickX=0;
					clickY=0;
				}
			}
			//imshow("t",inputImage);
			//waitKey(1);

			if(clicked == true)// && action_start == true)
			{
				cout<<trackbox.x<<" "<<trackbox.y<<" "<<trackbox.width<<" "<<trackbox.height<<endl;

				cvtColor(inputImage, grayImg, CV_RGB2GRAY);
				ct.processFrame(grayImg, trackbox);// Process frame
				//overlap
				double overlap_max = 0.0;
				int bi_max=0;
				for(int bi = 0; bi < bboxs.size(); ++bi)
				{
					double overlap = bbOverlap(Rect(bboxs[bi][0].x, bboxs[bi][0].y, bboxs[bi][1].x - bboxs[bi][0].x, bboxs[bi][1].y - bboxs[bi][0].y), trackbox);
					if(overlap > overlap_max)
					{
						overlap_max = overlap;
						bi_max = bi;
					}
				}
				trackbox=Rect(bboxs[bi_max][0].x, bboxs[bi_max][0].y, bboxs[bi_max][1].x - bboxs[bi_max][0].x, bboxs[bi_max][1].y - bboxs[bi_max][0].y);

				if(action_start == true){
					int person = bi_max;
					for(int part = 0; part < numberBodyParts; ++part)
					{
						const auto baseIndex = poseKeypoints.getSize(2) * (person * numberBodyParts + part);
						const int x = poseKeypoints[baseIndex];
						const int y = poseKeypoints[baseIndex + 1];


						pointsPerFrame1.push_back(Point(x, y));
						//std::cout << "test::::(" << x << "," << y << "):::::::::::::::::::::::" << std::endl;
						point_string += (":" + std::to_string((int)x) + ":" + std::to_string((int)y));
					}
					pointsVideo1.push_back(pointsPerFrame1);
					socketMat.sendString(point_string);
					cout<<point_string<<endl;
				}
				else{
					bboxs_string = ("bboxs:" + std::to_string(bboxs[bi_max][0].x) + ":" + std::to_string(bboxs[bi_max][0].y) + ":" + std::to_string(bboxs[bi_max][1].x) + ":" + std::to_string(bboxs[bi_max][1].y));
					socketMat.sendString(bboxs_string);
				}

			}
			/*
			if(clicked == false && !mousePoint.empty())
			{
			for(int bi = 0; bi < bboxs.size(); ++bi)
			{
			Rect rectbox(bboxs[bi][0].x, bboxs[bi][0].y, bboxs[bi][1].x - bboxs[bi][0].x, bboxs[bi][1].y - bboxs[bi][0].y);
			if(rectbox.contains(mousePoint[0]))
			{
			cvtColor(inputImage, grayImg, CV_RGB2GRAY);
			ct.init(grayImg, box);
			trackbox = rectbox;
			clicked = true;
			}
			}
			socketMat.sendString("empty");
			}
			*/

		clock_t m_end = clock();
		cout <<"====================================="<<  CLOCKS_PER_SEC/ (double)(m_end - m_begin) << endl;
		}


	}

	socketMat.socketDisconnect();
}

void recvClick( int &clickX, int &clickY){
	SocketMatTransmissionServer socketMat;
	if (socketMat.socketConnect(7777) < 0)
	{
		return;
	}
	while(1){
		string clickstr;
		socketMat.receiveString(clickstr);
		vector<string> splitwords;
		SplitString(clickstr, splitwords, ":");
		int x = stod(splitwords[1]);
		int y = stod(splitwords[2]);
		if(x > 0 && y> 0){
			clickX=x;
			clickY=y;
			std::cout<<x<<":"<<y<<std::endl;
		}
	}
	socketMat.socketDisconnect();
}
void keyboard(bool &action_start){
	int ch;
	while(1)
	{
		ch = getch();
		if(ch == 's'||ch=='S')
		{
			action_start = !(action_start);
			cout<<"action_start: "<<action_start<<endl;
		}
	}
}
int main_deal()
{


	// ------------------------- POSE ESTIMATION AND RENDERING -------------------------
	// Step 1 - Read and load image, error if empty (possibly wrong path)

	int clickX = 0;
	int clickY = 0;
	bool action_start = false;
	//std::thread receiveMatThread(recvMat, std::ref(socketMat), std::ref(cvMatToOpInput), std::ref(poseExtractorCaffe));
	//std::thread receiveDoubleThread(recvDouble, std::ref(socketMat));
	std::thread receiveMatThread(recvMat, std::ref(clickX), std::ref(clickY), std::ref(action_start));
	std::thread receiveClickThread(recvClick, std::ref(clickX), std::ref(clickY));
	std::thread keyboardThread(keyboard, std::ref(action_start));
	receiveMatThread.join();
	receiveClickThread.join();
	keyboardThread.join();

	// Step 2 - Logging information message
	op::log("successfully finished.", op::Priority::High);
	// Return successful message
	return 0;
}



int main(int argc, char *argv[])
{
	// Initializing google logging (Caffe uses it for logging)
	google::InitGoogleLogging("openPose");

	// Parsing command line flags
	gflags::ParseCommandLineFlags(&argc, &argv, true);

	// Running openPoseTutorialPose1
	return main_deal();
}