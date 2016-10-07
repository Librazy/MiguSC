#include "QtGui.h"
#include "Edit.h"

QString imgSrc;



QtGui::QtGui(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	this->setWindowTitle("QT");

	this->setMaximumSize(1000, 600);
	this->setMinimumSize(1000, 600);

	this->setStyleSheet("background:#111;font-size:12px;");
	this->setWindowFlags(Qt::FramelessWindowHint);

	

	close = new QPushButton("x", this);
	close->setStyleSheet("font-family:'Microsoft JhengHei UI Light';color: #666;font-size:18px;border:none");
	close->setGeometry(QRect(940, 10, 40, 40));
	connect(close, SIGNAL(released()), this, SLOT(close()));

	openImg = new QPushButton("Open Image from file", this);
	openImg->setStyleSheet("color: white;background:#122;font-size:18px;border:1px solid white;border-radius:4px");
	openImg->setGeometry(QRect(380, 500, 280, 60));
	connect(openImg, SIGNAL(released()), this, SLOT(FopenImg()));
	connect(openImg, SIGNAL(pressed()), this, SLOT(StyleChange()));

	QMovie*movie = new QMovie(":/QtGui/welcome.gif");
	
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
	this->hide();

}

void QtGui::StyleChange() const
{
	openImg->setStyleSheet("color: #122;background:white;font-size:18px;border:1px solid white;border-radius:4px");
}
