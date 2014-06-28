#include "stdafx.h"
#include "sxexplorer.h"
#include "sxeditor.h"
#include "sxcommontreemodel.h"
#include "sxdesktopdockwindow.h"

Q_GLOBAL_STATIC(SxExplorer, _SxExplorerInstance)

SxExplorer* SxExplorer::instance()
{
	return _SxExplorerInstance();
}


SxExplorer::SxExplorer( QObject *parent /*= NULL*/ )
: QObject(parent)
{
	m_pSysTrayIcon = new QSystemTrayIcon(this);
	m_pSysTrayIcon->setIcon(QIcon("res:/Resources/image/tree_note.png"));
	m_pSysTrayIcon->setToolTip("善学记事本");
	m_pSysTrayIcon->show();
	QObject::connect(m_pSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_system_tray_activated(QSystemTrayIcon::ActivationReason)));
	QObject::connect(&m_trayTimer, SIGNAL(timeout()), this, SLOT(on_trayTimer_timeout()));
	m_trayTimer.start(1000*10);
	m_pWindow = qobject_cast<SxDesktopDockWindow*>(KXmlUI::windowFromFile("res:/Resources/layout/sxexplorer.xml"));
	m_pTreeWidget = m_pWindow->findChild<KTreeWidget*>("filetree");
	KLineEdit *lineEdit = new KLineEdit(m_pTreeWidget);
	lineEdit->setGeometry(0,0,1,1);
	lineEdit->setZValue(1000);
	lineEdit->setFrame(true);
	lineEdit->hide();
	m_pTreeWidget->setData(LINEEDIT_ID, QVariant::fromValue<QObject*>(lineEdit));
	QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(on_tree_lineEdit_editingFinished()));
	m_pLineEdit = lineEdit;
	m_pTreeModel = new SxCommonTreeModel(this);
	KAbstractTreeModel *model = m_pTreeWidget->setModel(m_pTreeModel);
	m_pTreeWidget->setItemCreator(new KTreeItemCreatorT<SxTreeItem>());
	m_pTreeModel->setTreeWidget(m_pTreeWidget);
	m_pTreeWidget->setDoubleClickedAsCollapsedOrExpanded(false);

	KX_SAFE_DELETE(model);
	QObject::connect(m_pTreeWidget, SIGNAL(clicked(Qt::MouseButton)), SLOT(on_tree_clicked(Qt::MouseButton)));
	QObject::connect(m_pTreeWidget, SIGNAL(itemClicked(qint64,Qt::MouseButton)), SLOT(on_tree_itemClicked(qint64,Qt::MouseButton)));
	QObject::connect(m_pTreeWidget, SIGNAL(itemDbClicked(qint64,Qt::MouseButton)), SLOT(on_tree_itemDbClicked(qint64,Qt::MouseButton)));
	QObject::connect(m_pTreeWidget, SIGNAL(itemExpanded(qint64)), this, SLOT(on_tree_itemExpanded(qint64)));
	QObject::connect(m_pTreeWidget, SIGNAL(itemCollapsed(qint64)), this, SLOT(on_tree_itemCollapsed(qint64)));
	QObject::connect(m_pWindow, SIGNAL(aboutToClose()), this, SLOT(on_explorer_aboutToClose()));

	m_modelFile = QApplication::applicationDirPath() + "/filetree.sxm";
	m_cfgFile = QApplication::applicationDirPath() + "/configure.ini";
	init();
}

SxExplorer::~SxExplorer()
{
	m_pSysTrayIcon->hide();
}



void SxExplorer::init()
{
	m_pWindow->restoreLastPosition(m_cfgFile);
	m_pTreeModel->modelFromFile(m_modelFile);
	m_pTreeModel->enumTree(0, this);
	m_pWindow->show();
}

void SxExplorer::openFile( const QString& file, const QString& name )
{
	for(QList<SxEditor*>::iterator iter = m_lstEditor.begin(); iter != m_lstEditor.end(); iter++)
	{
		SxEditor *editor = *iter;
		if(editor->filePath() == file)
		{
			KWindow *window = editor->realWindow();
			window->activateWindow();
			return ;
		}
	}
	SxEditor *editor = new SxEditor(file, name, this);
	m_lstEditor.push_back(editor);
	KWindow *window = editor->realWindow();
	window->setWindowTitle(name);
	QObject::connect(window, SIGNAL(aboutToClose()), this, SLOT(on_editor_close()));
	window->activateWindow();
}

