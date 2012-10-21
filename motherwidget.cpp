#include <QtGui>

#include "draggabletabwidget.h"

#include "motherwidget.h"

MotherWidget::MotherWidget(QWidget *parent) :
	QWidget(parent)
{
	_mainSplitter = new QSplitter(Qt::Horizontal);
	
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(_mainSplitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
	
	_centralWidget = new QWidget;
	
	_mainSplitter->addWidget(_centralWidget);
	_mainSplitter->setStretchFactor(0, 1);
	
	setAcceptDrops(true);
}

void MotherWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (DraggableTabWidget::eventIsTabDrag(event))
		event->acceptProposedAction();
}

void MotherWidget::dropEvent(QDropEvent *event)
{
	DraggableTabWidget::TabInfo tabInfo = DraggableTabWidget::decodeTabDropEvent(event);
	if (!tabInfo.tabWidget)
		return;
	
	dropTab(tabInfo.tabWidget, tabInfo.index, event->pos());
	event->acceptProposedAction();
}

void MotherWidget::onTabWidgetWillBeDeleted(DraggableTabWidget *widget)
{
	for (QSplitter *splitter : _columnSplitterLists[Left] + _columnSplitterLists[Right])
	{
		if (splitter->count() == 1 && splitter->widget(0) == widget)
		{
			_columnSplitterLists[Left].removeAll(splitter);
			_columnSplitterLists[Right].removeAll(splitter);
			splitter->deleteLater();
		}
	}
}

void MotherWidget::dropTab(DraggableTabWidget *srcTabWidget, int srcIndex, const QPoint &pos)
{
	TabHandling tabHandling;
	Direction dir;
	int columnIndex;
	
	getTabHandling(pos, tabHandling, dir, columnIndex);
	
	if (tabHandling == NoHandling)
		return;
	
	DraggableTabWidget *dstTabWidget = new DraggableTabWidget;
	DraggableTabWidget::moveTab(srcTabWidget, srcIndex, dstTabWidget, 0);
	
	if (tabHandling == NewColumn)
	{
		QSplitter *splitter = new QSplitter(Qt::Vertical);
		splitter->addWidget(dstTabWidget);
		int mainIndex = dir == Left ? columnIndex : (_mainSplitter->count() - columnIndex);
		_mainSplitter->insertWidget(mainIndex, splitter);
		_mainSplitter->setStretchFactor(mainIndex, 0);
		_columnSplitterLists[dir] << splitter;
	}
	else
	{
		QSplitter *splitter = _columnSplitterLists[dir].at(columnIndex);
		switch (tabHandling)
		{
			default:
			case TabPrepend:
				splitter->insertWidget(0, dstTabWidget);
				break;
			case TabAppend:
				splitter->addWidget(dstTabWidget);
				break;
		}
	}
	
	connect(dstTabWidget, SIGNAL(willBeDeleted(DraggableTabWidget*)), this, SLOT(onTabWidgetWillBeDeleted(DraggableTabWidget*)));
}

MotherWidget::Direction MotherWidget::insertionDirection(QSplitter *splitter, const QPoint &pos)
{
	auto isInInsertDist = [](int x, int border)->bool { return border - InsertDistance <= x && x < border + InsertDistance; };
	
	QRect geom = splitter->geometry();
	int left = geom.left();
	int rightEnd = geom.left() + geom.width();
	int top = geom.top();
	int bottomEnd = geom.top() + geom.height();
	
	int x = pos.x();
	int y = pos.y();
	
	if (left <= x && x < rightEnd)
	{
		if (isInInsertDist(top, y))
			return Top;
		
		if (isInInsertDist(bottomEnd, y))
			return Bottom;
	}
	
	if (top <= y && y < bottomEnd)
	{
		if (isInInsertDist(left, x))
			return Left;
		
		if (isInInsertDist(rightEnd, x))
			return Right;
	}
	
	return NoDirection;
}

void MotherWidget::getTabHandling(const QPoint &dropPos, TabHandling &handling, Direction &columnDirection, int &index)
{
	for (int i = 0; i < 2; ++i)
	{
		columnDirection = i == 0 ? Left : Right;
		
		if (_columnSplitterLists[columnDirection].size() == 0)
		{
			int distFromSide = columnDirection == Left ? dropPos.x() : (width() - dropPos.x());
			
			if (distFromSide < InsertDistance)
			{
				handling = NewColumn;
				index = 0;
				return;
			}
		}
		else
		{
			for (int i = 0; i < _columnSplitterLists[columnDirection].size(); ++i)
			{
				QSplitter *splitter = _columnSplitterLists[columnDirection].at(i);
				Direction dir = insertionDirection(splitter, dropPos);
				
				if (columnDirection == Right)
				{
					if (dir == Left)
						dir = Right;
					else if (dir == Right)
						dir = Left;
				}
				
				switch (dir)
				{
					default:
						break;
					case Right:
						handling = NewColumn;
						index = i+1;
						return;
					case Top:
						handling = TabPrepend;
						index = i;
						return;
					case Bottom:
						handling = TabAppend;
						index = i;
						return;
					case Left:
						handling = NewColumn;
						index = i;
						return;
				}
			}
		}
	}
	handling = NoHandling;
}

