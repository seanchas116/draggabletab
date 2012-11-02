#include <QtGui>

#include "docktabwidget.h"

#include "docktabmotherwidget.h"

QPoint mapToAncestor(QWidget *ancestor, QWidget *widget, const QPoint &pos)
{
	QPoint result = pos;
	
	do
	{
		result = widget->mapToParent(result);
		widget = widget->parentWidget();
	}
	while (widget && widget != ancestor);
	
	return result;
}

DockTabMotherWidget::DockTabMotherWidget(QWidget *parent) :
	QWidget(parent)
{
	_mainHorizontalSplitter = new QSplitter(Qt::Horizontal);
	_mainVerticalSplitter = new QSplitter(Qt::Vertical);
	_centralWidget = new QWidget;
	
	_mainVerticalSplitter->addWidget(_centralWidget);
	_mainVerticalSplitter->setStretchFactor(0, 1);
	
	_mainHorizontalSplitter->addWidget(_mainVerticalSplitter);
	_mainHorizontalSplitter->setStretchFactor(0, 1);
	
	auto layout = new QHBoxLayout;
	layout->addWidget(_mainHorizontalSplitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
	
	setAcceptDrops(true);
}

bool DockTabMotherWidget::insertTabWidget(DockTabWidget *tabWidget, const TabWidgetArea &area)
{
	if (!area.isValid())
		return false;
	
	if (area.tabWidgetIndex == -1)
	{
		Qt::Orientation orientation;
		if (area.dir == Left || area.dir == Right)
			orientation = Qt::Vertical;
		else
			orientation = Qt::Horizontal;
		
		auto splitter = new QSplitter(orientation);
		splitter->addWidget(tabWidget);
		
		int mainSplitterIndex;
		QSplitter *mainSplitter;
		switch (area.dir)
		{
			case Left:
				mainSplitterIndex = area.splitterIndex;
				mainSplitter = _mainHorizontalSplitter;
				break;
			case Right:
				mainSplitterIndex = _mainHorizontalSplitter->count() - area.splitterIndex;
				mainSplitter = _mainHorizontalSplitter;
				break;
			case Top:
				mainSplitterIndex = area.splitterIndex;
				mainSplitter = _mainVerticalSplitter;
				break;
			case Bottom:
				mainSplitterIndex = _mainVerticalSplitter->count() - area.splitterIndex;
				mainSplitter = _mainVerticalSplitter;
				break;
			default:
				Q_ASSERT(0);
				break;
		}
		
		mainSplitter->insertWidget(mainSplitterIndex, splitter);
		mainSplitter->setStretchFactor(mainSplitterIndex, 0);
		
		_splitterLists[area.dir].insert(area.splitterIndex, splitter);
	}
	else
	{
		QSplitter *splitter = _splitterLists[area.dir].at(area.splitterIndex);
		splitter->insertWidget(area.tabWidgetIndex, tabWidget);
	}
	return true;
}

int DockTabMotherWidget::tabWidgetCount(Direction dir, int splitterIndex)
{
	return _splitterLists[dir].at(splitterIndex)->count();
}

int DockTabMotherWidget::splitterCount(Direction dir)
{
	return _splitterLists[dir].size();
}

bool DockTabMotherWidget::dropTab(DockTabWidget *tabWidget, int index, const QPoint &pos)
{
	TabWidgetArea area = dropArea(pos);
	
	if (!area.isValid())
		return false;
	
	auto newTabWidget = tabWidget->createAnother();
	DockTabWidget::moveTab(tabWidget, index, newTabWidget, 0);
	
	if (insertTabWidget(newTabWidget, area))
	{
		connect(newTabWidget, SIGNAL(willBeAutomaticallyDeleted(DockTabWidget*)), this, SLOT(onTabWidgetWillBeDeleted(DockTabWidget*)));
		return true;
	}
	else
	{
		newTabWidget->deleteLater();
		return false;
	}
}

DockTabMotherWidget::TabWidgetArea DockTabMotherWidget::dropArea(const QPoint &pos)
{
	for (Direction dir = Left; dir != Bottom; ++dir)
	{
		if (_splitterLists[dir].size() == 0)
		{
			int distFromSide;
			switch (dir)
			{
				default:
				case Left:
					distFromSide = pos.x();
					break;
				case Right:
					distFromSide = width() - pos.x();
					break;
				case Top:
					distFromSide = pos.y();
					break;
				case Bottom:
					distFromSide = height() - pos.y();
					break;
			}
			
			if (distFromSide < insertDistance())
			{
				return TabWidgetArea(dir, 0, -1);
			}
		}
		
		QRect rect = splittersRect(dir).adjusted(-insertDistance(), -insertDistance(), insertDistance(), insertDistance());
		if (rect.contains(pos))
		{
			TabWidgetArea area = dropAreaAt(pos, dir);
			if (area.isValid())
				return area;
		}
	}
	
	return TabWidgetArea();
}

DockTabMotherWidget::TabWidgetArea DockTabMotherWidget::dropAreaAt(const QPoint &pos, Direction dir)
{
	int countSplitter = _splitterLists[dir].size();
	
	for (int indexSplitter = 0; indexSplitter < countSplitter; ++indexSplitter)
	{
		QSplitter *splitter = _splitterLists[dir].at(indexSplitter);
		
		int countTabWidget = splitter->count();
		
		for (int indexTabWidget = 0; indexTabWidget < countTabWidget; ++indexTabWidget)
		{
			InsertionDirection insertionDir;
			
			if (getInsertionDirection(pos, splitter->widget(indexTabWidget), dir, insertionDir))
			{
				switch (insertionDir)
				{
					case NextSplitter:
						return TabWidgetArea(dir, indexSplitter + 1, -1);
					case PreviousSplitter:
						return TabWidgetArea(dir, indexSplitter, -1);
					case Next:
						return TabWidgetArea(dir, indexSplitter, indexTabWidget + 1);
					case Previous:
						return TabWidgetArea(dir, indexSplitter, indexTabWidget);
					default:
						break;
				}
			}
		}
	}
	return TabWidgetArea();
}

bool DockTabMotherWidget::getInsertionDirection(const QPoint &pos, QWidget *widget, Direction dockDir, InsertionDirection &insertDir)
{
	auto insertionFromAbsoluteDir = [dockDir](Direction absDir)->InsertionDirection
	{
		switch (dockDir)
		{
			default:
			case Left:
				switch (absDir)
				{
					default:
					case Left:
						return PreviousSplitter;
					case Right:
						return NextSplitter;
					case Top:
						return Previous;
					case Bottom:
						return Next;
				}
			case Right:
				switch (absDir)
				{
					default:
					case Left:
						return NextSplitter;
					case Right:
						return PreviousSplitter;
					case Top:
						return Previous;
					case Bottom:
						return Next;
				}
			case Top:
				switch (absDir)
				{
					default:
					case Left:
						return Previous;
					case Right:
						return Next;
					case Top:
						return PreviousSplitter;
					case Bottom:
						return NextSplitter;
				}
			case Bottom:
				switch (absDir)
				{
					default:
					case Left:
						return Previous;
					case Right:
						return Next;
					case Top:
						return NextSplitter;
					case Bottom:
						return PreviousSplitter;
				}
		}
	};
	
	auto isInInsertDist = [](int x, int border)->bool
	{
		return border - insertDistance() <= x && x < border + insertDistance();
	};
	
	QRect geom = widget->geometry();
	geom.moveTopLeft(mapToAncestor(this, widget, geom.topLeft()));
	int left = geom.left();
	int rightEnd = geom.left() + geom.width();
	int top = geom.top();
	int bottomEnd = geom.top() + geom.height();
	
	int x = pos.x();
	int y = pos.y();
	
	if (left <= x && x < rightEnd)
	{
		if (isInInsertDist(top, y))
		{
			insertDir = insertionFromAbsoluteDir(Top);
			return true;
		}
		
		if (isInInsertDist(bottomEnd, y))
		{
			insertDir = insertionFromAbsoluteDir(Bottom);
			return true;
		}
	}
	
	if (top <= y && y < bottomEnd)
	{
		if (isInInsertDist(left, x))
		{
			insertDir = insertionFromAbsoluteDir(Left);
			return true;
		}
		
		if (isInInsertDist(rightEnd, x))
		{
			insertDir = insertionFromAbsoluteDir(Right);
			return true;
		}
	}
	
	return false;
}

QRect DockTabMotherWidget::splittersRect(Direction dir)
{
	QRect rect;
	for (QSplitter *splitter : _splitterLists[dir])
		rect |= splitter->geometry();
	
	return rect;
}

void DockTabMotherWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (DockTabWidget::eventIsTabDrag(event))
		event->acceptProposedAction();
}

