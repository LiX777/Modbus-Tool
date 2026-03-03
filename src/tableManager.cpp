#include "tableManager.h"
#include <QDebug>
#include "common.h"

MapTableManager::MapTableManager(QTableWidget *tableWidget, QObject *parent)
	: QObject(parent), m_tableWidget(tableWidget)
{
	if (m_tableWidget) {
		initManager();
		connectTableSignals();
	}
}

void MapTableManager::setTableWidget(QTableWidget *tableWidget)
{
	if (m_tableWidget) {
		disconnectTableSignals();
	}

	m_tableWidget = tableWidget;

	if (m_tableWidget) {
		// 清空Map
		m_itemsMap.clear();
		m_originalTexts.clear();
		initManager();
		connectTableSignals();
	}
}
void MapTableManager::connectTableSignals()
{
	connect(m_tableWidget, &QTableWidget::itemChanged, this, &MapTableManager::onItemChanged);
}

void MapTableManager::disconnectTableSignals()
{
	disconnect(m_tableWidget, &QTableWidget::itemChanged, this, &MapTableManager::onItemChanged);
}
void MapTableManager::saveOriginalTexts()
{
	m_originalTexts.clear();
	for (auto it = m_itemsMap.constBegin(); it != m_itemsMap.constEnd(); ++it) {
		if (it.value().item) {
			QPair<int, int> key = it.key();
			m_originalTexts[key] = it.value().item->text();
		}
	}
}

void MapTableManager::onItemChanged(QTableWidgetItem *item)
{
	if (!item) 
		return;

	int row = item->row();
	int col = item->column();
	QPair<int, int> key(row, col);

	// 检查这个item是否由我们管理
	if (!m_itemsMap.contains(key))
		return;

	QString newText = item->text();
	QString oldText = m_originalTexts.value(key, "");

	// 更新原始文本
	m_originalTexts[key] = newText;


	// 可以在这里添加自定义处理逻辑
	handleItemTextChange(row, col, oldText, newText);
}
void MapTableManager::handleItemTextChange(int row, int col, const QString &oldText, const QString &newText)
{
	if (newText.isEmpty())//不管是否发送过都 需要清空flag  因为数据为空了
	{
		setItemUpdate(row, col, false);
	}
	else// 数据非空
	{
		if (m_isSend)//刚刚发送过 则清空之前所有的flag
		{
			for(auto& item : m_itemsMap)
			{
				item.b_update = false;
			}
			m_isSend = false;
		}
		setItemUpdate(row, col, true);
	}
		

}
bool MapTableManager::initManager()
{
	if (!m_tableWidget) {
		qWarning() << "Table widget is not set!";
		DbgPrint(2, "Table widget is not set!!");
		return false;
	}
	//获取行数 列数
	int rowCount = m_tableWidget->rowCount();
	int colCount = m_tableWidget->columnCount();
	QTableWidgetItem *item = nullptr;
	for (int col = 0; col < colCount; col++)
	{
		for (int row = 0; row < rowCount; row++)
		{
			// 添加到Map - 使用QPair<int, int>作为key
			QPair<int, int> key(row, col);
			item = m_tableWidget->item(row, col);
			if (item != nullptr)
			{
				TableItemData itemData(row, col, 0x00, false, false, item);
				m_itemsMap[key] = itemData;
				m_originalTexts[key] = item->text();
			}	
		}
	}
	return true;
}
bool MapTableManager::addItem(int row, int col, const QString &text, unsigned short addr, bool editable)
{
	if (!m_tableWidget) {
		qWarning() << "Table widget is not set!";
		DbgPrint(2, "Table widget is not set!!");
		return false;
	}

	// 确保表格有足够的行和列
	if (row >= m_tableWidget->rowCount()) {
		m_tableWidget->setRowCount(row + 1);
	}
	if (col >= m_tableWidget->columnCount()) {
		m_tableWidget->setColumnCount(col + 1);
	}

	// 创建QTableWidgetItem
	QTableWidgetItem *tableItem = new QTableWidgetItem(text);
	QSignalBlocker blocker(m_tableWidget);
	// 设置可编辑属性
	if (editable) {
		tableItem->setFlags(tableItem->flags() | Qt::ItemIsEditable);
		
	}
	else
	{
		tableItem->setFlags(tableItem->flags() & ~Qt::ItemIsEditable);
	}

	// 添加到Map - 使用QPair<int, int>作为key
	QPair<int, int> key(row, col);
	TableItemData itemData(row, col, addr, editable, false, tableItem);
	m_itemsMap[key] = itemData;
	m_originalTexts[key] = text;
	// 添加到表格
	
	m_tableWidget->setItem(row, col, tableItem);

	return true;
}

