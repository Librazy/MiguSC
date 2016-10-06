#pragma once

#include <opencv/cv.h> 
#include <opencv2/opencv.hpp>

#include <QtWidgets/QApplication>

#include <QtWidgets/QMainWindow>
#include "ui_QtGui.h"
#include <QPushButton>
#include <QLabel>
#include <QPaintEvent>  
#include <QPainter>  
#include <QPixmap>  
#include <QImage>  
#include <QGraphicsView>  
#include <QGraphicsScene>  
#include <QFileDialog>  
#include<QMovie>

using namespace cv;



class Show : public QMainWindow
{
	Q_OBJECT

public:
	Show(QWidget *parent = Q_NULLPTR);
	QPushButton *back;
	QPushButton *keep;
	QLabel *label;
	QImage image;
	QMovie movie;

private:
	Ui::QtGuiClass ui;

	private slots: //�������ܲۺ���
	void FkeepImg();
	void StyleChange();
};
#pragma once
