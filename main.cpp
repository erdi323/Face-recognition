#include <iostream>
#include "app_info.h"
#include "aip-cpp-sdk-4.16.1/face.h"//人脸识别头文件
#include "opencv2/opencv.hpp" //包含opencv相关头文件   
//ubuntu16.04.7位置：/usr/include/ ubuntu20.04.5位置：/usr/include/opencv4无法直接读取需使用-I指定路径
using namespace cv;
using namespace std;
#define log (std::cout << "[" << __FILE__ << ":第" << __LINE__ << "行 " << __FUNCTION__ << "] ")

int main()
{
    //打开摄像头接口文档查询
    //在opencv_v2.4.9文件的genindex.html文件查询VideoCapture
    //或index.html文件-> highgui. High-level GUI and Media I/O -> Reading and Writing Images and Video
    //->Reading and Writing Images and Video ->VideoCapture
    VideoCapture cap(0);//打开0号摄像头 需要链接-lopencv_highgui
    if (!cap.isOpened())//判断是否打开成功
    {
	cout << "摄像头打开失败" << endl;
	return -1;
    }
    cout << "摄像头打开成功" << endl;

    namedWindow("video", 1); //打开一个叫video的窗口 需要链接-lopencv_core 核心库

    Mat grayFrame;//存储图像被转换成灰色后的图像

    //对象检测文档查询			需要链接-lopencv_objdetect
    //index.html文件-> OpenCV API Reference-> objdetect. Object Detection ->CascadeClassifier
    //使用opencv提供的人脸图片特征分类结果文件构造一个分类器对象。
    //CascadeClassifier 变量名（用到的文件路径）
    CascadeClassifier classifier("/usr/share/opencv/haarcascades/haarcascade_frontalface_alt2.xml");
    vector<Rect> faceRectResults; //用于存储图像中检测到的人脸位置矩形集合  Rect为矩形类

    Mat faceFrame; //保存人脸图像截图
    vector<uchar> faceBuffer; //保存人脸图片数据的内存缓冲区 uchar为Unsigned char

    //生成调用智能云接口的客户端对象
    aip::Face client(AppID, APIKey, SecretKey);
    //使用Json::Value声明返回值变量，存储返回结果
    Json::Value result; //存储人脸搜索接口的返回结果


    while (true) //循环显示
    {
	Mat frame; //保存图像的变量
	cap >> frame; //从视频捕获对象中读取一帧图像

	//图片颜色处理文档查询
	//index.html文件-> imgproc. Image Processing -> Miscellaneous Image Transformations ->cvtColor
	//使用CV_BGR2GRAY参数将图像frame转换成灰色图像并存储到grayFrame
	//cvtColor(保存图像的变量, 储存改变后的图像变量,CV_BGR2GRAY);
	cvtColor(frame,grayFrame,CV_BGR2GRAY); //需要链接-lopencv_imgproc

	//图片对比度处理文档查询
	//index.html文件-> imgproc. Image Processing -> Miscellaneous Image Transformations ->Histograms -> equalizeHist
	equalizeHist(grayFrame,grayFrame);//调节灰色图像的对比度

	//使用CascadeClassifier内置的detectMultiScale检测输入图像中不同大小的对象。
	//检测到的对象将作为矩形列表返回。
	classifier.detectMultiScale(grayFrame,faceRectResults);
	//cout << "faceRectResults size: " << faceRectResults.size() << endl;

	if (!faceRectResults.empty())
	{
	    //在图像中绘制第一个人脸位置矩形，绘制成白色(255, 255, 255)
	    rectangle(grayFrame, faceRectResults[0], Scalar(255, 255, 255));
	    //截取灰色图像的人脸位置矩形区域，生成一个新的图像
	    faceFrame = grayFrame(faceRectResults[0]);

	    //将人脸图像编码成.jpg格式的图像数据，存储到缓冲区faceBuffer
	    imencode(".jpg", faceFrame, faceBuffer);

	    rectangle(frame, faceRectResults[0], Scalar(255, 255, 255));

	    //将图像数据faceBuffer转换成string类型数据
	    std::string faceImage(faceBuffer.begin(), faceBuffer.end());
	    //调用搜索接口
	    std::map<std::string, std::string> options; //可选参数，暂时传个空的
	    //可选参数，暂时传个空的
	    result = client.search(faceImage, "group01", options);
	    //cout << "---result---:\n" << result << endl; //打印返回结果
	    if (!result["result"].isNull()
	            && result["result"]["user_list"][0]["score"].asInt() > 80)
	    {
		//获取并输出用户名user_id
		cout << result["result"]["user_list"][0]["user_id"] << "  相识度："
		                                                    << result["result"]["user_list"][0]["score"] << endl
		                                                                                                 << __DATE__ << " " << __TIME__ << endl;//获取并输出时间戳

		//将用户名显示到图像上
		putText(frame,
		        result["result"]["user_list"][0]["user_id"].asString(),
		        Point(0, 100),
		        FONT_HERSHEY_SIMPLEX,
		        2,
		        Scalar(169, 50, 38)
		        );

		//将系统时间戳显示到图像上，使用__TIME__获取系统时间
		putText(frame,
		        std::string(__DATE__) + " " + __TIME__,
		        Point(0, 200),
		        FONT_HERSHEY_SIMPLEX,
		        1.5,
		        Scalar(169, 50, 38)
		        );

		waitKey(1500);//延迟1.5秒
	    }
	}

	//frame显示彩色图片（直拍），grayFrame显示经灰白处理后图片
	imshow("video", frame); //将图片显示到窗口video中
	if (waitKey(30) >= 0)//暂停30毫秒
	{
	    break;
	}
    }

    return 0;
}
