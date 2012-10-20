#ifndef MOTHERWIDGET_H
#define MOTHERWIDGET_H

#include <QWidget>

class QSplitter;

class MotherWidget : public QWidget
{
	Q_OBJECT
public:
	
	explicit MotherWidget(QWidget *parent = 0);
	
protected:
	
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	
signals:
	
public slots:
	
private slots:
	
	void onTabWidgetWillBeDeleted();
	
private:
	
	enum
	{
		InsertDistance = 20
	};
	
	QSplitter *_leftSplitter;
	int _leftWidgetCount;
	QWidget *_centralWidget;
	
	QSplitter *_splitter;
};

#endif // MOTHERWIDGET_H
