#include "KayMainWindow.h"
#include "ConstantDefine.h"
#include "KayGame.h"

int main(int args, char** argv)
{
	QApplication app(args, argv);
	KayMainWindow::MainWindow();
	return app.exec();
}