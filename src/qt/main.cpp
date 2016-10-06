#include <windows.h>

#include "QtGui.h"
#include <QtWidgets/QApplication>


int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR    lpCmdLine,
	int       nCmdShow)
{
	QApplication a(nCmdShow, nullptr);
	QtGui w;
	w.show();
	return a.exec();
}