void SxExplorer::on_editor_close()
{
	KWindow *window = qobject_cast<KWindow*>(sender());

	for(QList<SxEditor*>::iterator iter = m_lstEditor.begin(); iter != m_lstEditor.end(); iter++)
	{
		SxEditor *editor = *iter;
		if(editor->realWindow() == window)
		{
			m_lstEditor.erase(iter);
			return ;
		}
	}	
}

KWindow * SxExplorer::explorerWindow()
{
	return m_pWindow;
}

void SxExplorer::on_tree_clicked( Qt::MouseButton button )
{
	if(button != Qt::RightButton)
		return;

	KMenu *pMenu = KMenu::createPopupMenu();
	{
		QAction *pAction = pMenu->addAction("新建顶级文件", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 100);
		pAction->setData(data);
	}
	pMenu->exec();
}

void SxExplorer::on_tree_itemClicked( qint64 nodeid, Qt::MouseButton button )
{
	if(button != Qt::RightButton)
		return;

	KMenu *pMenu = KMenu::createPopupMenu();
	{
		QAction *pAction = pMenu->addAction("打开文件", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 150);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
	}
	{
		QAction *pAction = pMenu->addAction("打开文件夹", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 151);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
	}
	pMenu->addSeparator();
	{
		QAction *pAction = pMenu->addAction("新建顶级文件", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 100);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
	}
	{
		QAction *pAction = pMenu->addAction("新建子级文件", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 110);
		data.insert("nodeId", nodeid);
		pAction->setData(data);	
	}
	{
		QAction *pAction = pMenu->addAction("新建同级文件", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 111);
		data.insert("nodeId", nodeid);
		pAction->setData(data);	
	}
	{
		QAction *pAction = pMenu->addAction("重命名", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 130);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
	}
	pMenu->addSeparator();
	bool bHasMenuAdd = false;
	if(m_pTreeModel->hasPreviousSibling(nodeid))
	{
		QAction *pAction = pMenu->addAction("移上一层", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 140);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
		bHasMenuAdd = true;
	}
	if(m_pTreeModel->hasNextSibling(nodeid))
	{
		QAction *pAction = pMenu->addAction("移下一层", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 141);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
		bHasMenuAdd = true;
	}
	if(m_pTreeModel->parent(nodeid) > 0)
	{
		QAction *pAction = pMenu->addAction("移至父兄弟", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 142);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
		bHasMenuAdd = true;
	}
	if(m_pTreeModel->hasPreviousSibling(nodeid))
	{
		QAction *pAction = pMenu->addAction("移至上一层儿子", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 145);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
		bHasMenuAdd = true;
	}
	if(m_pTreeModel->hasPreviousSibling(nodeid))
	{
		QAction *pAction = pMenu->addAction("移至最前一层", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 143);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
		bHasMenuAdd = true;
	}
	if(m_pTreeModel->hasNextSibling(nodeid))
	{
		QAction *pAction = pMenu->addAction("移至最后一层", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 144);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
		bHasMenuAdd = true;
	}
	if(bHasMenuAdd)
	{
		pMenu->addSeparator();
	}
	if(m_pTreeModel->hasChildren(nodeid))
	{
		QAction *pAction = pMenu->addAction("删除文件树", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 160);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
	}
	else
	{
		QAction *pAction = pMenu->addAction("删除该文件", this, SLOT(on_tree_menu_action_triggered(bool)));
		QMap<QString,QVariant> data;
		data.insert("menuId", 161);
		data.insert("nodeId", nodeid);
		pAction->setData(data);
	}
	pMenu->exec();
}

