#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
using namespace std;
using namespace cv;
double count_GM(){
	string Path = "E:\\4.jpg";
	Mat oriFrame = imread(Path);
	imshow("ori", oriFrame);
	Mat gammaFrame = MyGammaCorrection(oriFrame, 0.45);
	imshow("gammaFrame", gammaFrame);

	cout << 5;
	Mat channels[3];
	split(oriFrame, channels);//分离色彩通道
	imshow("channels0", channels[0]);
	imshow("channels1", channels[1]);
	imshow("channels2", channels[2]);

	Mat mask_neg1;
	Mat mask_0 = cv::Mat::zeros(oriFrame.size(), CV_32F);
	Mat mask_1 = cv::Mat::ones(oriFrame.size(), CV_32F);
	cv::subtract(mask_1, mask_0, mask_neg1);
	//imshow("mask", mask_neg1);
	//imshow("mask0", mask_0);
	//imshow("mask1", mask_1);

	double max_grad_magni = -10000000.0;
	/*
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y, x2y2, grad_magni;

	Sobel(oriFrame, grad_x, oriFrame.depth(), 1, 0);
	convertScaleAbs(grad_x, abs_grad_x);
	imshow("abs_grad_x", abs_grad_x);

	Sobel(oriFrame, grad_y, oriFrame.depth(), 0, 1);
	convertScaleAbs(grad_y, abs_grad_y);
	imshow("abs_grad_y", abs_grad_y);

	pow(abs_grad_x, 2.0f, abs_grad_x);
	pow(abs_grad_y, 2.0f, abs_grad_y);

	add(abs_grad_x, abs_grad_y, x2y2, noArray(), CV_32F);
	imshow("x2y2", x2y2);

	sqrt(x2y2, grad_magni);
	imshow("gradient magnitud", grad_magni);*/
	//if (cvMatEQ(x2y2, x2y2))
	//	cout << "haha";
	Mat mask_res0, mask_res1, mask_res2;
	double GME = 0.0;
	double sum_his = 0.0;
	Mat  tidu, jiaodu;


	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Sobel(channels[2], grad_x, oriFrame.depth(), 1, 0);
	convertScaleAbs(grad_x, abs_grad_x);
	imshow("abs_grad_x", abs_grad_x);

	Sobel(channels[2], grad_y, oriFrame.depth(), 0, 1);
	convertScaleAbs(grad_y, abs_grad_y);
	grad_x.convertTo(grad_x, CV_32FC3, 1 / 255.0);
	grad_y.convertTo(grad_y, CV_32FC3, 1 / 255.0);
	//cout << grad_x << endl;
	int his[180] = { 0 };
	cartToPolar(grad_x, grad_y, tidu, jiaodu, true);
	for (int col = 0; col < tidu.cols; col++){
		for (int row = 0; row < tidu.rows; row++){
			int td = (int)(*(tidu.data + tidu.step[0] * row + tidu.step[1] * col));
			int jd = (int)(*(jiaodu.data + tidu.step[0] * row + tidu.step[1] * col));
			if (td != 0){
				//cout << k << endl;
				//cout << k << "  " << atan(k) * 180/3.1415926 << endl;
				// his[int(floor(abs(atan((double)(*(mask_res.data + mask_res.step[0] * row + mask_res.step[1] * col))))))]++;
				if (jd >= 180){
					jd -= 180;
				}
				his[jd]++;
			}
		}
	}
	//cout << tidu<< endl;
	//cout << jiaodu << endl;
	for (int i = 0; i < 180; ++i){
		//cout << i << "  " << his[i] << endl;
		sum_his += his[i];
	}
	double pi = 0.0;
	for (int i = 0; i < 180; ++i){
		pi = his[i] / sum_his;
		GME -= pi*log(pi);
	}

	cout << GME << endl;
	return GME;
}
Mat& MyGammaCorrection(Mat& I, float fGamma){
	CV_Assert(I.data);
	// accept only char type matrices
	CV_Assert(I.depth() != sizeof(uchar));
	// build look up table
	unsigned char lut[256];
	for (int i = 0; i < 256; i++){
		lut[i] = pow((float)(i / 255.0), fGamma) * 255.0;
	}
	const int channels = I.channels();
	switch (channels){
	case 1:{
			  MatIterator_<uchar> it, end;
			  for (it = I.begin<uchar>(), end = I.end<uchar>(); it != end; it++)
				  //*it = pow((float)(((*it))/255.0), fGamma) * 255.0;
				  *it = lut[(*it)];
			  break;
	}
	case 3:{
			  MatIterator_<Vec3b> it, end;
			  for (it = I.begin<Vec3b>(), end = I.end<Vec3b>(); it != end; it++){
				  //(*it)[0] = pow((float)(((*it)[0])/255.0), fGamma) * 255.0;
				  //(*it)[1] = pow((float)(((*it)[1])/255.0), fGamma) * 255.0;
				  //(*it)[2] = pow((float)(((*it)[2])/255.0), fGamma) * 255.0;
				  (*it)[0] = lut[((*it)[0])];
				  (*it)[1] = lut[((*it)[1])];
				  (*it)[2] = lut[((*it)[2])];
			  }
			  break;
	}
	}
	return I;
}
bool cvMatEQ(const cv::Mat& data1, const cv::Mat& data2)
{
	bool success = true;
	// check if is multi dimensional  
	if (data1.dims > 2 || data2.dims > 2)
	{
		if (data1.dims != data2.dims || data1.type() != data2.type())
		{
			return false;
		}
		for (int dim = 0; dim < data1.dims; dim++){
			if (data1.size[dim] != data2.size[dim]){
				return false;
			}
		}
	}
	else
	{
		if (data1.size() != data2.size() || data1.channels() != data2.channels() || data1.type() != data2.type()){
			return false;
		}
	}
	int nrOfElements = data1.total()*data1.elemSize1();
	//bytewise comparison of data  
	int cnt = 0;
	for (cnt = 0; cnt < nrOfElements && success; cnt++)
	{
		if (data1.data[cnt] != data2.data[cnt]){
			success = false;
		}
	}
	return success;
}
int main(){
	string Path = "E:\\4.jpg";
	Mat oriFrame = imread(Path);
	imshow("ori", oriFrame);
	Mat gammaFrame = MyGammaCorrection(oriFrame, 0.45);
	imshow("gammaFrame", gammaFrame);
	
	cout << 5;
	Mat channels[3];
	split(oriFrame, channels);//分离色彩通道
	imshow("channels0", channels[0]);
	imshow("channels1", channels[1]);
	imshow("channels2", channels[2]);

	Mat mask_neg1;
	Mat mask_0 = cv::Mat::zeros(oriFrame.size(), CV_32F);
	Mat mask_1 = cv::Mat::ones(oriFrame.size(), CV_32F);
	cv::subtract(mask_1, mask_0, mask_neg1);
	//imshow("mask", mask_neg1);
	//imshow("mask0", mask_0);
	//imshow("mask1", mask_1);

	double max_grad_magni=-10000000.0;
	/*
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y, x2y2, grad_magni;

	Sobel(oriFrame, grad_x, oriFrame.depth(), 1, 0);
	convertScaleAbs(grad_x, abs_grad_x);
	imshow("abs_grad_x", abs_grad_x);

	Sobel(oriFrame, grad_y, oriFrame.depth(), 0, 1);
	convertScaleAbs(grad_y, abs_grad_y);
	imshow("abs_grad_y", abs_grad_y);

	pow(abs_grad_x, 2.0f, abs_grad_x);
	pow(abs_grad_y, 2.0f, abs_grad_y);

	add(abs_grad_x, abs_grad_y, x2y2, noArray(), CV_32F);
	imshow("x2y2", x2y2);

	sqrt(x2y2, grad_magni);
	imshow("gradient magnitud", grad_magni);*/
	//if (cvMatEQ(x2y2, x2y2))
	//	cout << "haha";
	Mat mask_res0, mask_res1, mask_res2;
	double GME=0.0;
	double sum_his=0.0;
	Mat  tidu, jiaodu;

	
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Sobel(channels[2], grad_x, oriFrame.depth(), 1, 0);
	convertScaleAbs(grad_x, abs_grad_x);
	imshow("abs_grad_x", abs_grad_x);

	Sobel(channels[2], grad_y, oriFrame.depth(), 0, 1);
	convertScaleAbs(grad_y, abs_grad_y);
	grad_x.convertTo(grad_x, CV_32FC3, 1 / 255.0);
	grad_y.convertTo(grad_y, CV_32FC3, 1 / 255.0);
	//cout << grad_x << endl;
	int his[180] = { 0 };
	cartToPolar(grad_x, grad_y, tidu, jiaodu, true);
	for (int col = 0; col < tidu.cols; col++){
		for (int row = 0; row < tidu.rows; row++){
			int td = (int)(*(tidu.data + tidu.step[0] * row + tidu.step[1] * col));
			int jd = (int)(*(jiaodu.data + tidu.step[0] * row + tidu.step[1] * col));
			if (td != 0){
				//cout << k << endl;
				//cout << k << "  " << atan(k) * 180/3.1415926 << endl;
				// his[int(floor(abs(atan((double)(*(mask_res.data + mask_res.step[0] * row + mask_res.step[1] * col))))))]++;
				if (jd >= 180){
					jd -= 180;
				}
				his[jd]++;
			}
		}
	}
	//cout << tidu<< endl;
	//cout << jiaodu << endl;
	for (int i = 0; i < 180; ++i){
		//cout << i << "  " << his[i] << endl;
		sum_his += his[i];
	}
	double pi = 0.0;
	for (int i = 0; i < 180; ++i){
		pi = his[i] / sum_his;
		GME -= pi*log(pi);
	}

	cout << GME << endl;
/*	for (int i = 0; i < 3; ++i){
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y, x2y2, grad_magni;
		Sobel(channels[i], grad_x, oriFrame.depth(), 1, 0);
		convertScaleAbs(grad_x, abs_grad_x);
		imshow("abs_grad_x", abs_grad_x);

		Sobel(channels[i], grad_y, oriFrame.depth(), 0, 1);
		convertScaleAbs(grad_y, abs_grad_y);
		imshow("abs_grad_y", abs_grad_y);

		pow(abs_grad_x, 2.0f, abs_grad_x);
		pow(abs_grad_y, 2.0f, abs_grad_y);

		add(abs_grad_x, abs_grad_y, x2y2, noArray(), CV_32F);
		imshow("x2y2", x2y2);

		sqrt(x2y2, grad_magni);
		imshow("gradient magnitud", grad_magni);

		//-1 0 1
		if (i == 0){
			multiply(mask_neg1, grad_magni, mask_res0);
			imshow("mask_res1", mask_res0);

		}else if (i == 1){
			multiply(mask_0, grad_magni, mask_res1);
			imshow("mask_res2", mask_res1);

		}else if (i == 2){
			multiply(mask_1, grad_magni, mask_res2);
			imshow("mask_res3", mask_res2);
		}
		int his[180] = {0};
		cout << 66<<endl;
	}*/
	waitKey(0);
	return 0;
}

