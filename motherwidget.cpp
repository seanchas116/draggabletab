#include <QtGui>

#include "draggabletabwidget.h"

#include "motherwidget.h"

MotherWidget::MotherWidget(QWidget *parent) :
	QWidget(parent),
	_leftWidgetCount(0)
{
	_splitter = new QSplitter(Qt::Horizontal);
	
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(_splitter);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
	
	_leftSplitter = new QSplitter(Qt::Vertical);
	_leftSplitter->setVisible(false);
	
	_centralWidget = new QWidget;
	
	_splitter->addWidget(_leftSplitter);
	_splitter->addWidget(_centralWidget);
	
	setAcceptDrops(true);
}

void MotherWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (DraggableTabWidget::eventIsTabDrag(event))
		event->acceptProposedAction();
}

void MotherWidget::dropEvent(QDropEvent *event)
{
	// drop tab
	
	DraggableTabWidget::TabInfo tabInfo = DraggableTabWidget::decodeTabDropEvent(event);
	if (!tabInfo.tabWidget)
		return;
	
	enum TabHandling
	{
		NoHandling,
		TabAppend,
		TabPrepend,
		NewColumn
	} tabHandling = NoHandling;
	
	if (_leftSplitter->isVisible())
	{
		QRect prependRect(0, 0, _leftSplitter->width(), InsertDistance);
		QRect appendRect(0, height() - InsertDistance, _leftSplitter->width(), InsertDistance);
		
		if (prependRect.contains(event->pos()))
			tabHandling = TabPrepend;
		
		if (appendRect.contains(event->pos()))
			tabHandling = TabAppend;
	}
	else
	{
		QRect insertRect(0, 0, InsertDistance, height());
		if (insertRect.contains(event->pos()))
			tabHandling = NewColumn;
	}
	
	if (tabHandling == NoHandling)
		return;
	
	DraggableTabWidget *tabWidget = new DraggableTabWidget;
	DraggableTabWidget::moveTab(tabInfo.tabWidget, tabInfo.index, tabWidget, 0);
	
	switch (tabHandling)
	{
		case TabPrepend:
		{
			_leftSplitter->insertWidget(0, tabWidget);
			break;
		}
		case TabAppend:
		{
			_leftSplitter->addWidget(tabWidget);
			break;
		}
		case NewColumn:
		{
			_leftSplitter->addWidget(tabWidget);
			_leftSplitter->setVisible(true);
			break;
		}
		default:
		{
			tabWidget->deleteLater();
			return;
		}
	}
	
	_leftWidgetCount++;
	connect(tabWidget, SIGNAL(willBeDeleted()), this, SLOT(onTabWidgetWillBeDeleted()));
	event->acceptProposedAction();
}

void MotherWidget::onTabWidgetWillBeDeleted()
{
	if (_leftWidgetCount)
		_leftWidgetCount--;
	
	if (_leftWidgetCount == 0)
		_leftSplitter->setVisible(false);
}
