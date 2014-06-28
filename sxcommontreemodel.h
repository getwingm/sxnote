#pragma once;

#define LINEEDIT_ID		(1000)
#define TREEITEM_ID		(1001)
/*��������Ŀ¼���ļ������ԣ����нڵ㶼���ļ����ɱ༭*/
class SxTreeItem : public KTreeItem
{
	Q_OBJECT
public:
	explicit SxTreeItem(KTreeWidget *view);
	~SxTreeItem();
	
protected:
	bool setAction(int id, const QVariant& val, QVariant& result);
};

class SxCommonTreeModel : public KCommonTreeModel
{
	Q_OBJECT
public:
	enum ItemCustomRole
	{
		ItemDateCreate = Qt::UserRole + 100,
		ItemExpanded,
		ItemId
	};
public:
	explicit SxCommonTreeModel(QObject *parent=NULL);
	~SxCommonTreeModel();

	void setTreeWidget(KTreeWidget *pTreeWidget);

	void modelFromFile(const QString& file);
	void modelToFile(const QString& file);
	void modelFromMemory(const QString& data, Node *root);
	QString modelToMemory(Node *pNode, int indent);
	bool setData(qint64 nodeid, const QMap<Qt::ItemDataRole, QVariant> &val);
	bool setAction(int id, const QVariant& val, QVariant& result);
private:
	QPointer<KTreeWidget> m_pTreeWidget;
};