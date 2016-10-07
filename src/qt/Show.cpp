
#include "Show.h"
#include "global.h"



Show::Show(QWidget *parent, QImage x)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->setWindowTitle("Show");
	this->setMaximumSize(1000, 600);
	this->setMinimumSize(1000, 600);
	this->setStyleSheet("background:#111;font-size:12px;");
	this->setWindowFlags(Qt::FramelessWindowHint);	

	back = new QPushButton("<", this);
	back->setStyleSheet("color: #666;background:#111;font-size:64px;border:none");
	back->setGeometry(QRect(10, 160, 96, 96));
	connect(back, SIGNAL(released()), this, SLOT(close()));

	keep = new QPushButton("Keep and Show to your friends!", this);
	keep->setStyleSheet("color: #00d50f;background:#122;font-size:18px;border:1px solid #00d50f;border-radius:4px");
	keep->setGeometry(QRect(220, 500, 540, 60));
	connect(keep, SIGNAL(released()), this, SLOT(FkeepImg()));
	connect(keep, SIGNAL(pressed()), this, SLOT(StyleChange()));

	label = new QLabel(this);
	label->setGeometry(220, 40, 540, 420);
	label->setScaledContents(true);
	label->setStyleSheet("border:4px solid #222;border-radius:2px;");
	label->setPixmap(QPixmap::fromImage(x));
	imagex = x.copy();

}

void Show::FkeepImg()
{
	QString saveSrc = QFileDialog::getSaveFileName(
		this, "Save image file",
		".",
		"Image files (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;All files (*.*)");

	imagex.save(saveSrc);
	exit(0);
}

void Show::StyleChange() const
{
	keep->setStyleSheet("color: #122;background:white;font-size:18px;border:1px solid white;border-radius:4px");
}
