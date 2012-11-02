#ifndef DRAGGABLETABWIDGET_H
#define DRAGGABLETABWIDGET_H

#include <QTabBar>
#include <QTabWidget>


class DockTabBar;

class DockTabWidget : public QTabWidget
{
	Q_OBJECT
	friend class DockTabBar;
public:
	
	explicit DockTabWidget(QWidget *parent = 0);
	
	/**
	 * Creates a DockTabWidget copying other's DockTabWidget specific settings.
	 * @param other
	 * @param parent
	 */
	DockTabWidget(DockTabWidget *other, QWidget *parent = 0);
	
	static void moveTab(DockTabWidget *source, int sourceIndex, DockTabWidget *dest, int destIndex);
	static void decodeTabDropEvent(QDropEvent *event, DockTabWidget **p_tabWidget, int *p_index);
	static bool eventIsTabDrag(QDragEnterEvent *event);
	
	void setFloatingBaseWidget(QWidget *widget);
	QWidget *floatingBaseWidget() { return _floatingBaseWidget; }
	
	void setFloatingEnabled(bool x);
	bool isFloatingEnabled() const { return _floatingEnabled; }
	
	virtual bool isInsertable(QWidget *widget);
	bool isInsertable(DockTabWidget *other, int index) { return isInsertable(other->widget(index)); }
	virtual DockTabWidget *createAnother(QWidget *parent = 0);
	
signals:
	
	void willBeAutomaticallyDeleted(DockTabWidget *widget);
	
public slots:
	
	void deleteIfEmpty();
	
private:
	
	QWidget *_floatingBaseWidget = 0;
	bool _floatingEnabled = false;
};

class DockTabBar : public QTabBar
{
	Q_OBJECT
public:
	
	DockTabBar(DockTabWidget *tabWidget, QWidget *parent = 0);
	
	DockTabWidget *tabWidget() { return _tabWidget; }
	
protected:
	
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	
private:
	
	int insertionIndexAt(const QPoint &pos);
	
	DockTabWidget *_tabWidget = 0;
	bool _isStartingDrag = false;
	QPoint _dragStartPos;
};

#endif // DRAGGABLETABWIDGET_H
