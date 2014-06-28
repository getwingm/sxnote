#pragma once;

class SxRichEdit : public KTextEdit
{
	Q_OBJECT
	Q_PROPERTY(QString file READ filePath WRITE setFilePath)
public:
	explicit SxRichEdit(QGraphicsItem* parent);
	~SxRichEdit();

	bool openFile(const QString& file);
	bool saveFile(const QString& file);
	void save();

	QString filePath() const;
	void setFilePath(const QString& file);
private:
	QString m_filePath;
};

#if 0
void CreateBookWindow::createList(QAction *actionTemp)
{
	if(actionTemp==bulletDisc)
		listFormat.setStyle(QTextListFormat::ListDisc);
	else if(actionTemp==bulletCircle)
		listFormat.setStyle(QTextListFormat::ListCircle);
	else if(actionTemp==bulletSquare)
		listFormat.setStyle(QTextListFormat::ListSquare);
	else if(actionTemp==bulletDecimal)
		listFormat.setStyle(QTextListFormat::ListDecimal);
	else if(actionTemp==bulletAlphaLower)
		listFormat.setStyle(QTextListFormat::ListLowerAlpha);
	else if(actionTemp==bulletAlphaUpper)
		listFormat.setStyle(QTextListFormat::ListUpperAlpha);
	else if(actionTemp==bulletRomanLower)
		listFormat.setStyle(QTextListFormat::ListLowerRoman);
	else
		listFormat.setStyle(QTextListFormat::ListUpperRoman);

	listFormat.setIndent(listFormat.indent());
	editor->textCursor().insertList(listFormat);

}
#endif