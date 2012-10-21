#ifndef MOTHERWIDGET_H
#define MOTHERWIDGET_H

#include <QWidget>

class QSplitter;
class DraggableTabWidget;

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
	
private:
	
	enum
	{
		InsertDistance = 20,
		DirectionCount = 2
	};
	
	enum TabHandling
	{
		NoHandling,
		TabAppend,
		TabPrepend,
		NewColumn
	};
	
	enum Direction
	{
		Left = 0,
		Right = 1,
		Top = 2,
		Bottom = 3,
		NoDirection = -1
	};
	
	void dropTab(DraggableTabWidget *tabWidget, int index, const QPoint &pos);
	void getTabHandling(const QPoint &dropPos, TabHandling &handling, Direction &columnDirection, int &index);
	Direction insertionDirection(QSplitter *splitter, const QPoint &pos);
	
private slots:
	
	void onTabWidgetWillBeDeleted(DraggableTabWidget *widget);
	
private:
	
	QList<QSplitter *> _columnSplitterLists[DirectionCount];
	
	QWidget *_centralWidget = 0;
	QSplitter *_mainSplitter = 0;
};

#endif // MOTHERWIDGET_H
