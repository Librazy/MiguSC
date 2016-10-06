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



class QtGui : public QMainWindow
{
    Q_OBJECT

public:
    QtGui(QWidget *parent = Q_NULLPTR);
	QPushButton *close;
	QPushButton *openImg;
	QLabel *label;
	QImage image;

private:
    Ui::QtGuiClass ui;

private slots: //�������ܲۺ���
	void FopenImg();
	void StyleChange();
};