void DockTabMotherWidget::dropEvent(QDropEvent *event)
{
	DockTabWidget *srcWidget;
	int srcIndex;
	DockTabWidget::decodeTabDropEvent(event, &srcWidget, &srcIndex);
	if (srcWidget)
	{
		if (dropTab(srcWidget, srcIndex, event->pos()))
		{
			event->acceptProposedAction();
			return;
		}
	}
	event->setDropAction(Qt::IgnoreAction);
}

void DockTabMotherWidget::onTabWidgetWillBeDeleted(DockTabWidget *widget)
{
	for (QSplitterList &splitters : _splitterLists)
	{
		for (QSplitter *splitter : splitters)
		{
			if (splitter->count() == 1 && splitter->widget(0) == widget)
			{
				splitters.removeAll(splitter);
				splitter->deleteLater();
			}
		}
	}
}

/*
DockTabMotherWidget::DockTabMotherWidget(QWidget *parent) :
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

void DockTabMotherWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (DockTabWidget::eventIsTabDrag(event))
		event->acceptProposedAction();
}

void DockTabMotherWidget::dropEvent(QDropEvent *event)
{
	DockTabWidget *srcWidget;
	int srcIndex;
	DockTabWidget::decodeTabDropEvent(event, &srcWidget, &srcIndex);
	if (srcWidget)
	{
		if (dropTab(srcWidget, srcIndex, event->pos()))
		{
			event->acceptProposedAction();
			return;
		}
	}
	event->setDropAction(Qt::IgnoreAction);
	event->acceptProposedAction();
}

void DockTabMotherWidget::onTabWidgetWillBeDeleted(DockTabWidget *widget)
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

bool DockTabMotherWidget::dropTab(DockTabWidget *srcTabWidget, int srcIndex, const QPoint &pos)
{
	TabHandling tabHandling;
	Direction dir;
	int columnIndex;
	
	getTabHandling(pos, tabHandling, dir, columnIndex);
	
	if (tabHandling == NoHandling)
		return false;
	
	DockTabWidget *dstTabWidget = new DockTabWidget;
	DockTabWidget::moveTab(srcTabWidget, srcIndex, dstTabWidget, 0);
	
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
	
	connect(dstTabWidget, SIGNAL(willBeDeleted(DockTabWidget*)), this, SLOT(onTabWidgetWillBeDeleted(DockTabWidget*)));
	return true;
}

DockTabMotherWidget::Direction DockTabMotherWidget::insertionDirection(QSplitter *splitter, const QPoint &pos)
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

void DockTabMotherWidget::getTabHandling(const QPoint &dropPos, TabHandling &handling, Direction &columnDirection, int &index)
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
}*/

