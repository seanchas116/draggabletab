#include <QtGui>

#include "draggabletabwidget.h"

#define MIMETYPE_TABINDEX "x-paintfield-tabindex"

DraggableTabWidget::DraggableTabWidget(QWidget *parent) :
	QTabWidget(parent)
{
	setTabBar(new DraggableTabBar(this));
	setDocumentMode(true);
}

void DraggableTabWidget::moveTab(DraggableTabWidget *source, int sourceIndex, DraggableTabWidget *dest, int destIndex)
{
	if (source == dest && sourceIndex < destIndex)
		destIndex--;
	
	QWidget *widget = source->widget(sourceIndex);
	QString text = source->tabText(sourceIndex);
	
	source->removeTab(sourceIndex);
	
	dest->insertTab(destIndex, widget, text);
	dest->setCurrentIndex(destIndex);
}

DraggableTabWidget::TabInfo DraggableTabWidget::decodeTabDropEvent(QDropEvent *event)
{
	DraggableTabBar *tabBar = qobject_cast<DraggableTabBar *>(event->source());
	if (!tabBar)
		return TabInfo(0, 0);
	
	QByteArray data = event->mimeData()->data(MIMETYPE_TABINDEX);
	QDataStream stream(&data, QIODevice::ReadOnly);
	
	int index;
	stream >> index;
	
	return TabInfo(index, tabBar->tabWidget());
}

bool DraggableTabWidget::eventIsTabDrag(QDragEnterEvent *event)
{
	return event->mimeData()->hasFormat(MIMETYPE_TABINDEX) && qobject_cast<DraggableTabBar *>(event->source());
}

void DraggableTabWidget::deleteIfEmpty()
{
	if (count() == 0)
	{
		emit willBeDeleted();
		deleteLater();
	}
}


DraggableTabBar::DraggableTabBar(DraggableTabWidget *tabWidget, QWidget *parent) :
	QTabBar(parent),
	_tabWidget(tabWidget),
	_isStartingDrag(false)
{
	setAcceptDrops(true);
}

int DraggableTabBar::insertionIndexAt(const QPoint &pos)
{
	int index = count();
	for (int i = 0; i < count(); ++i)
	{
		QRect rect = tabRect(i);
		QRect rect1(rect.x(), rect.y(), rect.width() / 2, rect.height());
		QRect rect2(rect.x() + rect1.width(), rect.y(), rect.width() - rect1.width(), rect.height());
		if (rect1.contains(pos))
		{
			index = i;
			break;
		}
		if (rect2.contains(pos))
		{
			index = i + 1;
			break;
		}
	}
	return index;
}

void DraggableTabBar::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		_dragStartPos = event->pos();
		_isStartingDrag = true;
	}
	QTabBar::mousePressEvent(event);
}

void DraggableTabBar::mouseMoveEvent(QMouseEvent *event)
{
	if (!_isStartingDrag)
		return;
	
	if (!event->buttons() & Qt::LeftButton)
		return;
	
	if ((event->pos() - _dragStartPos).manhattanLength() < QApplication::startDragDistance())
		return;
	
	int index = tabAt(event->pos());
	
	if (index < 0)
		return;
	
	// create data
	
	QMimeData *mimeData = new QMimeData;
	
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << index;
	
	mimeData->setData(MIMETYPE_TABINDEX, data);
	
	// create pixmap
	
	QRect rect = tabRect(index);
	QPixmap pixmap(rect.size());
	
	render(&pixmap, QPoint(), QRegion(rect));
	
	// exec drag
	
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap);
	QPoint offset = _dragStartPos - rect.topLeft();
	drag->setHotSpot(offset);
	Qt::DropAction dropAction = drag->exec(Qt::MoveAction | Qt::IgnoreAction);
	
	if (dropAction != Qt::MoveAction)	// drop is not accepted
	{
		QRect newGeometry(QCursor::pos() - offset, tabWidget()->geometry().size());
		
		DraggableTabWidget *newTabWidget = new DraggableTabWidget;
		DraggableTabWidget::moveTab(_tabWidget, index, newTabWidget, 0);
		
		newTabWidget->setGeometry(newGeometry);
		newTabWidget->show();
	}
	
	_tabWidget->deleteIfEmpty();
	_isStartingDrag = false;
}

void DraggableTabBar::dragEnterEvent(QDragEnterEvent *event)
{
	if (DraggableTabWidget::eventIsTabDrag(event))
		event->acceptProposedAction();
}

void DraggableTabBar::dropEvent(QDropEvent *event)
{
	DraggableTabWidget::TabInfo oldInfo = DraggableTabWidget::decodeTabDropEvent(event);
	if (!oldInfo.tabWidget)
		return;
	
	int newIndex = insertionIndexAt(event->pos());
	
	DraggableTabWidget::moveTab(oldInfo.tabWidget, oldInfo.index, _tabWidget, newIndex);
	
	event->acceptProposedAction();
}
