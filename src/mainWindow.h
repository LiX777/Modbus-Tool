#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QThread>
#include <QTimer>
#include "serialManager.h"
#include "common.h"
#include "tableManager.h"
#include "modBusService.hpp"
namespace Ui {
	class MainWindow;
}

#define RMUP500_REG_START_ADDR		0x2F	//设备的固定地址
#define RMUP500_REG_STARTREAD_ADDR	0x00	//设备的只读地址
#define RMUP500_REG_COUNT			32		//可编程寄存器数量
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(SerialManager *serialManager, MapTableManager *tableManager, QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void onConnectClicked(); //连接
	void onDisconnectClicked();//断开连接
	void onSendClicked();//下方发送
	void onDataReceived(const QByteArray &data);//数据接收
	
	void onErrorOccurred(const QString &error);//错误处理
	void onLogClearClicked();//log窗口清除
	void onTableClearClicked();//表格数据清空
	void onTableSendClicked();//表格数据发送 1 检测那个单元格数据变动了  2 全部发送
	void onTableReadClicked();//读取寄存器数据到表格中

	void onRefreshPortsClicked();//刷新端口
	//void onPortsRefreshed(const QVector<QString> &portList); // 端口列表更新
	void onPortsRefreshed(const QList<QSerialPortInfo> &portList); // 端口列表更新

	void onConnectionResult(bool success, const QString &message); // 连接结果

	void onTimerout();

signals:
	void refreshPortsRequested();
	void openPortRequested(const QString name, qint32 rate);
	void closePortRequested();
	void writeDataRequested(const QByteArray &data);
	
private:
	void setupUI();
	void setupConnections();
	 
	void appendToLog(const QString &message, bool isSent = false);
	void initTabeWidget();

	void ConnectionStatusChanged(bool connected);//端口连接状态 按钮变化

	void setupSerialConnection(); // 设置串口连接
	
	void readRs485Request(uint8_t slaveAddr);//读操作
	

	Ui::MainWindow *ui;
	SerialManager *m_serialManager;
	MapTableManager *m_tableManager;
	QThread *m_workerThread;
	QTimer* m_rs485Timer;
	uint16_t m_readAddr = 0x2f;//读起始地址
	std::unique_ptr<ModbusFrame> m_modBusFramePtr;//即时存放的解析帧指针
	bool m_isConnected;//是否连接标志
	bool m_isModbusSend;//上一次是否是modbus占用的发送 是：需要解析帧 否：直接打印

};

#endif // MAINWINDOW_H