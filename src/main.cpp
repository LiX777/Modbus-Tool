#include <QApplication>
#include <QMainWindow>
#include "mainWindow.h"
#include "serialManager.h"
#include "tableManager.h"

int main(int argc, char *argv[])
{
	// 适应分辨率
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	// 初始化Qt应用程序
	QApplication app(argc, argv);
	// 设置应用程序信息
	QApplication::setApplicationName("RS485 Serial Tool");
	QApplication::setApplicationVersion("1.0.0");
	QApplication::setOrganizationName("Scott");

	// 创建核心管理器
	SerialManager serialManager;
	MapTableManager tableManager;
	// 创建主窗口并注入依赖
	MainWindow mainWindow(&serialManager, &tableManager);
	mainWindow.show();

	// 进入事件循环
	return app.exec();
}