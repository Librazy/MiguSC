#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <memory>

#include "triangle/triangle_internal.h"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace Migu {
	using OpenMeshT = OpenMesh::DefaultTraits;
	using TriMs = OpenMesh::TriMesh_ArrayKernelT<>;
	using Tpt_d = Eigen::Triplet<double>;
	using Spm_d = Eigen::SparseMatrix<double>;
	using Matx2_d = Eigen::Matrix<double, Eigen::Dynamic,2>;
}
using namespace Migu;

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

static cv::Scalar get_inverseColor(cv::Scalar c)
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

static void draw_point(cv::Mat& img, cv::Point fp, cv::Scalar color, int rad = 2)
{
	circle(img, fp, rad, color, CV_FILLED, CV_AA, 0);
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
	cv::Point tmp_pt = { bound(org.x, 0, img.cols - size.width),
					bound(org.y, size.height + baseline, img.rows - 1 - baseline) };
	putText(img, text, tmp_pt, cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 0.5, color, 1, cv::LineTypes::LINE_AA);
}

static void draw_text(cv::Mat& img, imgText label)
{
	draw_text(img, label.text, label.orgi, label.color);
}
static const std::string mainWindowName = "Iterative Linear Solvers";

static bool if_in_range(cv::Point2d const& point, std::vector<cv::Point2d> const& points)
{
	return !points.empty() && norm(point - points[0]) < 30;
}