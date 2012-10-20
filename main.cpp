#include <QtGui>
#include "draggabletabwidget.h"
#include "motherwidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	DraggableTabWidget *tabWidget = new DraggableTabWidget;
	
	tabWidget->addTab(new QLabel("tab1"), "tab1");
	tabWidget->addTab(new QLabel("tab2"), "tab2");
	tabWidget->addTab(new QLabel("tab3"), "tab3");
	
	tabWidget->show();
	
	MotherWidget *motherWidget = new MotherWidget;
	motherWidget->resize(400, 300);
	motherWidget->show();
	
	return a.exec();
}
