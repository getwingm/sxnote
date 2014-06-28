#include "stdafx.h"
#include "sxeditor.h"
#include "SxRichEdit.h"

SxEditor::SxEditor( const QString& file, const QString& name, QObject *parent /*= NULL*/ )
: QObject(parent)
, m_szFile(file)
, m_pTextEdit(NULL)
{
	m_pWindow = KXmlUI::windowFromFile("res:/Resources/layout/sxeditor.xml");
	KXmlUI::setPropertyEx(m_pWindow, "titlebar.title", name);

	KWidget *rootWidget = m_pWindow->rootWidget();
	KToolBar *toolbar = rootWidget->findChild<KToolBar*>("edittoolbar");
	QGraphicsLinearLayout *linear = dynamic_cast<QGraphicsLinearLayout*>(toolbar->layout());
	for(int i = 0; i < linear->count(); i++)
	{
		QGraphicsLayoutItem * item = linear->itemAt(i);
		KPushButton *button = dynamic_cast<KPushButton*>(item);
		if(button)
		{
			bool bok = QObject::connect(button, SIGNAL(clicked()), this, SLOT(on_common_command_clicked()));
		}
	}

	m_pTextEdit = rootWidget->findChild<SxRichEdit*>("richtexteditor");
	QObject::connect(m_pTextEdit, SIGNAL(contextMenu(QMenu*,bool*)), this, SLOT(on_editor_contextMenu(QMenu*,bool*)));

	QFont font("SimSun", 9);
	m_pTextEdit->setFont(font);

	QPoint pt = KUtility::nextCascadesWindowPosition();
	m_pWindow->move(pt);
	m_pWindow->show();

	QObject::connect(m_pWindow, SIGNAL(aboutToClose()), this, SLOT(on_aboutToClose_triggered()));
	
	QMetaObject::invokeMethod(this, "asynOpen", Qt::QueuedConnection);
}

SxEditor::~SxEditor()
{

}

void SxEditor::asynOpen()
{
	if(!m_pTextEdit->openFile(m_szFile))
	{
		KMessageBox::warning(m_pTextEdit, "警告", "打开文件失败。", "确定");
		return;
	}
}

QString SxEditor::filePath() const
{
	return m_szFile;
}

KWindow * SxEditor::realWindow()
{
	return m_pWindow;
}

void SxEditor::on_aboutToClose_triggered()
{
	if(m_pTextEdit->isModified())
	{
		int retval = KMessageBox::warning(m_pTextEdit, "警告", "文件已经被修改，是否保存文件后再关闭？", "保存并退出", "取消", "返回");
		if(retval == MSGBOX_ID0)
		{
			m_pTextEdit->save();
		}
		else if(retval == MSGBOX_ID2)
		{
			m_pWindow->abortCloseWindow();
			return;
		}
	}
}

void SxEditor::on_btn_clicked()
{
	QObject *obj = sender();
	QString objname = obj->objectName();
	QString num = objname.remove("btn");

	if(num.toInt() == 1)
	{
		QColorDialog dlg;
		QObject::connect(&dlg, SIGNAL(currentColorChanged(QColor)), this, SLOT(on_colorSelected(QColor)));
		dlg.exec();
	}
	else
	{
		KTheme::setThemeIndex(num.toInt()-1);
	}
}

void SxEditor::on_colorSelected( const QColor& color )
{
	KTheme::setThemeColor(color);
}

