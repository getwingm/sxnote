#include "stdafx.h"
#include "sxrichedit.h"


KX_WIDGET_CREATOR_GLOBAL_STATIC(SxRichEdit)

SxRichEdit::SxRichEdit( QGraphicsItem* parent )
: KTextEdit(parent)
{
}

SxRichEdit::~SxRichEdit()
{

}

bool SxRichEdit::openFile( const QString& file )
{
	m_filePath = file;
	QList<QPair<QString,QTextCharFormat>> frags;
	QFile fw(m_filePath);
	if(fw.open(QIODevice::ReadOnly))
	{
		QDataStream read(&fw);
		int fragCount = 0;
		read >> fragCount;
		KTextCursor txtCursor = textCursor();
		for(int i = 0; i < fragCount; i++)
		{
			QPair<QString,QTextCharFormat> frag;
			read >> frag;
			txtCursor.insertText(frag.first, frag.second);
		}
		QTextDocument *pdoc = document();
		pdoc->setModified(false);
		return true;
	}
	return false;
}

bool SxRichEdit::saveFile( const QString& file )
{
	KUtility::osCreateDirectory(file, true);

	QList<QPair<QString,QTextCharFormat>> frags;
	getTextFragments(frags);
	QFile fw(m_filePath);
	if(fw.open(QIODevice::WriteOnly))
	{
		QDataStream out(&fw);
		out << frags.size();
		for(QList<QPair<QString,QTextCharFormat>>::iterator iter = frags.begin(); iter != frags.end(); iter++)
		{
			QPair<QString,QTextCharFormat>& frag = *iter;
			QString txt = frag.first;
			QTextCharFormat fmt = frag.second;
			out << frag;
		}
		setModified(false);
		return true;
	}
	return false;
}

void SxRichEdit::save()
{
	saveFile(m_filePath);
}

QString SxRichEdit::filePath() const
{
	return m_filePath;
}

void SxRichEdit::setFilePath( const QString& file )
{
	m_filePath = file;
}

