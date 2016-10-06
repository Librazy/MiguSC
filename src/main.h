#pragma once

#include <opencv2/core.hpp>
#include <utility>
#define EXPORTS __declspec(dllexport)

EXPORTS void setImg(cv::Mat& img);
EXPORTS std::pair<bool, cv::Mat> addPoint(int event, int x, int y);
EXPORTS std::pair<bool, cv::Mat> selectPoint(int event, int x, int y);
EXPORTS cv::Mat dragPoint(int event, int x, int y);