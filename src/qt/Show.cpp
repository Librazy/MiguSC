﻿
#include "Show.h"
#include "global.h"



Show::Show(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	/**********************窗口编辑******************************/
	//窗体标题
	this->setWindowTitle("Show");
	//窗口大小
	this->setMaximumSize(1000, 600);
	this->setMinimumSize(1000, 600);
	//默认窗体居中显示，如果想要更改用move 或setGeometry	
	//背景
	this->setStyleSheet("background:#111;font-size:12px;");//设置背景色
														   //this->setAttribute(Qt::WA_TranslucentBackground, true);//设置背景透明，与上一个冲突
	this->setWindowFlags(Qt::FramelessWindowHint);			//去除框架




	/**********************按钮******************************/
	back = new QPushButton("<", this);
	back->setStyleSheet("color: #666;background:#111;font-size:64px;border:none");
	back->setGeometry(QRect(10, 160, 96, 96));
	connect(back, SIGNAL(released()), this, SLOT(close()));

	keep = new QPushButton("Keep and Show to your friends!", this);
	keep->setStyleSheet("color: #00d50f;background:#122;font-size:18px;border:1px solid #00d50f;border-radius:4px");
	keep->setGeometry(QRect(220, 500, 540, 60));
	connect(keep, SIGNAL(released()), this, SLOT(FkeepImg()));
	connect(keep, SIGNAL(pressed()), this, SLOT(StyleChange()));

	/**********************图片载入******************************/
	QMovie*movie = new QMovie("editarea");

	label = new QLabel(this);
	label->setGeometry(220, 40, 540, 420);
	label->setScaledContents(true);
	//image = QImage("C:/Users/Administrator/Desktop/testPic.gif");
	//label->setPixmap(QPixmap::fromImage(image));	
	//label->show();
	label->setStyleSheet("border:4px solid #222;border-radius:2px;");
	label->setMovie(movie);
	movie->start();


}

void Show::FkeepImg()
{
	/*QString fileName = QFileDialog::getOpenFileName(
	this, "open image file",
	".",
	"Image files (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;All files (*.*)");
	*/

}

void Show::StyleChange()
{
	keep->setStyleSheet("color: #122;background:white;font-size:18px;border:1px solid white;border-radius:4px");
}
