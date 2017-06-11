#include "base.h"

#define img_b(img,y,x) img.at<Vec3b>(y,x)[0]
#define img_g(img,y,x) img.at<Vec3b>(y,x)[1]
#define img_r(img,y,x) img.at<Vec3b>(y,x)[2]

using namespace cv;

void Pseudocolor(Mat &img8u,Mat &img_color)
{

	uchar tmp= 0;
	for(int y =0;y<img8u.rows;y++)
	{
		for(int x = 0;x<img8u.cols;x++)	
		{
			tmp = img8u.at<uchar>(y,x);
			if(tmp<=51)
			{
				img_b(img_color,y,x) = 255;
				img_g(img_color,y,x) = tmp*5;
				img_r(img_color,y,x) = 0;
			}
			else if(tmp<=102)
			{
				tmp -=51;
			    img_b(img_color,y,x) = 255-tmp*5;
				img_g(img_color,y,x) = 255;
				img_r(img_color,y,x) = 0;
			}
			else if(tmp<=153)
			{
				tmp-=102;
				img_b(img_color,y,x) = 0;
				img_g(img_color,y,x) = 255;
				img_r(img_color,y,x) = tmp*5;
			}
			else if(tmp<=204)
			{
				tmp -= 153;
				img_b(img_color,y,x) = 0;
				img_g(img_color,y,x) = 255-uchar(128.0*tmp/51.0+0.5);
				img_r(img_color,y,x) = 255;
			}
			else
			{
				tmp -= 204;
				img_b(img_color,y,x) = 0;
				img_g(img_color,y,x) = 127-uchar(127.0*tmp/51.0+0.5);
				img_r(img_color,y,x) = 255;
			}
		}
	}
}
