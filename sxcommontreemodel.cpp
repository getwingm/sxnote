#include "stdafx.h"
#include "sxcommontreemodel.h"

SxCommonTreeModel::SxCommonTreeModel( QObject *parent/*=NULL*/ )
: KCommonTreeModel(parent)
{

}

SxCommonTreeModel::~SxCommonTreeModel()
{

}

void SxCommonTreeModel::modelFromFile( const QString& file )
{
	QString model = file;
	QFile iof(model);
	if(iof.open(QFile::ReadOnly))
	{
		QByteArray utf8Data = iof.readAll();
		QString szModel = QString::fromUtf8(utf8Data.data(), utf8Data.size());
		Node *root = new Node;
		modelFromMemory(szModel, root);
		m_roots = root->childs;
		for(int i = 0; i < m_roots.size(); i++)
		{
			Node *n = m_roots[i];
			n->parent = NULL;
		}
		root->childs.clear();
	}
	iof.close();
	emit modelReset();
}

void SxCommonTreeModel::modelToFile( const QString& file )
{
	QString model = file;
	QString szMode;
	for(int i = 0;  i < m_roots.size(); i++)
	{
		szMode += modelToMemory(m_roots.at(i), 0);
	}
	QFile iof(model);
	if(iof.open(QFile::WriteOnly))
	{
		QByteArray utf8 = szMode.toUtf8();
		iof.write(utf8);
	}
	iof.close();
}

void SxCommonTreeModel::modelFromMemory( const QString& data, Node *root )
{
	const QStringList lines = data.split("\n");
	QList<Node*> parents;
	QList<int> indentations;
	parents << root;
	indentations << 0;

	int number = 0;
	while (number < lines.count()) 
	{
		int position = 0;
		while (position < lines[number].length()) 
		{
			if (lines[number].mid(position, 1) != " ")
				break;
			position++;
		}

		QString lineData = lines[number].mid(position).trimmed();

		if (!lineData.isEmpty()) 
		{
			// Read the column data from the rest of the line.
			QStringList columnStrings = lineData.split("\t");
			QVector<QVariant> columnData;
			for (int column = 0; column < columnStrings.count(); ++column)
			{
				columnData << columnStrings[column];
			}

			if (position > indentations.last()) 
			{
				// The last child of the current parent is now the new parent
				// unless the current parent has no children.
				Node *np = parents.last();

				if (np->childs.size()) 
				{
					parents << np->childs.last();
					indentations << position;
				}
			} 
			else 
			{
				while (position < indentations.last() && parents.count() > 0) 
				{
					parents.pop_back();
					indentations.pop_back();
				}
			}

			// Append a new item to the current parent's list of children.
			qint64 nodeid = columnData[0].toLongLong();
			QString name = columnData[1].value<QString>();
			qint64 time = QDateTime::fromString(columnData[2].value<QString>(), "yyMMddhhmmsszzz").toMSecsSinceEpoch();
			bool bExpanded = columnData[3].toBool();
			QMap<Qt::ItemDataRole,QVariant> data;
			data.insert((Qt::ItemDataRole)ItemDateCreate, time);
			data.insert((Qt::ItemDataRole)ItemExpanded, bExpanded);
			data.insert(Qt::DisplayRole, name);
			Node* np = parents.last();
			Node *nc = new Node;
			nc->data = data;
			nc->parent = np;
			nc->nodeid = nodeid;
			np->childs.push_back(nc);
			m_nodes.insert(nc->nodeid, nc);
		}

		number++;
	}
}

QString SxCommonTreeModel::modelToMemory( Node *pNode, int indent )
{
	QString szBuffer;
	QByteArray space;

	space.resize(indent * 4);
	space.fill(' ');
	QMap<Qt::ItemDataRole, QVariant> vals = pNode->data;
	QString name = vals.value(Qt::DisplayRole).toString();
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(vals.value((Qt::ItemDataRole)ItemDateCreate).value<qint64>());
	QString time = dt.toString("yyMMddhhmmsszzz");
	bool bExpanded = vals.value(Qt::ItemDataRole(ItemExpanded)).toBool();
	QString szTxt = QString("%1%2\t%3\t%4\t%5\n").arg(space.data()).arg(pNode->nodeid).arg(name).arg(time).arg(bExpanded);
	szBuffer += szTxt;
	for(int i = 0; i < pNode->childs.size(); i++)
	{
		szBuffer += modelToMemory(pNode->childs.at(i), indent+1);
	}
	return szBuffer;
}

bool SxCommonTreeModel::setData( qint64 nodeid, const QMap<Qt::ItemDataRole, QVariant> &val )
{
	Node *pn = m_nodes.value(nodeid);
	if(pn)
	{
		pn->data = val;
		return true;
	}
	return false;
}

bool SxCommonTreeModel::setAction( int id, const QVariant& val, QVariant& result )
{
	if(id == TREE_ITEM_EXPANDED)
	{
		Node *n = m_nodes.value(val.toLongLong());
		if(n)
		{
			n->data.insert(Qt::ItemDataRole(ItemExpanded), 1);
		}
		return true;
	}
	else if(id == TREE_ITEM_COLLAPSED)
	{
		Node *n = m_nodes.value(val.toLongLong());
		if(n)
		{
			n->data.insert(Qt::ItemDataRole(ItemExpanded), 0);
		}
		return true;
	}
	return false;
}

void SxCommonTreeModel::setTreeWidget( KTreeWidget *pTreeWidget )
{
	m_pTreeWidget = pTreeWidget;
}



SxTreeItem::SxTreeItem( KTreeWidget *view )
: KTreeItem(view)
{

}

SxTreeItem::~SxTreeItem()
{

}

bool SxTreeItem::setAction( int id, const QVariant& val, QVariant& result )
{
	if(id == TREE_ITEM_RENAME)
	{
		KLineEdit *lineEdit = qobject_cast<KLineEdit*>(m_treeWidget->data(LINEEDIT_ID).value<QObject*>());
		lineEdit->setData(TREEITEM_ID, m_nodeid);
		QMargins m = margins();
		QRectF geom = geometry();
		geom.adjust(m.left(), m.top(), -m.right(), -m.bottom());
		QString name = itemText();
		lineEdit->setGeometry(geom);
		lineEdit->setText(name);
		lineEdit->show();
		lineEdit->setFocus(true);
		return true;
	}
	else if(id == TREE_ITEM_UPDATE)
	{
		m_data.insert(Qt::DisplayRole, val);
		m_label->setText(val.toString()); 
		return true;
	}
	return false;
}