void SxExplorer::on_tree_menu_action_triggered( bool triggered )
{
	QAction *pAction = qobject_cast<QAction*>(sender());
	QMap<QString,QVariant> data = pAction->data().toMap();
	int menuId = data.value("menuId").toInt();
	qint64 nodeId = data.value("nodeId").toLongLong();
	if(menuId == 100)
	{
		//新建顶级文件
		QMap<Qt::ItemDataRole, QVariant> item;
		item.insert(Qt::DisplayRole, "New File");
		item.insert((Qt::ItemDataRole)SxCommonTreeModel::ItemDateCreate, QDateTime::currentMSecsSinceEpoch());
		qint64 nodeid = m_pTreeModel->addChild(0, item);
		QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(nodeid);
		KUtility::osCreateDirectory(path, true);
		QFile sxn(path);
		bool bOk = sxn.open(QIODevice::ReadWrite);
		setEdit(nodeid);
		m_pTreeModel->modelToFile(m_modelFile);
	}
	else if(menuId == 110)
	{
		//新建子级文件
		QMap<Qt::ItemDataRole, QVariant> item;
		item.insert(Qt::DisplayRole, "New File");
		item.insert((Qt::ItemDataRole)SxCommonTreeModel::ItemDateCreate, QDateTime::currentMSecsSinceEpoch());
		qint64 nodeid = m_pTreeModel->addChild(nodeId, item);
		m_pTreeWidget->setExpandedNode(nodeId);
		QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(nodeid);
		KUtility::osCreateDirectory(path, true);
		QFile sxn(path);
		bool bOk = sxn.open(QIODevice::ReadWrite);
		setEdit(nodeid);
		m_pTreeModel->modelToFile(m_modelFile);		
	}
	else if(menuId == 111)
	{
		//新建同级文件
		QMap<Qt::ItemDataRole, QVariant> item;
		item.insert(Qt::DisplayRole, "New File");
		item.insert((Qt::ItemDataRole)SxCommonTreeModel::ItemDateCreate, QDateTime::currentMSecsSinceEpoch());
		qint64 nodeid = m_pTreeModel->insertAfter(nodeId, item);
		QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(nodeid);
		KUtility::osCreateDirectory(path, true);
		QFile sxn(path);
		bool bOk = sxn.open(QIODevice::ReadWrite);
		setEdit(nodeid);
		m_pTreeModel->modelToFile(m_modelFile);	
	}
	else if(menuId == 130)
	{
		//重命名
		setEdit(nodeId);
	}
	else if(menuId == 140)
	{
		//移上一层
		m_pTreeModel->moveToLast(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);	
	}
	else if(menuId == 141)
	{
		//移下一层
		m_pTreeModel->moveToNext(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);	
	}
	else if(menuId == 142)
	{
		//移至父兄弟
		m_pTreeModel->moveToParent(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);
		m_pTreeWidget->update();
	}
	else if(menuId == 143)
	{
		//移至最前一层
		m_pTreeModel->moveToFront(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);
	}
	else if(menuId == 144)
	{
		//移至最后一层
		m_pTreeModel->moveToBack(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);
	}
	else if(menuId == 145)
	{
		//移至上一层儿子
		qint64 npid = m_pTreeModel->previousSibling(nodeId);
		m_pTreeModel->moveToChild(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);
		m_pTreeWidget->setExpandedNode(npid);
		m_pTreeWidget->update();
	}
	else if(menuId == 150)
	{
		//打开文件
		QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(nodeId);
		QMap<Qt::ItemDataRole, QVariant> data = m_pTreeModel->data(nodeId);
		QString name = data.value(Qt::DisplayRole).toString();
		openFile(path, name);
	}
	else if(menuId == 151)
	{
		//打开文件夹
		QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(nodeId);
		KUtility::openDirAndSelectFile(path);
	}
	else if(menuId == 160 || menuId == 161)
	{
		//删除文件树或删除该文件
		if(KMessageBox::information(m_pTreeWidget, "提示", "是否继续删除文件吗？", "确定", "取消") != KMessageBox::Accepted)
		{
			return;
		}
		ItemCall tic;
		m_pTreeModel->enumTree(nodeId, &tic);
		QList<qint64> items = tic.m_itemsResult;
		for(int i = 0; i < items.size(); i++)
		{
			QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(items.at(i));
			QFile::remove(path);
		}
		m_pTreeModel->removeNode(nodeId);
		m_pTreeModel->modelToFile(m_modelFile);
	}
}

void SxExplorer::on_tree_itemEditRequest( qint64 nodeid )
{
	m_pTreeWidget->tryToSetAction(nodeid, TREE_ITEM_RENAME, QVariant(), QVariant());
}

void SxExplorer::setEdit( qint64 nodeid )
{
	QMetaObject::invokeMethod(this, "on_tree_itemEditRequest", Qt::QueuedConnection, Q_ARG(qint64,nodeid));
}

void SxExplorer::on_tree_lineEdit_editingFinished()
{
	m_pLineEdit->hide();
	QString name = m_pLineEdit->text();
	qint64 nodeid = m_pLineEdit->data(TREEITEM_ID).toLongLong();
	QMap<Qt::ItemDataRole,QVariant> data = m_pTreeModel->data(nodeid);
	QString nameOld = data.value(Qt::DisplayRole).toString();
	if(nameOld == name)
		return;
	data.insert(Qt::DisplayRole, name);
	m_pTreeModel->setData(nodeid, data);
	m_pTreeWidget->tryToSetAction(nodeid, TREE_ITEM_UPDATE, name, QVariant());
}

void SxExplorer::on_tree_itemExpanded( qint64 nodeid )
{
	m_pTreeModel->setAction(TREE_ITEM_EXPANDED, nodeid, QVariant());
	m_pTreeModel->modelToFile(m_modelFile);
}

