#pragma once;

class KWindow;
class SxEditor;
class SxCommonTreeModel;
class SxDesktopDockWindow;

class SxExplorer : public QObject, public ItemCall
{
	Q_OBJECT
public:
	explicit SxExplorer(QObject *parent = NULL);
	~SxExplorer();
public:
	static SxExplorer *instance();
	void openFile(const QString& file, const QString& name);

	KWindow *explorerWindow();
private:
	void init();
	void setEdit(qint64 nodeid);
	NextOP walkAt(qint64 nodeid, int nchild);

private slots:
	void on_editor_close();
	void on_tree_clicked(Qt::MouseButton button);
	void on_tree_itemClicked(qint64 nodeid, Qt::MouseButton button);
	void on_tree_itemDbClicked(qint64 nodeid, Qt::MouseButton button);
	void on_tree_menu_action_triggered(bool triggered);
	void on_tree_itemEditRequest(qint64 nodeid);
	void on_tree_lineEdit_editingFinished();
	void on_tree_itemExpanded(qint64 nodeid);
	void on_tree_itemCollapsed(qint64 nodeid);
	void setItemExpanded(qint64 nodeid);
	void on_explorer_aboutToClose();
	void on_trayTimer_timeout();

	void on_system_tray_activated(QSystemTrayIcon::ActivationReason reason);
	void on_system_tray_menu_action_triggered(bool triggered);
private:
	QPointer<SxDesktopDockWindow> m_pWindow;
	QPointer<KTreeWidget> m_pTreeWidget;
	QPointer<SxCommonTreeModel> m_pTreeModel;

	QList<SxEditor*> m_lstEditor;

	QPointer<KLineEdit> m_pLineEdit;

	QString m_modelFile;
	QString m_cfgFile;

	QPointer<QSystemTrayIcon> m_pSysTrayIcon;

	QTimer m_trayTimer;
};