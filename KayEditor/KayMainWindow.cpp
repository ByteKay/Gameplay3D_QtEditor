#include "KayMainWindow.h"
#include "ConstantDefine.h"
#include "ui_main.h"

KayMainWindow::KayMainWindow(QWidget *parent) : QMainWindow(parent), mMainWindow(new Ui::MainWindow)
{
	mMainWindow->setupUi(this);
	show();
}

KayMainWindow::~KayMainWindow()
{
	if (mMainWindow)
	{
		delete mMainWindow;
		mMainWindow = 0;
	}
}

KayMainWindow* KayMainWindow::MainWindow()
{
	static KayMainWindow mainWindow;
	return &mainWindow;
}
