//Includes all the headers necessary to use the most common public pieces of the ROS system.
#include<ros/ros.h>
//Use image_transport for publishing and subscribing to images in ROS
#include<image_transport/image_transport.h>
//Use cv_bridge to convert between ROS and OpenCV Image formats
#include<cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
//Include headers for OpenCV Image processing
#include<opencv2/imgproc/imgproc.hpp>
//Include headers for OpenCV GUI handling
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core.hpp>
#include<opencv2/opencv.hpp>
#include<std_msgs/UInt8.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h> 
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h> 
#include<string.h> 
#include<sys/ioctl.h> 

using namespace cv;
using namespace std;
 
//Store all constants for image encodings in the enc namespace to be used later.  
//namespace enc = sensor_msgs::image_encodings; 

int image_socket(Mat inImg);
static Mat image1;
static int IMG_WIDTH, IMG_HEIGHT;
static int image_num = 1;
static int sockfd;
static int  BUFFER_SIZE;

#define PACKAGE_NUM 2
#define IMG_WIDTH 640	// 需传输图像的宽
#define IMG_HEIGHT 480	// 需传输图像的高
#define PACKAGE_NUM 2
//默认格式为CV_8UC3
#define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3/PACKAGE_NUM

struct sentbuf
{
   char buf[BUFFER_SIZE];
   int flag;
};

//static ros::Publisher capture;  
//This function is called everytime a new image_info message is published
void camInfoCallback(const sensor_msgs::CameraInfo & camInfoMsg)
{
   //Store the image width for calculation of angle
   //IMG_WIDTH = camInfoMsg.width;
   //cout<<imgWidth<<endl;
   //IMG_HEIGHT = camInfoMsg.height;

}
 
//This function is called everytime a new image is published
void imageCallback(const sensor_msgs::ImageConstPtr& original_image)
{
   //Convert from the ROS image message to a CvImage suitable for working with OpenCV for processing
   cv_bridge::CvImagePtr cv_ptr;
    try
    {
      //Always copy, returning a mutable CvImage  
//OpenCV expects color images to use BGR channel order.  
cv_ptr = cv_bridge::toCvCopy(original_image, sensor_msgs::image_encodings::BGR8);
 }
catch (cv_bridge::Exception& e)
{
//if there is an error during conversion, display it  
ROS_ERROR("tutorialROSOpenCV::main.cpp::cv_bridge exception: %s", e.what());
return;
}
image_socket(cv_ptr->image);
 
}
 
int image_socket(Mat image1)
{
         imshow("client_video", image1);
         waitKey(1);

      if (image1.empty())
	{
		printf("empty image1\n\n");
		return -1;
	}
 
	struct sentbuf data;
 
	for(int k = 0; k < PACKAGE_NUM; k++) 
	{
		int num1 = IMG_HEIGHT / PACKAGE_NUM * k;
		for (int i = 0; i < IMG_HEIGHT / PACKAGE_NUM; i++)
		{
			int num2 = i * IMG_WIDTH * 3;
			uchar* ucdata = image1.ptr<uchar>(i + num1);
			for (int j = 0; j < IMG_WIDTH * 3; j++)
			{
				data.buf[num2 + j] = ucdata[j];
			}
		}
 
		if(k == PACKAGE_NUM - 1)
			data.flag = 2;
		else
			data.flag = 1;
 
		if (write(sockfd, (char *)(&data), sizeof(data)) < 0)
		{
			printf("send image1 error: %s(errno: %d)\n", strerror(errno), errno);
			return -1;
		}
	}

}
 

 //This is ROS node to track the destination image

int main(int argc, char **argv)
{
ros::init(argc, argv, "image_socket");
ROS_INFO("-----------------");

struct sockaddr_in serv_addr;
//创建socket
if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
{
perror("Socket failed!\n");
exit(1);
}
printf("Socket id = %d\n",sockfd);
//设置sockaddr_in 结构体中相关参数
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(44000);
serv_addr.sin_addr.s_addr = inet_addr("192.168.1.106");
//inet_pton(AF_INET, servInetAddr, &serv_addr.sin_addr);
bzero(&(serv_addr.sin_zero), 8);
//调用connect 函数主动发起对服务器端的连接
 if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr))== -1)
 {
 perror("Connect failed!\n");
exit(1);
}
 printf("welcome\n");

ros::NodeHandle nh;
 image_transport::ImageTransport it(nh);
 
image_transport::Subscriber sub = it.subscribe("/usb_cam/image_raw", 1, imageCallback);
ros::Subscriber camInfo= nh.subscribe("/usb_cam/camera_info", 1, camInfoCallback);
ros::Rate loop_rate(100);
 
while (ros::ok())
{
 ros::spinOnce();
 loop_rate.sleep();
}
 
 close(sockfd);
//ROS_INFO is the replacement for printf/cout.
ROS_INFO("No error.");
return 0;
 
}
