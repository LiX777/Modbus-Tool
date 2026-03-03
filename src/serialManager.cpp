#include "serialManager.h"
#include <QDebug>
#include <QThread>
SerialManager::SerialManager(QObject *parent)
	: QObject(parent)
	, m_serialPort(nullptr)
	, m_isConnected(false)
{
	m_serialPort = new QSerialPort(this);

	connect(m_serialPort, &QSerialPort::readyRead,this, &SerialManager::handleReadyRead);
	connect(m_serialPort, &QSerialPort::errorOccurred,this, &SerialManager::handleError);
}

SerialManager::~SerialManager()
{
	if (m_serialPort->isOpen()) {
		m_serialPort->close();
	}
}


QList<QSerialPortInfo> SerialManager::getAvailablePorts()
{
	// 这个方法在SerialWorker线程中执行
	qDebug() << "Getting available ports in thread:" << QThread::currentThreadId();
	return QSerialPortInfo::availablePorts();
}
void SerialManager::refreshPorts()
{
	QList<QSerialPortInfo> portLists = getAvailablePorts();
	QVector<QString> portListVec;
	for (const auto &port : portLists) {
		QString portStr = QString("%1  %2  %3")
			.arg(port.portName())
			.arg(port.description())
			.arg(port.manufacturer());
		portListVec.append(portStr);
	}
	//emit portsRefreshed(portListVec);
	emit portsRefreshed(portLists);
}


void SerialManager::openSerialPort(const QString &portName, qint32 baudRate)
{
	// 先检查端口是否存在（现在在同一个线程中）
	QList<QSerialPortInfo> ports = getAvailablePorts();
	bool portExists = false;

	foreach(const QSerialPortInfo &info, ports) {
		if (info.portName() == portName) {
			portExists = true;
			break;
		}
	}

	if (!portExists) {
		emit connectionResult(false, QString("%1 is not exist").arg(portName));
		return;
	}
	if (m_isConnected) {
		closeSerialPort();
	}

	m_serialPort->setPortName(portName);
	m_serialPort->setBaudRate(baudRate);
	m_serialPort->setDataBits(QSerialPort::Data8);
	m_serialPort->setParity(QSerialPort::NoParity);
	m_serialPort->setStopBits(QSerialPort::OneStop);
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

	if (m_serialPort->open(QIODevice::ReadWrite)) {
		m_isConnected = true;
		m_serialPort->clear(QSerialPort::AllDirections);
		m_serialPort->readAll(); // 丢弃可能存在的旧数据
		emit connectionResult(true, QString("%1 : connected success at %2 baud").arg(portName).arg(baudRate));
		//emit connectionStatusChanged(true);
		return;
	}
	else {
		emit connectionResult(false, QString(" %1 : connected failure").arg(portName));
		emit mySerialErrorOccurred(m_serialPort->errorString());
		return;
	}
}

void SerialManager::closeSerialPort()
{
	if (m_serialPort->isOpen()) {
		m_serialPort->close();
	}
	m_isConnected = false;
	emit connectionResult(false, QString("Disconnected"));
}

bool SerialManager::isConnected() const
{
	return m_isConnected;
}

void SerialManager::sendData(const QByteArray &data)
{
	if (m_isConnected && m_serialPort->isOpen()) {
	
		qint64 bytesWritten = m_serialPort->write(data);
		if (bytesWritten == -1)
		{
			qCritical() << "Write error:" << m_serialPort->errorString();
			return;
		}
		// 等待发送完成并清空发送缓冲区
		if (!m_serialPort->waitForBytesWritten(1000))
		{
			//m_serialPort->clear(QSerialPort::Output);
			// 详细的超时诊断
			qCritical() << "Timeout! Possible reasons:";
			qCritical() << "1. Error:" << m_serialPort->errorString();
			qCritical() << "2. Bytes in buffer:" << m_serialPort->bytesToWrite();
			qCritical() << "3. Is open:" << m_serialPort->isOpen();
			qCritical() << "4. Is writable:" << m_serialPort->isWritable();

			// 检查特定错误
			if (m_serialPort->error() == QSerialPort::TimeoutError) {
				qCritical() << "Timeout error - check cable and device";
			}
			else if (m_serialPort->error() == QSerialPort::WriteError) {
				qCritical() << "Write error - check permissions";
			}
		}
		
	}
}

QByteArray SerialManager::receiveData()
{
	if (m_isConnected && m_serialPort->isOpen()) {
		return m_serialPort->readAll();
	}
	return QByteArray();
}



void SerialManager::handleReadyRead()
{
	QByteArray data = m_serialPort->readAll();
	//m_serialPort->clear(QSerialPort::AllDirections);
	//m_serialPort->readAll(); // 丢弃可能存在的旧数据
	if (!data.isEmpty()) { 
		emit dataReceived(data);
	}
}

void SerialManager::handleError(QSerialPort::SerialPortError error)
{
	if (error != QSerialPort::NoError) {
		switch (error) {
		case QSerialPort::WriteError:
			qCritical() << "Write error - clearing output buffer";
			m_serialPort->clear(QSerialPort::Output);
			break;
		case QSerialPort::TimeoutError:
			qCritical() << "Timeout error - check connection";
			break;
		case QSerialPort::ResourceError:
			qCritical() << "Resource error - device disconnected?";
			break;
		default:
			qCritical() << "Other serial error";
			break;
		}
		emit mySerialErrorOccurred(m_serialPort->errorString());
		closeSerialPort();
	}
}