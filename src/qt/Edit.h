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
#include <QMovie>


using namespace cv;



class Edit : public QMainWindow
{
	Q_OBJECT

public:
	Edit(QWidget *parent = Q_NULLPTR);
	void editAnchor();
	void editDeformation();

	QPushButton *finish1;
	QPushButton *finish2;
	QPushButton *finish3;
	QLabel *imglabel;
	QLabel *imglabel2;
	QLabel *insRect;
	QLabel *instruction;
	QLabel *parameter;
	QImage editarea;
	Mat cvimg;
	QImage example;

	int step;

	double Px;
	double Py;
private:
	Ui::QtGuiClass ui;

private slots:
	void step1(); 
	void step2();
	void step3();
	void StyleChange1() const;
	void StyleChange2() const;
	void StyleChange3() const;
public:
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	bool callAddPoint(QMouseEvent *m, int type);
};