void SxExplorer::on_tree_itemCollapsed( qint64 nodeid )
{
	m_pTreeModel->setAction(TREE_ITEM_COLLAPSED, nodeid, QVariant());
	m_pTreeModel->modelToFile(m_modelFile);
}

ItemCall::NextOP SxExplorer::walkAt( qint64 nodeid, int nchild )
{
	QMap<Qt::ItemDataRole, QVariant> vals = m_pTreeModel->data(nodeid);
	bool bExpanded = vals.value((Qt::ItemDataRole)SxCommonTreeModel::ItemExpanded).toBool();
	if(bExpanded)
	{
		QMetaObject::invokeMethod(this, "setItemExpanded", Qt::QueuedConnection, Q_ARG(qint64, nodeid));
	}
	return eWalkChild;
}

void SxExplorer::setItemExpanded( qint64 nodeid )
{
	m_pTreeWidget->setExpandedNode(nodeid);
}

void SxExplorer::on_tree_itemDbClicked( qint64 nodeid, Qt::MouseButton button )
{
	QString path = QApplication::applicationDirPath() + QString("/locals/%1.sxn").arg(nodeid);
	QMap<Qt::ItemDataRole, QVariant> data = m_pTreeModel->data(nodeid);
	QString name = data.value(Qt::DisplayRole).toString();
	openFile(path, name);
}

void SxExplorer::on_explorer_aboutToClose()
{
	m_pWindow->abortCloseWindow();
	m_pWindow->hide();
}

void SxExplorer::on_system_tray_activated( QSystemTrayIcon::ActivationReason reason )
{
	if(reason == QSystemTrayIcon::Trigger)
	{
		//显示在最前面
		KPopupWindow *pWindow = qobject_cast<KPopupWindow*>(m_pWindow);
		pWindow->show();
		pWindow->activateWindow();
	}
	else if(reason == QSystemTrayIcon::Context)
	{
		KMenu *pMenu = KMenu::createPopupMenu();
		{
			QAction *pAction = pMenu->addAction("随系统启动", this, SLOT(on_system_tray_menu_action_triggered(bool)));
			QMap<QString,QVariant> data;
			data.insert("menuId", 100);
			pAction->setData(data);
			pAction->setCheckable(true);
			bool bchecked = KUtility::hasWindowStartRun("sxnote");
			pAction->setChecked(bchecked);
		}
		{
			QAction *pAction = pMenu->addAction("显示在最前面", this, SLOT(on_system_tray_menu_action_triggered(bool)));
			QMap<QString,QVariant> data;
			data.insert("menuId", 110);
			pAction->setData(data);
		}
		{
			QAction *pAction = pMenu->addAction("重置到屏幕中心", this, SLOT(on_system_tray_menu_action_triggered(bool)));
			QMap<QString,QVariant> data;
			data.insert("menuId", 120);
			pAction->setData(data);
		}
		{
			QAction *pAction = pMenu->addAction("退出程序", this, SLOT(on_system_tray_menu_action_triggered(bool)));
			QMap<QString,QVariant> data;
			data.insert("menuId", 130);
			pAction->setData(data);
		}
		pMenu->exec();
	}
}

void SxExplorer::on_system_tray_menu_action_triggered( bool triggered )
{
	QAction *pAction = qobject_cast<QAction*>(sender());
	QMap<QString,QVariant> data = pAction->data().toMap();
	int menuId = data.value("menuId").toInt();
	if(menuId == 100)
	{
		//随系统启动
		bool bchecked = pAction->isChecked();
		if(bchecked)
		{
			KUtility::createWindowStartRun(KUtility::osExeInstancePath(), "sxnote");
		}
		else
		{
			KUtility::removeWindowStartRun("sxnote");
		}
	}
	else if(menuId == 110)
	{
		//显示在最前面
		KPopupWindow *pWindow = qobject_cast<KPopupWindow*>(m_pWindow);
		pWindow->show();
		pWindow->activateWindow();
	}
	else if(menuId == 120)
	{
		//重置到屏幕中心
		KPopupWindow *pWindow = qobject_cast<KPopupWindow*>(m_pWindow);
		pWindow->moveCenter(NULL);
		pWindow->show();
		pWindow->activateWindow();
	}
	else if(menuId == 130)
	{
		//退出程序
		m_pWindow->saveLastPosition(m_cfgFile);
		QApplication::exit();
	}
}

void SxExplorer::on_trayTimer_timeout()
{
	m_pSysTrayIcon->setIcon(QIcon("res:/Resources/image/tree_note.png"));
}
