#include <QtGui>
#include "draggabletabwidget.h"
#include "motherwidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	DockTabWidget *tabWidget = new DockTabWidget;
	
	tabWidget->addTab(new QLabel("tab1"), "tab1");
	tabWidget->addTab(new QLabel("tab2"), "tab2");
	tabWidget->addTab(new QLabel("tab3"), "tab3");
	
	tabWidget->show();
	
	DockTabMotherWidget *motherWidget = new DockTabMotherWidget;
	motherWidget->resize(400, 300);
	motherWidget->show();
	motherWidget->setStyleSheet("QSplitter::handle { background:darkGray; } QSplitter::handle:horizontal { width:1px; } QSplitter::handle:vertical { height:1px; }");
	
	return a.exec();
}
