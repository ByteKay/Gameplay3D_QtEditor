#include "KayMainWindow.h"
#include "ConstantDefine.h"
#include "KayGame.h"

int main(int args, char** argv)
{
	QApplication app(args, argv);
	QApplication::setWindowIcon(QIcon("res/icon.png"));
	KayMainWindow::MainWindow();
	return app.exec();
}