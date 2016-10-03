#include <windows.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>
#include <set>
#include <memory>

template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args)
{
	auto size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

int bound(int i, int a, int b)
{
	return std::min(std::max(i, std::min(a, b)), std::max(a, b));
}

cv::Scalar getInverseColor(cv::Scalar c)
{
	return cv::Scalar(255, 255, 255, 0) - c;
}

struct imgLine
{
	size_t index;
	double length;
	size_t div;

	imgLine(size_t index, double length, size_t div
	) :index(index), length(length), div(div)
	{}

	bool operator<(imgLine const& a) const noexcept
	{
		return length / div > a.length / a.div;
	}
};

struct imgText
{
	std::string text;
	cv::Point org;
	cv::Scalar color;

	imgText(std::string text, cv::Point org, cv::Scalar color
	) :text(text), org(org), color(color)
	{}
};

cv::Mat src;
cv::Mat dst;
cv::Mat ori;
int n = 0;

std::vector<cv::Point2f> points;
std::vector<cv::Point2f> oriPoints;
std::vector<imgText> labels;

static bool checkCricle(cv::Point2f point)
{
	return !points.empty() && norm(point - points[0]) < 30;
}

// Draw a single point
static void draw_point(cv::Mat& img, cv::Point fp, cv::Scalar color)
{
	circle(img, fp, 2, color, CV_FILLED, CV_AA, 0);
}

// Draw a single point
static void draw_line(cv::Mat& img, cv::Point p1, cv::Point p2, cv::Scalar color = cv::Scalar(0, 0, 0))
{
	line(src, p1, p2,
		color, 2, cv::LineTypes::LINE_AA);
}

static void draw_text(cv::Mat& img, cv::String text, cv::Point org, cv::Scalar color = cv::Scalar(0, 0, 0))
{
	int baseline;
	auto size = getTextSize(text, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, 1, &baseline);
	cv::Point tmp_pt = { bound(org.x, 0, src.cols - size.width),
					bound(org.y, size.height + baseline, src.rows - 1 - baseline) };
	putText(src, text, tmp_pt, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, color, 1, cv::LineTypes::LINE_AA);
}

static void draw_text(cv::Mat& img, imgText label)
{
	draw_text(img, label.text, label.org, label.color);
}

// Draw delaunay triangles
static void draw_delaunay(cv::Mat& img, cv::Subdiv2D& subdiv, cv::Scalar delaunay_color)
{

	std::vector<cv::Vec6f> triangleList;
	subdiv.getTriangleList(triangleList);
	std::vector<cv::Point> pt(3);
	cv::Size size = img.size();
	cv::Rect rect(0, 0, size.width, size.height);

	for (size_t i = 0; i < triangleList.size(); i++)
	{
		cv::Vec6f t = triangleList[i];
		pt[0] = cv::Point(cvRound(t[0]), cvRound(t[1]));
		pt[1] = cv::Point(cvRound(t[2]), cvRound(t[3]));
		pt[2] = cv::Point(cvRound(t[4]), cvRound(t[5]));

		// Draw rectangles completely inside the image.
		if (rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
		{
			line(img, pt[0], pt[1], delaunay_color, 1, CV_AA, 0);
			line(img, pt[1], pt[2], delaunay_color, 1, CV_AA, 0);
			line(img, pt[2], pt[0], delaunay_color, 1, CV_AA, 0);
		}
	}
}

void addPoint()
{
	auto tmp = src.clone();

	auto division = std::set<imgLine>();

	for (size_t i = 0; i != points.size() - 1; ++i) {
		division.emplace(imgLine(i, norm(points[i] - points[i + 1]), 1));
	}

	for (size_t j = 0; j != 30; ++j) {
		auto i = division.begin();
		auto p = imgLine(i->index, i->length, i->div + 1);
		division.erase(i);
		division.emplace(p);
	}
	auto all = std::vector<cv::Point>();
	for (auto curLine : division) {
		auto st = points[curLine.index];
		auto ed = points[curLine.index + 1];
		int tot = curLine.div;
		for (auto i = 1; i != tot; ++i) {
			all.emplace_back((st * i + ed * (tot - i)) / tot);
		}
	}
	for (auto p : all) {
		circle(tmp, p, 2, cv::Scalar(200, 255, 0, 0), CV_FILLED, CV_AA, 0);
	}

	auto size = tmp.size();
	cv::Rect rect(0, 0, size.width, size.height);

	cv::Subdiv2D subdiv(rect);
	std::vector<cv::Point> pointsd;
	pointsd.insert(pointsd.end(), all.begin(), all.end());
	pointsd.insert(pointsd.end(), points.begin(), points.end());

	for (auto it = pointsd.begin(); it != pointsd.end(); ++it)
	{
		subdiv.insert(*it);
		// Show animation
		auto img_copy = tmp.clone();
		// Draw delaunay triangles
		draw_delaunay(img_copy, subdiv, cv::Scalar(0, 0, 255, 0));
		imshow("src", img_copy);
		cv::waitKey(500);
	}

	imshow("src", tmp);
}

void dragPoint(int event, int x, int y, int flags, void* ustc)
{
	src = ori.clone();
	static auto isDragging = false;
	static size_t m = 0;
	auto pt = cv::Point2f(x, y);
	auto min = norm(pt - points[0]);
	switch (event) {
	case CV_EVENT_MOUSEMOVE:
		if(isDragging) {
			points[m] = pt;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		for (size_t i = 0;i != points.size();++i)
		{
			if(min >= norm(pt - points[i] )) {
				m = i;
				min = norm(pt - points[m]);
			}
		}
		isDragging = true;
		break;
	case CV_EVENT_LBUTTONUP:
		isDragging = false;
		break;
	default:break;
	}

	auto size = cv::Size(src.cols, src.rows);
	cv::Mat dst2;
	dst = ori.clone();

	cv::Mat black(src.rows, src.cols, src.type(), cv::Scalar::all(0));
	cv::Mat mask(src.rows, src.cols, CV_8UC1, cv::Scalar(0));
	std::vector<std::vector<cv::Point> >  co_ordinates;
	co_ordinates.push_back(std::vector<cv::Point>{
		oriPoints[0],
			oriPoints[1],
			oriPoints[2]
	});

	cv::Mat mask2(src.rows, src.cols, CV_8UC1, cv::Scalar(0));
	std::vector<std::vector<cv::Point> >  co_ordinates2;
	co_ordinates2.push_back(std::vector<cv::Point>{
			points[0],
			points[1],
			points[2]
	});

	drawContours(mask, co_ordinates, 0, cv::Scalar(255), CV_FILLED, 8);
	drawContours(mask2, co_ordinates2, 0, cv::Scalar(255), CV_FILLED, 8);

	cv::Mat Kernel(cv::Size(3, 3), CV_8UC1);
	Kernel.setTo(cv::Scalar(1));
	dilate(mask, mask, Kernel, cv::Point(-1, -1), 3);

	cv::namedWindow("x", 1);
	imshow("x", mask);

	dst = black.clone();
	ori.copyTo(dst,mask);
	auto affTrans = getAffineTransform(oriPoints, points);
	warpAffine(dst, dst2, affTrans, size, cv::InterpolationFlags::INTER_AREA);

	src = ori.clone();
	dst2.copyTo(src,mask2);

	for (auto p : points) {
		draw_point(src, p, cv::Scalar(255, 200, 0, 0));
	}
	imshow("src", src);
}

void on_mouse(int event, int x, int y, int flags, void* ustc)
{
	cv::Point pt;
	std::string temp;

	auto clrPoint = cv::Scalar(255, 0, 0, 0);
	auto clrText = cv::Scalar(255, 200, 0, 0);
	if (event == CV_EVENT_MOUSEMOVE) {
		src = dst.clone();

		x = bound(x, 0, src.cols - 1);
		y = bound(y, 0, src.rows - 1);
		pt = cv::Point(x, y);

		if (points.size() >= 3 || checkCricle(pt)) {
			pt = points[0];
			temp = string_format("%s", "");
		}
		else {
			temp = string_format("%d (%d,%d)", n + 1, pt.x, pt.y);
		}

		draw_text(src, temp, pt, getInverseColor(clrText));
		draw_point(src, pt, getInverseColor(clrPoint));

		for (auto p : points) {
			draw_point(src, p, clrPoint);
		}

		for (auto l : labels) {
			draw_text(src, l);
		}

		if (points.size() > 1) {
			for (size_t i = 1; i != points.size(); ++i)
			{
				draw_line(src, points[i - 1], points[i]);
			}
		}

		if (points.size() >= 1) {
			draw_line(src, points[points.size() - 1], pt);
		}
		imshow("src", src);
	}else if (event == CV_EVENT_LBUTTONDOWN) {

		pt = cv::Point(x, y);

		if (points.size() >= 3 || checkCricle(pt)) {
			imshow("src", src);
			cv::setMouseCallback("src", nullptr, nullptr);
			cv::setMouseCallback("src", dragPoint, nullptr);
			oriPoints.insert(oriPoints.cend(), points.begin(), points.end());
		} else {
			points.push_back(pt);
			temp = string_format("%d (%d,%d)", n, pt.x, pt.y);
			labels.emplace_back(imgText(temp, pt, clrText));
			n++;
		}
	}else if (event == CV_EVENT_RBUTTONDOWN) {
		if (!points.empty()) {
			points.pop_back();
			labels.pop_back();
		}
	}
}

#ifdef _DEBUG
int main()
#else
int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
#endif // !DEBUG
{
	cv::namedWindow("src", 1);

	ori = cv::imread("qwe.jpg");
	src = dst = ori.clone();

	imshow("src", src);
	cv::setMouseCallback("src", on_mouse, nullptr);
	cvWaitKey(0);
	cvDestroyAllWindows();


	return 0;
}
