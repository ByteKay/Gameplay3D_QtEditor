#pragma once

#include "ConstantDefine.h"
#include "ui_main.h"

class KayMainWindow : QMainWindow
{
	Q_OBJECT
public:
	~KayMainWindow();
	static KayMainWindow* MainWindow();
private:
	KayMainWindow(QWidget *parent = 0);
private:
	Ui::MainWindow *mMainWindow;
};
