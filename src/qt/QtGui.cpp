

#include "QtGui.h"
#include "Edit.h"

QString imgSrc;



QtGui::QtGui(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	/**********************���ڱ༭******************************/
	//��������
	this->setWindowTitle("QT");
	//���ڴ�С
	this->setMaximumSize(1000, 600);
	this->setMinimumSize(1000, 600);
	//Ĭ�ϴ���������ʾ��������Ҫ������move ��setGeometry	
	//����
	this->setStyleSheet("background:#111;font-size:12px;");//���ñ���ɫ
	//this->setAttribute(Qt::WA_TranslucentBackground, true);//���ñ���͸��������һ����ͻ
	this->setWindowFlags(Qt::FramelessWindowHint);			//ȥ������
	

	

	/**********************��ť******************************/
	close = new QPushButton("x", this);
	close->setStyleSheet("font-family:'Microsoft JhengHei UI Light';color: #666;font-size:18px;border:none");
	close->setGeometry(QRect(940, 10, 40, 40));
	connect(close, SIGNAL(released()), this, SLOT(close()));

	openImg = new QPushButton("Open Image from file", this);
	openImg->setStyleSheet("color: white;background:#122;font-size:18px;border:1px solid white;border-radius:4px");
	openImg->setGeometry(QRect(380, 500, 280, 60));
	connect(openImg, SIGNAL(released()), this, SLOT(FopenImg()));
	connect(openImg, SIGNAL(pressed()), this, SLOT(StyleChange()));

	/**********************ͼƬ����******************************/
	QMovie*movie = new QMovie("welcome.gif");
	

	
	label = new QLabel(this);
	label->setGeometry(120, 80, 760, 360);
	label->setScaledContents(true);
	//image = QImage("C:/Users/Administrator/Desktop/testPic.gif");
	//label->setPixmap(QPixmap::fromImage(image));	
	//label->show();

	label->setMovie(movie);
	movie->start();


}

void QtGui::FopenImg()
{
	imgSrc= QFileDialog::getOpenFileName(
	this, "open image file",
	".",
	"Image files (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;All files (*.*)");
	
	Edit *w=new Edit();
	w->show();


}

void QtGui::StyleChange()
{
	openImg->setStyleSheet("color: #122;background:white;font-size:18px;border:1px solid white;border-radius:4px");
}
