#include <main.h>
#include <opencv2/highgui.hpp>
#include "Edit.h"
#include "Show.h"
#include "global.h"

Edit::Edit(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	step = 1;
	this->setMouseTracking(true);
	this->setWindowTitle("Edit");
	//´°¿Ú´óÐ¡
	this->setMaximumSize(1000, 600);
	this->setMinimumSize(1000, 600);	
	this->setStyleSheet("background:#111;font-size:12px;"); 
	this->setWindowFlags(Qt::FramelessWindowHint);

	imglabel = new QLabel(this);
	imglabel->setGeometry(40, 30, 740, 540);
	imglabel->setScaledContents(true);
	imglabel->setStyleSheet("border:4px solid #222;border-radius:2px;");
	editarea = QImage(imgSrc);
	imglabel->setPixmap(QPixmap::fromImage(editarea));	
	imglabel->show();	
	imglabel->setMouseTracking(true);
	auto cvimg = imread(imgSrc.toStdString());
	setImg(cvimg);

	insRect = new QLabel(this);
	insRect->setGeometry(800, 30, 160, 460);
	insRect->setScaledContents(true);
	insRect->setStyleSheet("border:1px solid #999;background:#222;border-radius:4px");
	insRect->show();

	instruction = new QLabel(this);
	instruction->setGeometry(810, 240, 140, 240);
	instruction->setScaledContents(true);
	instruction->setStyleSheet("font-family:'Microsoft JhengHei UI Light';color:#fff;font-size 12px;background:#222");
	instruction->setText("<h2>Take the Mask</h2><p>Use line to draw<br/> an area which contains<br/>the area you want to <br/>deformed . <br/> <br/> If you want make <br/> the deformed in <br/> the face,then you <br/>should take all face<br/>in the area.</p>");
	instruction->show();

	imglabel2 = new QLabel(this);
	imglabel2->setGeometry(810, 40, 140, 180);
	imglabel2->setScaledContents(true);
	example = QImage("example1.jpg");
	imglabel2->setPixmap(QPixmap::fromImage(example));
	imglabel2->show();
	
	finish1 = new QPushButton("Next Step", this);
	finish1->setStyleSheet("font-family:'Microsoft JhengHei UI';color:rgb(220,120,13);font-size:18px;border:1px solid rgb(220,120,13);border-radius:4px;");
	finish1->setGeometry(QRect(800, 520, 160, 50));
	finish1->show();
	connect(finish1, SIGNAL(released()), this, SLOT(step1()));
	connect(finish1, SIGNAL(pressed()), this, SLOT(StyleChange1()));
	

}

void Edit::editAnchor()
{	
	instruction->close();
	imglabel2->close();
	insRect->close();
	finish1->close();

	step = 2;
	insRect->setGeometry(800, 30, 160, 400);
	insRect->setStyleSheet("border:1px solid #999;background:#222;border-radius:4px");
	insRect->show();

	instruction->setGeometry(810, 240, 140, 180);
	instruction->setText("<h2>Anchor the Points</h2><p>Anchor the anchor point<br/> on the image, the <br/> anchor part will not be <br/>deformed to move. <br/> <br/> If the face is pinned, <br/> if the body is pegged<br/> joints.</p>");
	instruction->show();

	example = QImage("example2.jpg");
	imglabel2->setPixmap(QPixmap::fromImage(example));
	imglabel2->show();

	parameter = new QLabel(this);
	parameter->setGeometry(805, 445, 150, 60);
	parameter->setScaledContents(true);
	parameter->setStyleSheet("font-family:'Microsoft JhengHei UI Light';color:#666;font-size 12px;");
	parameter->setText("Num of Points: 0<br/>X: 0; Y: 0<br/>Width:  740;   Height: 540<br/>");
	parameter->show();

	finish2 = new QPushButton("Next Step", this);
	finish2->setStyleSheet("font-family:'Microsoft JhengHei UI';color:rgb(250,60,13);font-size:18px;border:1px solid rgb(200,90,13);border-radius:4px;");
	finish2->setGeometry(QRect(800, 520, 160, 50));
	finish2->show();
	connect(finish2, SIGNAL(released()), this, SLOT(step2()));
	connect(finish2, SIGNAL(pressed()), this, SLOT(StyleChange2()));
}
void Edit::editDeformation() 
{
	step = 3;

	instruction->close();
	imglabel2->close();
	insRect->close();
	parameter->close();
	finish2->close();

	insRect->setGeometry(800, 30, 160, 460);
	insRect->setStyleSheet("border:1px solid #999;background:#222;border-radius:4px");
	insRect->show();

	instruction->setGeometry(810, 240, 140, 240);
	instruction->setText("<h2>Deform the face</h2><p>On the image,you<br/> just get some anchors. <br/> Now you can drag other <br/>area to deform the <br/>image. <br/><br/> The part near the <br/>anchors will not moved,<br/> so you can deform it <br/>by your mouse.<br/>So,just smile.</p>");
	instruction->show();

	example = QImage("example3.jpg");
	imglabel2->setPixmap(QPixmap::fromImage(example));
	imglabel2->show();


	finish3 = new QPushButton("Finish", this);
	finish3->setStyleSheet("font-family:'Microsoft JhengHei UI';color:rgb(255,20,13);font-size:18px;border:1px solid rgb(255,20,13);border-radius:4px;");
	finish3->setGeometry(QRect(800, 520, 160, 50));
	finish3->show();
	connect(finish3, SIGNAL(released()), this, SLOT(step3(),close()));
	connect(finish3, SIGNAL(pressed()), this, SLOT(StyleChange3()));

}

