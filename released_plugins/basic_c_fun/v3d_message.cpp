//by Hanchuan Peng
//090516

#include "v3d_message.h"

#include <stdio.h>

#include <QtGui>
#include <QString>

void v3d_msg(char *msg, bool b_disp_QTDialog)
{
	printf("%s\n", msg);
	if (b_disp_QTDialog)
		QMessageBox::information(0, "Information", msg);
}

void v3d_msg(const QString & msg, bool b_disp_QTDialog) //note that if I don't force (char *) conversion then there is a crash. noted by Hanchuan, 090516
{
	v3d_msg((char *)(qPrintable(msg)), b_disp_QTDialog);
}