void SxEditor::on_common_command_clicked()
{
	QObject *btn = sender();
	QString name = btn->objectName();

	if(name.compare("selectAll") == 0)
	{
		m_pTextEdit->selectAll();
	}
	if(name.compare("save") == 0)
	{
		m_pTextEdit->save();
	}
	else if(name.compare("copy") == 0)
	{
		m_pTextEdit->copy();
	}
	else if(name.compare("paste") == 0)
	{
		m_pTextEdit->paste();
	}
	else if(name.compare("cut") == 0)
	{
		m_pTextEdit->cut();
	}
	else if(name.compare("redo") == 0)
	{
		m_pTextEdit->redo();
	}
	else if(name.compare("undo") == 0)
	{
		m_pTextEdit->undo();
	}
	else if(name.compare("delete") == 0)
	{
		m_pTextEdit->deleteText();
	}
	else if(name.compare("bold") == 0)
	{
		m_pTextEdit->setFontBold(!m_pTextEdit->fontBold());
	}
	else if(name.compare("italic") == 0)
	{
		m_pTextEdit->setFontItalic(!m_pTextEdit->fontItalic());
	}
	else if(name.compare("underline") == 0)
	{
		m_pTextEdit->setFontUnderline(!m_pTextEdit->fontUnderline());
	}
	else if(name.compare("throughout") == 0)
	{
		m_pTextEdit->setFontStrikeOut(!m_pTextEdit->fontStrikeOut());
	}
	else if(name.compare("alignleft") == 0)
	{
		m_pTextEdit->setTextAlignment(Qt::AlignLeft);
	}
	else if(name.compare("aligncenter") == 0)
	{
		m_pTextEdit->setTextAlignment(Qt::AlignCenter);
	}
	else if(name.compare("alignright") == 0)
	{
		m_pTextEdit->setTextAlignment(Qt::AlignRight);
	}
	else if(name.compare("textcolor") == 0)
	{
		QColor clr = QColorDialog::getColor(m_pTextEdit->textColor(), m_pTextEdit->view());
		m_pTextEdit->setTextColor(clr);
	}
	else if(name.compare("link") == 0)
	{
		m_pUrlInsert = qobject_cast<KDialogBase*>(KXmlUI::windowFromFile("res:/Resources/layout/urlinsert.xml", m_pWindow));
		KPushButton *btnApply = m_pUrlInsert->findChild<KPushButton*>("btnApply");
		KPushButton *btnCancel = m_pUrlInsert->findChild<KPushButton*>("btnCancel");
		QObject::connect(btnApply, SIGNAL(clicked()), this, SLOT("on_urlInsert_button_clicked()"));
		QObject::connect(btnCancel, SIGNAL(clicked()), m_pUrlInsert, SLOT("setRejected()"));
		m_pUrlInsert->doModal(m_pWindow->rootWidget());
	}
	else if(name.compare("replace") == 0)
	{

	}
	else if(name.compare("find") == 0)
	{

	}
}

void SxEditor::on_urlInsert_button_clicked()
{
	KLineEdit *label = m_pUrlInsert->findChild<KLineEdit*>("label");
	KLineEdit *link = m_pUrlInsert->findChild<KLineEdit*>("link");
	QString labelTxt = label->text();
	QString linkTxt = link->text();
	
	if(labelTxt.isEmpty())
	{
		KMessageBox::information(m_pUrlInsert->rootWidget(), "超链接", "标签名不能为空", "确定");
		return;
	}
	if(linkTxt.isEmpty())
	{
		KMessageBox::information(m_pUrlInsert->rootWidget(), "超链接", "链接的内容不能为空", "确定");
		return;
	}
	m_pUrlInsert->setAccepted();
	KTextCursor txtCursor = m_pTextEdit->textCursor();
	txtCursor.insertText(labelTxt, Qt::blue, linkTxt, linkTxt, true);
}

void SxEditor::on_editor_contextMenu( QMenu* menu, bool *pbContinue )
{
	KMenu *pMenu = qobject_cast<KMenu*>(menu);
	{
		QAction *action = pMenu->addAction("粗体", this, SLOT(on_common_command_clicked()));
		action->setObjectName("bold");
	}
	{
		QAction *action = pMenu->addAction("斜体", this, SLOT(on_common_command_clicked()));
		action->setObjectName("italic");
	}
	{
		QAction *action = pMenu->addAction("下划线", this, SLOT(on_common_command_clicked()));
		action->setObjectName("underline");
	}
	{
		QAction *action = pMenu->addAction("删除线", this, SLOT(on_common_command_clicked()));
		action->setObjectName("throughout");
	}
	{
		QAction *action = menu->addSeparator();
	}

	{
		QAction *action = menu->addAction("选择全部", this, SLOT(on_common_command_clicked()));
		action->setObjectName("selectAll");
	}

	{
		QAction *action = menu->addAction("粘贴", this, SLOT(on_common_command_clicked()));
		bool bEnable = false;
		QClipboard *clipboard = QApplication::clipboard();
		if(clipboard )
		{
			const QMimeData *mimedata = clipboard->mimeData();
			if (mimedata->hasHtml() || mimedata->hasImage() || mimedata->hasText() || mimedata->hasUrls() || mimedata->hasFormat(KTextEditMime))
			{
				bEnable = true;
			}
		}
		action->setEnabled(bEnable && !m_pTextEdit->isReadOnly());
		action->setObjectName("paste");
	}

	QTextCursor cursor = m_pTextEdit->textCursor();
	bool hasText = !cursor.selection().isEmpty();
	{
		QAction *action = menu->addAction("剪切", this, SLOT(on_common_command_clicked()));
		action->setEnabled(hasText && !m_pTextEdit->isReadOnly());
		action->setObjectName("cut");
	}

	{
		QAction *action = menu->addAction("复制", this, SLOT(on_common_command_clicked()));
		action->setEnabled(hasText);
		action->setObjectName("copy");
	}
	*pbContinue  = false;
}
