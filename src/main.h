#pragma once

#include <opencv2/core.hpp>
#include <utility>

static void setImg(cv::Mat& img);
static std::pair<bool, cv::Mat> addPoint(int event, int x, int y);
static std::pair<bool, cv::Mat> selectPoint(int event, int x, int y);
static cv::Mat dragPoint(int event, int x, int y);