TableItemData* MapTableManager::getItem(int row, int col)
{
	QPair<int, int> key(row, col);
	if (m_itemsMap.contains(key)) {
		return &m_itemsMap[key];
	}
	return nullptr;
}

QTableWidgetItem* MapTableManager::getTableItem(int row, int col)
{
	TableItemData *itemData = getItem(row, col);
	return itemData ? itemData->item : nullptr;
}

bool MapTableManager::removeItem(int row, int col)
{
	QPair<int, int> key(row, col);
	if (m_itemsMap.contains(key)) {
		// 从表格中移除
		if (m_tableWidget) {
			delete m_tableWidget->takeItem(row, col);
		}
		// 从Map中移除
		m_itemsMap.remove(key);
		if(m_originalTexts.contains(key))
			m_originalTexts.remove(key);
		return true;
	}
	return false;
}

void MapTableManager::clearAll()
{
	// 清空Map
	m_itemsMap.clear();
	m_originalTexts.clear();

	// 清空表格
	if (m_tableWidget) {
		m_tableWidget->clear();
		m_tableWidget->setRowCount(0);
		m_tableWidget->setColumnCount(0);
	}
}
bool MapTableManager::setItemText(int row, int col, QString &text)
{
	QPair<int, int> key(row, col);
	if (m_itemsMap.contains(key)) {
		if (m_tableWidget) 
		{
			m_originalTexts[key] = text;
			//m_itemsMap[key].b_update = false;
			if (text.isEmpty())
			{
				m_itemsMap[key].b_update = false;
			}
			QSignalBlocker blocker(m_tableWidget);
			m_tableWidget->item(row, col)->setText(text);
		
		}
		return true;
	}
	return false;

}
void MapTableManager::setItemEditable(int row, int col, bool editable)
{
	TableItemData *itemData = getItem(row, col);
	if (itemData && itemData->item) {
		QSignalBlocker blocker(m_tableWidget);
		itemData->b_editable = editable;
		if (editable) {
			itemData->item->setFlags(itemData->item->flags() | Qt::ItemIsEditable);
		}
		else {
			itemData->item->setFlags(itemData->item->flags() & ~Qt::ItemIsEditable);
		}
	}
}

void MapTableManager::setItemUpdate(int row, int col, bool update)
{
	TableItemData *itemData = getItem(row, col);
	if (itemData && itemData->item) 
	{
		itemData->b_update = update;
	}
}
void MapTableManager::setItemAddress(int row, int col, unsigned short addr)
{
	TableItemData *itemData = getItem(row, col);
	if (itemData && itemData->item) {
		itemData->registerAddr = addr;
	}
}
void MapTableManager::setRs485SendFlag(bool b)
{
	m_isSend = b;
}
const QMap<QPair<int, int>, TableItemData>& MapTableManager::getAllItems() const
{
	return m_itemsMap;
}

QList<QPair<int, int>> MapTableManager::findItemsByText(const QString &text)
{
	QList<QPair<int, int>> results;

	for (auto it = m_itemsMap.constBegin(); it != m_itemsMap.constEnd(); ++it) {
		if (it.value().item && it.value().item->text() == text) {
			results.append(it.key());
		}
	}

	return results;
}

QList<QPair<int, int>> MapTableManager::findItemsByEditable(bool editable)
{
	QList<QPair<int, int>> results;

	for (auto it = m_itemsMap.constBegin(); it != m_itemsMap.constEnd(); ++it) {
		if (it.value().b_editable == editable) {
			results.append(it.key());
		}
	}

	return results;
}

QList<QPair<int, int>> MapTableManager::findItemsByUpdate(bool update)
{
	QList<QPair<int, int>> results;

	for (auto it = m_itemsMap.constBegin(); it != m_itemsMap.constEnd(); ++it) {
		if (it.value().b_update == update) {
			results.append(it.key());
		}
	}
	return results;
}

QPair<int, int> MapTableManager::findItemsByAddr(unsigned short addr)
{
	for (auto it = m_itemsMap.constBegin(); it != m_itemsMap.constEnd(); ++it) {
		if (it.value().registerAddr == addr) {
			return it.key();
		}
	}
	
}

int MapTableManager::itemCount() const
{
	return m_itemsMap.size();
}

bool MapTableManager::contains(int row, int col) const
{
	return m_itemsMap.contains(QPair<int, int>(row, col));
}

QList<QPair<int, int>> MapTableManager::getAllPositions() const
{
	return m_itemsMap.keys();
}