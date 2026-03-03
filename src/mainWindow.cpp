#include "mainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <iostream>

MainWindow::MainWindow(SerialManager *serialManager, MapTableManager *tableManager, QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_serialManager(serialManager)
	, m_tableManager(tableManager)
	, m_isConnected(false)
{
	ui->setupUi(this);
	// 初始化串口工作线程
	m_rs485Timer = new QTimer(this);
	setupSerialConnection();
	m_tableManager->setTableWidget(ui->tableWidget);
	initTabeWidget();
	setupConnections();
	setupUI();

	//onRefreshPortsClicked();
	setWindowTitle("RS485 Serial Tool v1.0.0");
	m_rs485Timer->start(2000);
}

MainWindow::~MainWindow()
{ // 清理资源
	if (m_workerThread && m_workerThread->isRunning()) {
		m_workerThread->quit();
		m_workerThread->wait();
	}
	delete ui;
}

void MainWindow::setupSerialConnection()
{
	// 创建工作线程和工作者对象
	m_workerThread = new QThread(this);
	//m_serialManager = new SerialManager;
	// 将工作者对象移动到工作线程
	m_serialManager->moveToThread(m_workerThread);

		// 在使用前注册
	qRegisterMetaType<QSerialPortInfo>("QSerialPortInfo");
	// 连接信号槽
	// 从工作者到主窗口的信号
	connect(m_serialManager, &SerialManager::portsRefreshed, this, &MainWindow::onPortsRefreshed, Qt::QueuedConnection);
	//connect(m_serialManager, SIGNAL(portsRefreshed(QList<QSerialPortInfo>)), this, SLOT(onPortsRefreshed(QList<QSerialPortInfo>)), Qt::QueuedConnection);
	connect(m_serialManager, &SerialManager::connectionResult, this, &MainWindow::onConnectionResult, Qt::QueuedConnection);
	connect(m_serialManager, &SerialManager::dataReceived, this, &MainWindow::onDataReceived, Qt::QueuedConnection);
	connect(m_serialManager, &SerialManager::mySerialErrorOccurred, this, &MainWindow::onErrorOccurred, Qt::QueuedConnection);
	
	
	// 从主窗口到工作者的信号
	//connect(this, &MainWindow::refreshPortsRequested, m_serialManager, &SerialManager::refreshPorts, Qt::QueuedConnection);
	connect(this, SIGNAL(refreshPortsRequested()), m_serialManager, SLOT(refreshPorts()), Qt::QueuedConnection);
	connect(this, SIGNAL(openPortRequested(QString, qint32)), m_serialManager, SLOT(openSerialPort(QString, qint32)), Qt::QueuedConnection);
	connect(this, SIGNAL(closePortRequested()), m_serialManager, SLOT(closeSerialPort()), Qt::QueuedConnection);
	connect(this, SIGNAL(writeDataRequested(QByteArray)), m_serialManager, SLOT(sendData(QByteArray)), Qt::QueuedConnection);

	// 线程结束时自动删除工作者对象
	//connect(m_workerThread, &QThread::finished, m_serialManager, &QObject::deleteLater);
	//connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
	// 启动工作线程
	m_workerThread->start();
	// 触发子线程操作
	//QMetaObject::invokeMethod(m_serialManager, "refreshPorts", Qt::QueuedConnection);
}


void MainWindow::setupUI()
{
	this->resize(1000, 800);
	// 初始化UI组件
	ui->comboBoxBaudRate->addItems({ "9600", "19200", "38400", "57600", "115200" });
	ui->comboBoxBaudRate->setCurrentText("115200");
	ui->comboBoxPort->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	ui->textEditLog->setReadOnly(true);
	ui->pushButtonDisconnect->setEnabled(false);
	

	// 设置表格的尺寸策略为扩展（Expanding）
	ui->tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// 关键设置：列自动拉伸
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	// 关键设置：行自动拉伸
	ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	QString styleSheet =
		// 表格整体
		"QTableWidget {"
		"    background-color: white;"
		"    gridline-color: #e0e6ed;"
		"    border: 1px solid #e0e6ed;"
		"    border-radius: 6px;"
		"    outline: 0;"
		"}"

		// 水平表头
		"QTableWidget QHeaderView::section:horizontal {"
		"    background-color: #34495e;"
		"    color: white;"
		"    padding: 8px;"
		"    border: 1px solid #2c3e50;"
		"    font-weight: bold;"
		"    font-size: 14px;"
		"}"
		// 水平表头悬停效果
		"QTableWidget QHeaderView::section:horizontal:hover {"
		"    background-color: #607d8b;"
		"}"

		// 垂直表头
		"QTableWidget QHeaderView::section:vertical {"
		"    background-color: #ecf0f1;"
		"    color: #34495e;"
		"    padding: 6px;"
		"    border: 1px solid #bdc3c7;"
		"}"
		// 表格项
		"QTableWidget::item {"
		"    padding: 1px;"
		"    border: none;"
		"    border-bottom: 1px solid #f1f3f4;"
		"    color: #212529;"
		"}"

		// 可编辑项
		"QTableWidget::item:editable {"
		"    background-color: white;"
		"}"

		// 不可编辑项
		"QTableWidget::item:!editable {"
		"    background-color: #f8f9fa;"
		"    color: #6c757d;"
		"}"

		// 选中项
		"QTableWidget::item:selected {"
		"    background-color: #e7f3ff;"
		"    color: #0056b3;"
		"    border-bottom: 1px solid #c2e7ff;"
		"}"
		// 交替行颜色
		"QTableWidget::item:nth-child(even) {"
		"    background-color: #fafbfc;"
		"}"

		// 角部按钮（左上角）
		"QTableCornerButton::section {"
		"    background-color: #f8f9fa;"
		"    border: none;"
		"    border-bottom: 2px solid #e9ecef;"
		"    border-right: 1px solid #e9ecef;"
		"}";

	ui->tableWidget->setStyleSheet(styleSheet);
	// 查找所有QPushButton并设置样式表
	QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
	
	for (QPushButton* button : buttons) {
		button->setStyleSheet(
			"QPushButton {"
			"   background-color: #546e7a;"
			"   border: none;"
			"   color: white;"
			"   padding: 9px 10px;"
			"   text-align: center;"
			"   font-size: 16px;"
			"	font-weight: bold;"
			"   border-radius: 10px;"
			"}"
			"QPushButton:hover {"
			"   background-color: #455a64;"
			"}"
			"QPushButton:pressed {"
			"   background-color: #263238;"
			"}"
			"QPushButton:disabled {"
			"   background-color: #cfd8dc;"
			"   color: #bdc3c7;"
			"   border: 1px dashed #bdc3c7;"
			"}"
			
		);
	}

}

void MainWindow::setupConnections()
{
	// 连接按钮信号
	connect(ui->pushButtonConnect, &QPushButton::clicked,
		this, &MainWindow::onConnectClicked);
	connect(ui->pushButtonDisconnect, &QPushButton::clicked,
		this, &MainWindow::onDisconnectClicked);
	connect(ui->pushButtonSend, &QPushButton::clicked,
		this, &MainWindow::onSendClicked);
	connect(ui->pushButtonRefresh, &QPushButton::clicked,
		this, &MainWindow::onRefreshPortsClicked);

	connect(ui->pushButtonClear, &QPushButton::clicked,
		this, &MainWindow::onLogClearClicked);
	connect(ui->pushButtonClear_2, &QPushButton::clicked,
		this, &MainWindow::onTableClearClicked);
	connect(ui->pushButtonSend_2, &QPushButton::clicked,
		this, &MainWindow::onTableSendClicked);
	connect(ui->pushButtonRead, &QPushButton::clicked,
		this, &MainWindow::onTableReadClicked);

	//连接定时器信号
	connect(m_rs485Timer, &QTimer::timeout, this, &MainWindow::onTimerout);
}


void MainWindow::onConnectionResult(bool success, const QString &message)
{
	appendToLog(message);
	if (success)
	{
		m_isConnected = true;
	}
	else
	{
		m_isConnected = false;
	}
	ConnectionStatusChanged(m_isConnected);
}
void MainWindow::onConnectClicked()
{
	QString portName = ui->comboBoxPort->currentText();
	qint32 baudRate = ui->comboBoxBaudRate->currentText().toInt();

	if (portName.isEmpty()) {
		QMessageBox::warning(this, "Warning", "Please select a serial port");
		return;
	}
	
	appendToLog("Connecting...");
	emit openPortRequested(portName.split(" ").first(), baudRate);
	//appendToLog("Connected to " + portName + " at " + QString::number(baudRate) + " baud");	
}

void MainWindow::onDisconnectClicked()
{
	emit closePortRequested();
	//m_serialManager->closeSerialPort();
	//appendToLog("Disconnected");
}

void MainWindow::onSendClicked()
{
	if (!m_isConnected)
	{
		appendToLog("The port is not connected!");
		return;
	}

	QString text = ui->lineEditSend->text();
	if (!text.isEmpty()) 
	{
		QByteArray data = text.toUtf8();
	//	m_serialManager->sendData(data);
		emit writeDataRequested(data);
		m_isModbusSend = false;//非modbus 置零
		appendToLog("Sent: " + text, true);
		//ui->lineEditSend->clear();
	}
}

//void MainWindow::onPortsRefreshed(const QVector<QString> &portList)
void MainWindow::onPortsRefreshed(const  QList<QSerialPortInfo> &portList)
{
	if (portList.isEmpty())
		return;
	ui->comboBoxPort->clear();
	for (const auto &port : portList) {
		
		//ui->comboBoxPort->addItem(port);
		ui->comboBoxPort->addItem(port.portName());
	}
	appendToLog("Port list refreshed");
}
void MainWindow::onRefreshPortsClicked()
{
	emit refreshPortsRequested();
}

void MainWindow::onDataReceived(const QByteArray &data)
{
	if (m_isModbusSend)
	{//modbus 帧解析
		m_isModbusSend = false;
		try
		{
			std::vector<uint8_t> vecResponse(data.begin(), data.end());
			m_modBusFramePtr = std::move(ModbusFrame::parseResponse(vecResponse));

			if (m_modBusFramePtr && m_modBusFramePtr->isValid())
			{
				QString responseString = QString::fromStdString(m_modBusFramePtr->toString());
				if (m_modBusFramePtr->isException())
				{
					appendToLog(("Exceptional response") + responseString);
				}
				else
				{
					auto stdData = m_modBusFramePtr->getRawData();
					QByteArray qData(reinterpret_cast<const char*>(stdData.data()), stdData.size());
					appendToLog("Recv: " + qData.toHex(' '), false);

					ModbusFunctionCode code = m_modBusFramePtr->getFunctionCode();
					//auto rawData = m_modBusFramePtr->getRawData();
					uint16_t  regCount = stdData[2] / 2;
					switch (code)//只有读命令才有下面的过程 写命令在打印时已经结束了4 
					{
					case ModbusFunctionCode::ReadHoldingRegisters://读显示在表格中
					{
						if (ui->tableWidget && m_tableManager)
						{
							for (int i = 0; i < regCount; i++)
							{
								QPair<int, int> coordinate = m_tableManager->findItemsByAddr(m_readAddr + i);

								uint16_t tmp = stdData[3 + 2 * i];
								tmp <<= 8;
								tmp += stdData[3 + 2 * i + 1];
								//uint16_t* regData = reinterpret_cast<uint16_t*>(&stdData[3 + 2 * i]);
								m_tableManager->setItemText(coordinate.first, coordinate.second, QString::number(tmp));
								//m_tableManager->setItemUpdate(coordinate.first, coordinate.second, false);
								//uint16_t* regData = rawData.data() + (i * 2);
								//m_tableManager->setItemText
							}
						}
						break;
					}
					case ModbusFunctionCode::ReadInputRegisters:
					case ModbusFunctionCode::ReadCoils:
					case ModbusFunctionCode::ReadDiscreteInputs:
					case ModbusFunctionCode::WriteSingleRegister:
					case ModbusFunctionCode::WriteSingleCoil: 
					case ModbusFunctionCode::WriteMultipleRegisters:
					case ModbusFunctionCode::WriteMultipleCoils: 
						break;
					
					default:
						break;
					}
					//auto data = responseFrame->getData();
				}
			}
			else
			{
				appendToLog("Frame format error");
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Receive Error: " << e.what() << std::endl;
		}
		
	}
	else
	{//普通串口
		QString received = QString::fromUtf8(data);
		appendToLog("Recv: " + received);
	}
	
}

void MainWindow::ConnectionStatusChanged(bool connected)
{
	m_isConnected = connected;
	ui->pushButtonConnect->setEnabled(!connected);
	ui->pushButtonDisconnect->setEnabled(connected);
	ui->comboBoxPort->setEnabled(!connected);
	ui->comboBoxBaudRate->setEnabled(!connected);
}

void MainWindow::onErrorOccurred(const QString &error)
{
	appendToLog("Error: " + error);
	QMessageBox::critical(this, "Error", error);
}

void MainWindow::onLogClearClicked()
{
	if (ui->textEditLog)
	{
		ui->textEditLog->clear();
	}
}

void MainWindow::onTableClearClicked()
{
	if (ui->tableWidget && m_tableManager)
	{
		QList<QPair<int, int>> indexList = m_tableManager->findItemsByEditable(true);
		for (const auto& pair : indexList)
		{
			m_tableManager->setItemText(pair.first, pair.second, QString(""));
		}
	}
}
void MainWindow::onTableSendClicked()
{
	if (!m_isConnected)
	{
		appendToLog("The port is not connected!");
		return;
	}
	if (ui->lineEditAddr->text().isEmpty())
	{
		appendToLog("Slave address not filled in!");
		//QMessageBox::information(nullptr, QString("提示"), QString("先填写slave 地址"));
		return;
	}
	uint8_t slaveAddr = ui->lineEditAddr->text().toInt();
	if (ui->tableWidget && m_tableManager)
	{
		QList <QPair<int, int>> indexList = m_tableManager->findItemsByUpdate(true);
		if (indexList.isEmpty())
		{
			appendToLog("The table data has not been updated. Please update at least one piece of data first!");
			//QMessageBox::information(nullptr, QString("提示"), QString("请更新至少一个数据"));
			return;
		}
		else //采用modbus 多数据发送模式有弊端 因为地址必须连续采用单个多次发送
		{
			for (const auto& pair : indexList)
			{
				const TableItemData* itmeData = m_tableManager->getItem(pair.first, pair.second);
				if (itmeData == nullptr)
				{//不该走到这里来
					QMessageBox::critical(this, "Error", "key don't match value");
					return;
				}
				// 表示该项用过了 这样写 不太好 后面看看能不能优化一下
				//itmeData->b_update = false;
				auto text = itmeData->item->text();

				if (!text.isEmpty())
				{
					try
					{
						auto modbusRequest = ModbusFrame::createRequest(
							slaveAddr,
							ModbusFunctionCode::WriteSingleRegister,
							itmeData->registerAddr,
							text.toUShort()
						);
						auto stdData = modbusRequest->getRawData();
						QByteArray qData(reinterpret_cast<const char*>(stdData.data()),stdData.size());
					//	m_serialManager->sendData(qData);
						emit writeDataRequested(qData);
						m_isModbusSend = true;//发送标志设1
						m_tableManager->setRs485SendFlag(true);
						appendToLog("Send: " + qData.toHex(' '), true);
					}
					catch (const std::exception& e)
					{
						std::cerr << "error: " << e.what() << std::endl;
					}
				}
			}
		}
	}
}

void MainWindow::onTableReadClicked()
{
	if (!m_isConnected)
	{
		appendToLog("The port is not connected!");
		return;
	}
	if (ui->lineEditAddr->text().isEmpty())
	{
		appendToLog("Slave address not filled in!");
		//QMessageBox::information(nullptr, QString("提示"), QString("先填写slave 地址"));
		return;
	}
	uint8_t slaveAddr = ui->lineEditAddr->text().toInt();

	try
	{
		auto modbusRequest = ModbusFrame::createRequest(
			slaveAddr,
			ModbusFunctionCode::ReadHoldingRegisters,
			(unsigned short)RMUP500_REG_START_ADDR,
			(unsigned short)RMUP500_REG_COUNT
		);
		m_readAddr = RMUP500_REG_START_ADDR;
		auto stdData = modbusRequest->getRawData();
		QByteArray qData(reinterpret_cast<const char*>(stdData.data()), stdData.size());
		emit writeDataRequested(qData);
		m_isModbusSend = true;
		appendToLog("Send: " + qData.toHex(' '), true);
		
	}
	catch (const std::exception& e)
	{
		std::cerr << "error: " << e.what() << std::endl;
	}
}
void  MainWindow::onTimerout()
{

}
void MainWindow::readRs485Request(uint8_t slaveAddr)
{
	try
	{
		auto modbusRequest = ModbusFrame::createRequest(
			slaveAddr,
			ModbusFunctionCode::ReadHoldingRegisters,
			(unsigned short)RMUP500_REG_START_ADDR,
			(unsigned short)RMUP500_REG_COUNT
		);
		
		m_readAddr = RMUP500_REG_START_ADDR;
		auto stdData = modbusRequest->getRawData();
		QByteArray qData(reinterpret_cast<const char*>(stdData.data()), stdData.size());
		emit writeDataRequested(qData);
		m_isModbusSend = true;
		appendToLog("Send: " + qData.toHex(' '), true);


		Sleep(1);

		auto modbusRequest1 = ModbusFrame::createRequest(
			slaveAddr,
			ModbusFunctionCode::ReadHoldingRegisters,
			(unsigned short)RMUP500_REG_STARTREAD_ADDR,
			(unsigned short)1
		);

		m_readAddr = 0;

		auto stdData1 = modbusRequest1->getRawData();
		QByteArray qData1(reinterpret_cast<const char*>(stdData1.data()), stdData1.size());
		emit writeDataRequested(qData1);
		m_isModbusSend = true;
		appendToLog("Send: " + qData.toHex(' '), true);

	}
	catch (const std::exception& e)
	{
		std::cerr << "error: " << e.what() << std::endl;
	}
}

void MainWindow::appendToLog(const QString &message, bool isSent)
{
	QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
	QString formattedMessage = QString("[%1] %2").arg(timestamp).arg(message);

	// 为发送和接收的消息使用不同的颜色
	if (isSent) {
		ui->textEditLog->setTextColor(Qt::blue);
	}
	else {
		ui->textEditLog->setTextColor(Qt::black);
	}

	ui->textEditLog->append(formattedMessage);

	// 自动滚动到底部
	QScrollBar *scrollbar = ui->textEditLog->verticalScrollBar();
	scrollbar->setValue(scrollbar->maximum());
}
//	初始化表格
void MainWindow::initTabeWidget()
{ 
	if (m_tableManager == nullptr)
		return;
	//获取行数 列数
	int rowCount = ui->tableWidget->rowCount();
	int colCount = ui->tableWidget->columnCount();
	unsigned short regeisterAddr = 40;
	QTableWidgetItem *item = nullptr;
	// 定义哪些列可编辑（索引从0开始）
	QSet<int> editableColumns = { 1, 3, 5, 7}; 
	QSet<int> readOnlyColumns = { 0, 2, 4 ,6}; 
	for (int col = 0; col < colCount; col++)
	{
		for (int row = 0; row < rowCount; row++)
		{
			item = ui->tableWidget->item(row, col);
			if (editableColumns.contains(col))
			{	
				if (item == nullptr)
				{
					m_tableManager->addItem(row, col, QString(""), 0, true);
				}
				//相对应的地址赋值
				m_tableManager->setItemAddress(row, col, regeisterAddr++);
				m_tableManager->setItemEditable(row, col, true);
				//几个特殊单元格
				if ((col == 1 && (row == 5 || row == 6)) || (col == 7 && row == 9))
				{
					m_tableManager->setItemEditable(row, col, false);
					DbgPrint(2, "row : %d  col : %d\n", row, col);
				}
			}
			else if (readOnlyColumns.contains(col))
			{
				if (item == nullptr)
				{
					m_tableManager->addItem(row, col, QString(""), 0, false);
				}
				m_tableManager->setItemEditable(row, col, false);
			}
		}
	}
	

}
