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
#include<string>
#include "std_msgs/String.h"

#define BUFFER_SIZE 50000
#define PACKAGE_NUM 2
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
 
#define BLOCKSIZE IMG_WIDTH*IMG_HEIGHT*3/PACKAGE_NUM

static int server_fd;
static int client_fd;

using namespace cv;
using namespace std;

struct recvBuf
{
	char buf[BLOCKSIZE];
	int flag;
};
static struct recvBuf data_buf;
int receive(Mat& image)
{
        //struct recvBuf data;
	int returnflag = 0;
	cv::Mat img(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, cv::Scalar(0));
	int needRecv = sizeof(recvBuf);
	int count = 0;
	memset(&data_buf,0,sizeof(data_buf));
 
	for (int i = 0; i < PACKAGE_NUM; i++)
	{
		int pos = 0;
		int len0 = 0;
 
		while (pos < needRecv)
		{
			len0 = read(client_fd, (char*)(&data_buf) + pos, needRecv - pos);
			if (len0 < 0)
			{
				printf("Server Recieve Data Failed!\n");
				break;
			}
			pos += len0;
		}
 
		count = count + data_buf.flag;
 
		int num1 = IMG_HEIGHT / PACKAGE_NUM * i;
		for (int j = 0; j < IMG_HEIGHT / PACKAGE_NUM; j++)
		{
			int num2 = j * IMG_WIDTH * 3;
			uchar* ucdata = img.ptr<uchar>(j + num1);
			for (int k = 0; k < IMG_WIDTH * 3; k++)
			{
				ucdata[k] = data_buf.buf[num2 + k];
			}
		}
 
		if (data_buf.flag == 2)
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


int main(int argc,char **argv)
{

   struct sockaddr_in ser_addr;
   struct sockaddr_in cli_addr;
   //creat the socket;
   if((server_fd=socket(AF_INET,SOCK_STREAM,0))==-1)

     {
       	perror("socket failed!\n");
        exit(1);
     }
    cout<<"creat socket successful! socket="<<server_fd<<endl;
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_port=htons(44000);
    ser_addr.sin_addr.s_addr=inet_addr("192.168.1.106");
    if(bind(server_fd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr))==-1)
     {
      perror("bind error!\n");
      exit(1);
     }
    cout<<"bind successful!"<<endl;
    //set listen port
    if(listen(server_fd,5)==-1)
      {
       perror("bind error!\n");
       exit(1);
      }
     cout<<"set listen port sucessful!"<<endl;
     socklen_t sin_size =sizeof(struct sockaddr_in);
    if((client_fd=accept(server_fd,(struct sockaddr*)&cli_addr,&sin_size))==-1)
      {
	perror("accept error!");
	return 0;
       }
     cout<<"accept successful!"<<endl;

     ros::init(argc,argv,"video_ser_node");
     ros::NodeHandle nh;
     ros::Publisher pub = nh.advertise<std_msgs::String>("socket_string", 1000);
     cout<<"节点句柄成功！"<<endl;

  
  Mat image;
  //ros::Rate loop_rate(200);
while(1)
{
    if(receive(image)>0)
      {
         imshow("server_video", image);
         waitKey(30);
       }

}
cout<<"5"<<endl;
//close(client_fd);
close(server_fd);
return 0;
}
