#include <windows.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>
#include <set>
#include <functional>

int bound(int i, int a, int b)
{
	return std::min(std::max(i, std::min(a, b)), std::max(a, b));
}

cv::Scalar getInverseColor(cv::Scalar c)
{
	return cv::Scalar(255, 255, 255, 0) - c;
}

cv::Mat src;
cv::Mat dst;
int n = 0;
std::vector<cv::Point> points;

bool checkCricle(cv::Point point)
{
	return norm(point - points[0]) < 30;
}

struct imgline
{
	size_t index;
	double length;
	size_t div;

	imgline(size_t index, double length, size_t div
	) :index(index), length(length), div(div)
	{}

	bool operator<(imgline const& a) const noexcept
	{
		return length / div > a.length / a.div;
	}
};

void addPoint()
{
	auto tmp = src.clone();

	auto division = std::set<imgline>();

	for (size_t i = 0; i != points.size() - 1; ++i) {
		division.emplace(imgline(i, norm(points[i] - points[i + 1]), 1));
	}

	for (size_t j = 0; j != 30; ++j) {
		auto i = division.begin();
		auto p = imgline(i->index, i->length, i->div + 1);
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
	imshow("src", tmp);
}

void on_mouse(int event, int x, int y, int flags, void* ustc)
{
	cv::Point pt;
	cv::Point tmp_pt = { -1,-1 };
	char temp[16];
	int baseline;

	auto clrPoint = cv::Scalar(255, 0, 0, 0);
	auto clrText = cv::Scalar(255, 200, 0, 0);
	auto clrTextCur = clrText;
	if (event == CV_EVENT_MOUSEMOVE) {
		src = dst.clone();

		x = bound(x, 0, src.cols - 1);
		y = bound(y, 0, src.rows - 1);
		pt = cv::Point(x, y);

		if (points.size() >= 3 && checkCricle(pt)) {
			pt = points[0];
			sprintf(temp, "%s", "");
		}
		else {
			sprintf(temp, "%d (%d,%d)", n + 1, pt.x, pt.y);
		}

		circle(src, pt, 2, clrPoint, CV_FILLED, CV_AA, 0);

		if (points.size() >= 1)
			line(src, points[points.size() - 1], pt,
				cv::Scalar(0, 0, 0), 2, cv::LineTypes::LINE_AA);


		clrTextCur = clrText;
	}
	else if (event == CV_EVENT_LBUTTONDOWN) {
		src = dst.clone();

		pt = cv::Point(x, y);

		if (points.size() >= 3 && checkCricle(pt)) {
			pt = points[0];
		}

		points.push_back(pt);
		n++;
		circle(src, pt, 2, clrPoint, CV_FILLED, CV_AA, 0);

		if (points.size() >= 2)
			line(src, points[points.size() - 2], points[points.size() - 1],
				cv::Scalar(0, 0, 0), 2, cv::LineTypes::LINE_AA);

		sprintf(temp, "%d (%d,%d)", n, pt.x, pt.y);
		clrTextCur = clrText;


		if (points.size() >= 3 && checkCricle(pt)) {
			imshow("src", src);
			cv::setMouseCallback("src", nullptr, nullptr);
			addPoint();
			return;
		}

		auto size = getTextSize(temp, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, 1, &baseline);
		tmp_pt.x = bound(pt.x, 0, src.cols - size.width);
		tmp_pt.y = bound(pt.y, size.height + baseline, src.rows - 1 - baseline);
		putText(src, temp, tmp_pt, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, clrTextCur, 1, cv::LineTypes::LINE_AA);

		dst = src.clone();
	}
	else if (event == CV_EVENT_RBUTTONDOWN) {
		if (!points.empty()) {
			pt = points.back();
			points.pop_back();
			circle(src, pt, 2, getInverseColor(clrPoint), CV_FILLED, CV_AA, 0);

			sprintf(temp, "%d (%d,%d)", n, pt.x, pt.y);
			--n;

			clrTextCur = getInverseColor(clrText);
			dst = src.clone();
		}
	}
	auto size = getTextSize(temp, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, 1, &baseline);
	tmp_pt.x = bound(pt.x, 0, src.cols - size.width);
	tmp_pt.y = bound(pt.y, size.height + baseline, src.rows - 1 - baseline);
	putText(src, temp, tmp_pt, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, clrTextCur, 1, cv::LineTypes::LINE_AA);

	imshow("src", src);
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

	src = cv::imread("qwe.jpg");
	dst = src.clone();
	imshow("src", src);
	cv::setMouseCallback("src", on_mouse, nullptr);
	cvWaitKey(0);
	cvDestroyAllWindows();


	return 0;
}
