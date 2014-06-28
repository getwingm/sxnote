#pragma once;

class SxRichEdit;

class SxEditor : public QObject
{
	Q_OBJECT
public:
	explicit SxEditor(const QString& file, const QString& name, QObject *parent = NULL);
	~SxEditor();

	QString filePath() const;
	KWindow *realWindow();
private slots:
	void on_aboutToClose_triggered();
	void on_btn_clicked();
	void on_colorSelected(const QColor& color);
	void on_common_command_clicked();
	void on_urlInsert_button_clicked();
	void on_editor_contextMenu(QMenu* menu, bool *pbContinue);
private:
	Q_INVOKABLE void asynOpen();
private:
	KWindow *m_pWindow;
	QPointer<KDialogBase> m_pUrlInsert;
	SxRichEdit *m_pTextEdit;
	const QString m_szFile;
};