 #include <QApplication>

 #include "display.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	AT3DVIEW view;
	view.show();
	app.exec(); //we shouldn't use return app.exec() here
	return 0;
}


