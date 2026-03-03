#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QVector>
Q_DECLARE_METATYPE(QSerialPortInfo)

class SerialManager : public QObject
{
	Q_OBJECT

public:
	explicit SerialManager(QObject *parent = nullptr);
	~SerialManager();

	
	bool isConnected() const;
	// 数据收发
	QByteArray receiveData();

	// 串口列表
	QList<QSerialPortInfo> getAvailablePorts(); // 获取可用端口
	

signals:
	//void portsRefreshed(const QVector<QString> &portList); // 端口列表更新
	void portsRefreshed(const QList<QSerialPortInfo> &portList); // 端口列表更新
	void dataReceived(const QByteArray &data);
	void mySerialErrorOccurred(const QString &error);
	void connectionResult(bool success, const QString &message); // 连接结果信号
	void portClosed(); // 端口关闭信号
private slots:
	void handleReadyRead();
	// 串口操作
	void openSerialPort(const QString &portName, qint32 baudRate);
	void sendData(const QByteArray &data);
	void closeSerialPort();
	void refreshPorts(); // 刷新可用端口列表
	void handleError(QSerialPort::SerialPortError error);
private:
	QSerialPort *m_serialPort;
	bool m_isConnected;

};

#endif // SERIALMANAGER_H