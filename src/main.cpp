#ifdef _MSC_VER
#include <windows.h>
#endif

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>
#include <set>
#include <memory>

#include "triangle/util.h"
#include "triangle/triangle_internal.h"

template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args)
{
	auto size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

static int bound(int i, int a, int b)
{
	return std::min(std::max(i, std::min(a, b)), std::max(a, b));
}

static cv::Scalar getInverseColor(cv::Scalar c)
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
	cv::Point orgi;
	cv::Scalar color;

	imgText(std::string text, cv::Point orgi, cv::Scalar color
	) :text(text), orgi(orgi), color(color)
	{}
};

cv::Mat src;
cv::Mat dst;
cv::Mat ori;
int n = 0;

std::vector<cv::Point2f> points;
std::vector<cv::Point2f> oriPoints;
std::vector<imgText> labels;

static const std::string mainWindowName = "Affine with CUDA support";

static bool if_in_range(cv::Point2f point)
{
	return !points.empty() && norm(point - points[0]) < 30;
}

static void draw_point(cv::Mat& img, cv::Point fp, cv::Scalar color)
{
	circle(img, fp, 2, color, CV_FILLED, CV_AA, 0);
}

static void draw_line(cv::Mat& img, cv::Point p1, cv::Point p2, cv::Scalar color = cv::Scalar(0, 0, 0))
{
	line(img, p1, p2,
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
	draw_text(img, label.text, label.orgi, label.color);
}

static int triangle_create()
{
	auto ctx = triangle_context_create();
	auto in = new triangleio();
	reset_triangleio(in);
	triangle_context_options(ctx, "pq10a256");
	in->numberofsegments = oriPoints.size();
	in->numberofpoints = oriPoints.size();

	in->segmentlist = static_cast<int *>(malloc(in->numberofsegments * 2 * sizeof(int)));
	in->segmentmarkerlist = static_cast<int *>(malloc(in->numberofsegments * sizeof(int)));


	in->pointlist = static_cast<REAL *>(malloc(in->numberofpoints * 2 * sizeof(REAL)));
	in->pointmarkerlist = static_cast<int *>(malloc(in->numberofpoints * sizeof(int)));

	for (size_t i = 0; i != in->numberofsegments; ++i) {

		in->pointlist[i * 2] = oriPoints[i].x;
		in->pointlist[i * 2 + 1] = oriPoints[i].y;
		in->pointmarkerlist[i] = 2;

		in->segmentlist[i * 2] = i + 1;
		in->segmentlist[i * 2 + 1] = (i + 1) % in->numberofsegments + 1;

		in->segmentmarkerlist[i] = 2;
	}
	auto vertexsFin = std::vector<cv::Point2d>();
	auto segmentsFin = std::vector<cv::Point>();
	auto trisegsFin = std::vector<cv::Point>();

	auto res = triangle_mesh_create(ctx, in);

	if (!res) {
		struct osub subsegloop;
		vertex endpoint1, endpoint2;
		long subsegnumber;

		auto m = ctx->m;
		auto b = ctx->b;

		vertex vertexloop;
		int vertexnumber;

		dst = ori.clone();

		traversalinit(&m->vertices);
		vertexnumber = 0;
		vertexloop = vertextraverse(m);
		while (vertexloop != static_cast<vertex>(nullptr)) {
			if (!b->jettison || vertextype(vertexloop) != UNDEADVERTEX) {
				vertexsFin.emplace_back(cv::Point2d{ vertexloop[0], vertexloop[1] });
				setvertexmark(vertexloop, vertexnumber);
				vertexnumber++;
			}
			vertexloop = vertextraverse(m);
		}

		for (auto p : vertexsFin) {
			draw_point(dst, p , cv::Scalar(0, 200, 0, 0));
		}

		traversalinit(&m->subsegs);
		subsegloop.ss = subsegtraverse(m);
		subsegloop.ssorient = 0;
		subsegnumber = b->firstnumber;
		while (subsegloop.ss != static_cast<subseg *>(nullptr)) {
			sorg(subsegloop, endpoint1);
			sdest(subsegloop, endpoint2);
			segmentsFin.emplace_back(cv::Point{ vertexmark(endpoint1), vertexmark(endpoint2) });
			subsegloop.ss = subsegtraverse(m);
			subsegnumber++;
		}
		
		struct otri triangleloop;
		vertex p1, p2, p3;
		long elementnumber;

		int plus1mod3[3] = { 1, 2, 0 };
		int minus1mod3[3] = { 2, 0, 1 };

		traversalinit(&m->triangles);
		triangleloop.tri = triangletraverse(m);
		triangleloop.orient = 0;
		elementnumber = b->firstnumber;
		while (triangleloop.tri != static_cast<triangle *>(nullptr)) {
			org(triangleloop, p1);
			dest(triangleloop, p2);
			apex(triangleloop, p3);
			trisegsFin.emplace_back(cv::Point{ vertexmark(p1), vertexmark(p2) });
			trisegsFin.emplace_back(cv::Point{ vertexmark(p2), vertexmark(p3) });
			trisegsFin.emplace_back(cv::Point{ vertexmark(p3), vertexmark(p1) });

			triangleloop.tri = triangletraverse(m);
			elementnumber++;
		}
		if (trisegsFin.size() >= 1) {
			for (auto se : trisegsFin) {
				line(dst, vertexsFin[se.x] , vertexsFin[se.y], cv::Scalar(0, 255, 255), 1, CV_AA, 0);
			}
		}
		if (segmentsFin.size() >= 1) {
			for (auto se : segmentsFin) {
				line(dst, vertexsFin[se.x] , vertexsFin[se.y] , cv::Scalar(0,0,255), 1, CV_AA, 0);
			}
		}
		cv::namedWindow("x", 1);
		imshow("x", dst);

		free(in->segmentlist);
		free(in->segmentmarkerlist);


		free(in->pointlist);
		free(in->pointmarkerlist);

		delete in;
	}

	return res;
}

static void mouse_event_handle(int event, int x, int y, int flags, void* ustc)
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

		if (points.size() >= 3 && if_in_range(pt)) {
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
		imshow(mainWindowName, src);
	} else if (event == CV_EVENT_LBUTTONDOWN) {

		pt = cv::Point(x, y);

		if (points.size() >= 3 && if_in_range(pt)) {
			imshow(mainWindowName, src);
			cv::setMouseCallback(mainWindowName, nullptr, nullptr);
			//cv::setMouseCallback(mainWindowName, drag_point, nullptr);
			oriPoints.insert(oriPoints.cend(), points.begin(), points.end());
			//addPoint();
			triangle_create();
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



#ifdef _MSC_VER
#ifdef _DEBUG
int main()
#else
int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
#endif // !DEBUG

#elif
int main()
#endif// _MSC_VER
{
	cv::namedWindow(mainWindowName, 1);

	ori = cv::imread("qwe.jpg");
	src = dst = ori.clone();

	imshow(mainWindowName, src);
	cv::setMouseCallback(mainWindowName, mouse_event_handle, nullptr);

	while(cv::waitKey(0) != 27) {
		
	}

	cv::destroyAllWindows();


	return 0;
}