void Edit::step1()
{
	editAnchor();
}

void Edit::step2()
{
	editDeformation();
}
void Edit::step3()
{
	Show *w = new Show();
	w->show();
	finish3->setStyleSheet("font-family:'Microsoft JhengHei UI';color:rgb(255,20,13);font-size:18px;border:1px solid rgb(255,20,13);border-radius:4px;");
	this->hide();
}

void Edit::StyleChange1() const
{
	finish1->setStyleSheet("font-family:'Microsoft JhengHei UI';color:#fff;background: rgb(220,120,13);font-size:18px;border:1px solid #fff;border-radius:4px;");
}
void Edit::StyleChange2() const
{
	finish2->setStyleSheet("font-family:'Microsoft JhengHei UI';color:#fff;background: rgb(250,60,13);font-size:18px;border:1px solid #fff;border-radius:4px;");
}
void Edit::StyleChange3() const
{
	finish3->setStyleSheet("font-family:'Microsoft JhengHei UI';color:#fff;background: rgb(255,20,10);font-size:18px;border:1px solid #fff;border-radius:4px;");
}

QPoint labelPos(40,30);

bool Edit::callAddPoint(QMouseEvent *m, int type)
{
	Px = (static_cast<double>(m->pos().x()) - labelPos.x()) / static_cast<double>(imglabel->size().width());
	Py = (static_cast<double>(m->pos().y()) - labelPos.y()) / static_cast<double>(imglabel->size().height());
	auto imgx = static_cast<int>(Px * editarea.size().width());
	auto imgy = static_cast<int>(Py * editarea.size().height());
	auto add = addPoint(type, imgx, imgy);
	auto pic = add.second;
	cv::Mat rgb;
	cvtColor(pic, rgb, CV_BGR2RGB);
	auto img = QImage(static_cast<const unsigned char*>(rgb.data), rgb.cols, rgb.rows, QImage::Format_RGB888);
	imglabel->setPixmap(QPixmap::fromImage(img));
	return add.first;
}

void Edit::mousePressEvent(QMouseEvent *m)
{
	QRect labelRect = QRect(labelPos, imglabel->size());

	if (labelRect.contains(m->pos())) {
		
		if (m->buttons() & Qt::LeftButton) {

			if (step == 1)
			{
				callAddPoint(m,CV_EVENT_LBUTTONDOWN);
			}
		} else if (m->buttons() & Qt::RightButton) {
			if (step == 1)
			{
				callAddPoint(m, CV_EVENT_RBUTTONDOWN);
			}
		}
	} 
}
void Edit::mouseMoveEvent(QMouseEvent *m)
{
	if (step == 1)
	{
		if (!(m->buttons() & Qt::LeftButton)) {
			callAddPoint(m,CV_EVENT_MOUSEMOVE);
		}
	}
}

