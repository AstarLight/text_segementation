#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <algorithm> 
using namespace cv;


#define V_PROJECT 1
#define H_PROJECT 2

typedef struct
{
	int begin;
	int end;

}char_range_t;

void draw_projection(vector<int>& pos, int mode)
{
	vector<int>::iterator max = std::max_element(std::begin(pos), std::end(pos)); //�����ֵ
	if (mode == H_PROJECT)
	{
		int height = pos.size();
		int width = *max;
		Mat project = Mat::zeros(height, width, CV_8UC1);
		for (int i = 0; i < project.rows; i++)
		{
			for (int j = 0; j < pos[i]; j++)
			{
				project.at<uchar>(i, j) = 255;
			}
		}
		cvNamedWindow("horizational projection", 0);
		imshow("horizational projection", project);

	}
	else if (mode == V_PROJECT)
	{
		int height = *max;
		int width = pos.size();
		Mat project = Mat::zeros(height, width, CV_8UC1);
		for (int i = 0; i < project.cols; i++)
		{
			for (int j = project.rows - 1; j >= project.rows - pos[i]; j--)
			{
				//std::cout << "j:" << j << "i:" << i << std::endl;
				project.at<uchar>(j, i) = 255;
			}
		}

		imshow("vertical projection", project);
	}

	//waitKey();
}

//��ȡ�ı���ͶӰ���ڷָ��ַ�(��ֱ��ˮƽ)
int GetTextProjection(Mat &src, vector<int>& pos, int mode)
{
	if (mode == V_PROJECT)
	{
		for (int i = 0; i < src.rows; i++)
		{
			uchar* p = src.ptr<uchar>(i);
			for (int j = 0; j < src.cols; j++)
			{
				if (p[j] == 0)
				{
					pos[j]++;
				}
			}
		}

		draw_projection(pos, V_PROJECT);
	}
	else if (mode == H_PROJECT)
	{
		for (int i = 0; i < src.cols; i++)
		{

			for (int j = 0; j < src.rows; j++)
			{
				if (src.at<uchar>(j, i) == 0)
				{
					pos[j]++;
				}
			}
		}
		draw_projection(pos, H_PROJECT);

	}	

	return 0;
}

//��ȡÿ���ָ��ַ��ķ�Χ��min_thresh���������С���ȣ�min_range�������������С���
int GetPeekRange(vector<int> &vertical_pos, vector<char_range_t> &peek_range, int min_thresh = 2, int min_range = 10)
{
	int begin = 0;
	int end = 0;
	for (int i = 0; i < vertical_pos.size(); i++)
	{

		if (vertical_pos[i] > min_thresh && begin == 0)
		{
			begin = i;
		}
		else if (vertical_pos[i] > min_thresh && begin != 0)
		{
			continue;
		}
		else if (vertical_pos[i] < min_thresh && begin != 0)
		{
			end = i;
			if (end - begin >= min_range)
			{
				char_range_t tmp;
				tmp.begin = begin;
				tmp.end = end;
				peek_range.push_back(tmp);
				begin = 0;
				end = 0;
			}

		}
		else if (vertical_pos[i] < min_thresh || begin == 0)
		{
			continue;
		}
		else
		{
			//printf("raise error!\n");
		}
	}

	return 0;
}




inline void save_cut(const Mat& img, int id)
{
	char name[128] = { 0 };
	sprintf(name, "./save_cut/%d.jpg", id);
	imwrite(name, img);
}

//�и��ַ�
int CutChar(Mat &img, const vector<char_range_t>& v_peek_range, const vector<char_range_t>& h_peek_range, vector<Mat>& chars_set)
{
	static int count = 0;
	int norm_width = img.rows;  //��Ϊ���ֶ����������εģ��������Ƕ���norm_width���Ǻ��ֵĸ߶�
	Mat show_img = img.clone();
	cvtColor(show_img, show_img, CV_GRAY2BGR);
	for (int i = 0; i < v_peek_range.size(); i++)
	{
		int char_gap = v_peek_range[i].end - v_peek_range[i].begin;
		//if (char_gap <= (int)(norm_width*1.2) && char_gap >= (int)(norm_width*0.8))
		{
			int x = v_peek_range[i].begin - 2>0 ? v_peek_range[i].begin - 2 : 0;
			int width = char_gap + 4 <= img.rows ? char_gap : img.rows;
			Rect r(x, 0, width, img.rows);			
			rectangle(show_img, r, Scalar(255, 0, 0), 1);
			Mat single_char = img(r).clone();
			chars_set.push_back(single_char);
			save_cut(single_char, count);
			count++;
		}
	}

	imshow("cut", show_img);
	waitKey();

	return 0;
}

Mat cut_one_line(const Mat& src,int begin,int end)
{
	Mat line = src(Rect(0,begin,src.cols,end-begin)).clone();
	return line;
}


vector<Mat> CutSingleChar(Mat& img)
{
	Mat show = img.clone();
	cvtColor(show, show, CV_GRAY2BGR);
	threshold(img, img, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	imshow("binary", img);
	vector<int> horizion_pos(img.rows, 0);
	vector<char_range_t> h_peek_range;
	GetTextProjection(img, horizion_pos, H_PROJECT);
	GetPeekRange(horizion_pos, h_peek_range, 2, 10);

#if 1

	/*��ÿһ�ı����и�*/
	vector<Mat> lines_set;
	//vector<Mat> lines_set_show;
	for (int i = 0; i < h_peek_range.size(); i++)
	{
		Mat line = cut_one_line(img, h_peek_range[i].begin, h_peek_range[i].end);
		lines_set.push_back(line);		
		//Mat line_show = show(Rect(0, h_peek_range[i].begin, show.cols, h_peek_range[i].end - h_peek_range[i].begin));
		//lines_set_show.push_back(line_show);
	}

	vector<Mat> chars_set;
	for (int i = 0; i < lines_set.size(); i++)
	{
		Mat line = lines_set[i];
		//Mat line2 = lines_set_show[i];
		imshow("raw line", line);
		vector<int> vertical_pos(line.cols, 0);
		vector<char_range_t> v_peek_range;
		GetTextProjection(line, vertical_pos, V_PROJECT);
		GetPeekRange(vertical_pos, v_peek_range);
		CutChar(line, v_peek_range, h_peek_range, chars_set);
		//CutChar(line2, v_peek_range, h_peek_range, chars_set);
	}
#endif

	//imshow("line_show", show);
	//imwrite("show.png", show);
	return chars_set;
}


int main()
{
	Mat img = imread("16.png", 0);
	imshow("src", img);
	resize(img, img, Size(), 2, 2, INTER_LANCZOS4);
	vector<Mat> chars_set = CutSingleChar(img);

	for (int i = 0; i < chars_set.size(); i++)
	{
		/*�ַ�ʶ��*/
	}

	waitKey();
	return 0;
}