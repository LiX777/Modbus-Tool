#ifndef MAPTABLEMANAGER_H
#define MAPTABLEMANAGER_H

#include <QTableWidget>
#include <QMap>
#include <QPair>
#include <QObject>

// 简化的表格项结构体
typedef struct TableItemData
{
	int row;						// 行坐标
	int col;						// 列坐标
	unsigned short registerAddr;	//对应的寄存器地址
	bool b_editable;				// 是否可编辑
	bool b_update;					//是否更新过
	QTableWidgetItem *item;			// QTableWidgetItem对象

	// 构造函数
	TableItemData(int r = -1, int c = -1, unsigned short addr = 0x002f, bool edit = false, bool up = false, QTableWidgetItem *it = nullptr)
		: row(r), col(c), registerAddr(addr), b_editable(edit), b_update(up), item(it) {}
}TableItemData;

// 使用Map的表格管理器
class MapTableManager : public QObject
{
	Q_OBJECT
public:
	MapTableManager(QTableWidget *tableWidget = nullptr, QObject * parent = nullptr);

	// 设置关联的表格控件
	void setTableWidget(QTableWidget *tableWidget);
	// 初始化管理器
	bool initManager();
	// 添加项目到表格和Map
	bool addItem(int row, int col, const QString &text, unsigned short addr, bool editable = false);

	// 获取项目 - 通过坐标快速查找
	TableItemData* getItem(int row, int col);
	QTableWidgetItem* getTableItem(int row, int col);
	// 根据地址查找项目regeisteraddr - 返回坐标列表
	QPair<int, int> findItemsByAddr(unsigned short addr);
	// 移除项目
	bool removeItem(int row, int col);
	// 清空所有项目
	void clearAll();

	// 添加发送标志位
	void setRs485SendFlag(bool b);
	// 设置项目文字
	bool setItemText(int row, int col, QString &text);
	// 设置项目可编辑属性
	void setItemEditable(int row, int col, bool editable);
	// 设置项目更新属性
	void setItemUpdate(int row, int col, bool update);
	// 设置项目地址
	void setItemAddress(int row, int col, unsigned short addr);
	// 获取所有项目
	const QMap<QPair<int, int>, TableItemData>& getAllItems() const;

	// 根据文本查找项目 - 返回坐标列表
	QList<QPair<int, int>> findItemsByText(const QString &text);

	// 根据可编辑属性查找项目 - 返回坐标列表
	QList<QPair<int, int>> findItemsByEditable(bool editable);

	// 根据内容更新属性查找项目b_update - 返回坐标列表
	QList<QPair<int, int>> findItemsByUpdate(bool update);
	

	// 获取项目数量
	int itemCount() const;

	// 检查是否存在某个位置的项目
	bool contains(int row, int col) const;

	// 获取所有坐标列表
	QList<QPair<int, int>> getAllPositions() const;



private slots:
	void onItemChanged(QTableWidgetItem *item);

private:
	void connectTableSignals();
	void disconnectTableSignals();
	void saveOriginalTexts();
	void handleItemTextChange(int row, int col, const QString &oldText, const QString &newText);

private:
	bool m_isSend = false;
	QTableWidget *m_tableWidget;
	QMap<QPair<int, int>, TableItemData> m_itemsMap; // 使用Map存储，key为(行,列)
	QMap<QPair<int, int>, QString> m_originalTexts; // 存储原始文本用于比较
};

#endif // MAPTABLEMANAGER_H