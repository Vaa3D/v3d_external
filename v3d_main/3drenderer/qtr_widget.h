/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/***************************************************************
 * qtr_widget.h
 * 2009-08-05 original created and collected by Zongcai Ruan
 * *************************************************************
 * include:
 * 	AutoTabBar
 *  AutoTabWidget
 *  HighlightDial
 *  SliderTipFilter
 *  SharedDialog
 *
*/
//autotab.h 2008-11-23 separated by Hanchuan Peng from v3dr_mainwindow.h
//

#ifndef __QTR_WIDGET_H__
#define __QTR_WIDGET_H__


#ifdef Q_WS_X11
#define QEVENT_KEY_PRESS 6 //for crazy RedHat error: expected unqualified-id before numeric constant
#else
#define QEVENT_KEY_PRESS QEvent::KeyPress
#endif


//---------------------------------------------------

class AutoTabBar : public QTabBar
{
public:
	AutoTabBar(QWidget * parent = 0) : QTabBar (parent)
	{
		setMouseTracking(true);
	}

protected:
	virtual void mouseMoveEvent(QMouseEvent *e)
	{
		int i = tabAt(e->pos());
		if (i>=0)  setCurrentIndex(i);
		//qDebug() << e << i;
	}
};


class AutoTabWidget : public QTabWidget
{
public:
	AutoTabWidget(QWidget * parent = 0) : QTabWidget (parent)
	{
		QTabBar* tabbar = new AutoTabBar(parent);
		setTabBar(tabbar);
	}
};


//---------------------------------------------------

class HighlightDial : public QDial
{
public:
	HighlightDial(QWidget * parent = 0) : QDial (parent)
	{
		QPalette pe = this->palette();
		pe.setBrush(QPalette::Button, pe.highlight());
		setPalette(pe);
	}
};


//---------------------------------------------------
typedef QString (* FUNC_altTip) (QWidget* parent, int v, int minv, int maxv, int offset);

class SliderTipFilter : public QObject
{
	//Q_OBJECT
public:
	SliderTipFilter(QObject* parent, QString prefix="", QString surffix="", int offset=0, FUNC_altTip func_alt_tip=0)
		: QObject (parent)
	{
		w = (QWidget*)parent;
		this->prefix = prefix;
		this->surffix = surffix;
		this->offset = offset;
		this->func_alt_tip = func_alt_tip;
	}
protected:
	QWidget* w;
	QString prefix, surffix;
	int offset;
	FUNC_altTip func_alt_tip;
    bool eventFilter(QObject *obj, QEvent *e)
	{
		QAbstractSlider* slider = (QAbstractSlider*)obj;
		if (slider)	{
			slider->setAttribute(Qt::WA_Hover); // this control the QEvent::ToolTip and QEvent::HoverMove
			slider->setFocusPolicy(Qt::WheelFocus); // accept KeyPressEvent when mouse wheel move
		}

		bool event_tip = false;
		QPoint pos(0,0);
		switch (e->type())
		{
		case QEvent::ToolTip: // must turned on by setAttribute(Qt::WA_Hover) under Mac 64bit
//			qDebug("QEvent::ToolTip in SliderTipFilter");
			pos = ((QHelpEvent*)e)->pos(); //globalPos();
			event_tip = true;
			break;
//		case QEvent::HoverMove:
//			qDebug("QEvent::HoverMove in SliderTipFilter");
//			pos = ((QHoverEvent*)e)->pos();
//			event_tip = true;
//			break;
		case QEvent::MouseMove: // for mouse dragging
//			qDebug("QEvent::MouseMove in SliderTipFilter");
			pos = ((QMouseEvent*)e)->pos();
			event_tip = true;
			break;
		case QEVENT_KEY_PRESS: //QEvent::KeyPress: // for arrow key dragging
//			qDebug("QEvent::KeyPress in SliderTipFilter");
			if (slider) pos = slider->mapFromGlobal(slider->cursor().pos());
			event_tip = true;
			break;
		}
		if (event_tip && slider)
		{
			QPoint gpos = slider->mapToGlobal(pos);
			QString tip = QString(prefix + "%1(%2~%3)" + surffix)
							.arg(slider->value()+offset).arg(slider->minimum()+offset).arg(slider->maximum()+offset);
			if (func_alt_tip)
			{
				QString alt_tip = func_alt_tip(w, slider->value(), slider->minimum(), slider->maximum(), offset);
				tip += alt_tip;
			}
			QToolTip::showText(gpos, (tip), slider);
		}

		return QObject::eventFilter(obj, e);
	}
};

//---------------------------------------------------

class SharedDialog: public QWidget //QDialog
{
public:
	SharedDialog(QWidget* w, QWidget* parent=0)
		: QWidget(parent)
	//	: QDialog(parent)
	{
		setWindowFlags( Qt::Dialog
				//| Qt::Popup
				//| Qt::WindowStaysOnTopHint
				//| Qt::Tool
				);
		setAttribute(Qt::WA_MacAlwaysShowToolWindow); //for convenient screen shot on mac with Qt::Tool

		oldpos = (pos());
		widget = 0;
		reflist.clear();
		ref = 0;
		IncRef(w);///////////
	}

protected:
	QPoint oldpos;
	QWidget * widget;
	QList<QWidget*> reflist;
	int ref;

	virtual void closeEvent(QCloseEvent* e)
	{
		hide();
		e->ignore();
	}
	virtual void moveEvent(QMoveEvent* e)
	{
		oldpos = (e->pos());
	}
	virtual void showEvent(QShowEvent* e)
	{
		move(oldpos);
		if(widget) widget->activateWindow();;
	}
	virtual void mousePressEvent(QMouseEvent* e )
	{
		if (widget) widget->show();
	}

public slots:
	virtual int IncRef(QWidget* w)
	{
		if (w  //&& w != widget )
			&& !reflist.contains(w))
		{
			widget = w;
			reflist.append(w); ref = reflist.size();
			//++ref;
		}
		return ref;
	}
	virtual int DecRef(QWidget* w)
	{
		if (w //&& w == widget )
			&& reflist.contains(w))
		{
			reflist.removeOne(w); ref = reflist.size();
			//--ref;
			if (ref<1)
			{
				widget = 0;
				ref = 0;
				hide();
				deleteLater();
			}
		}
		return ref;
	}
	virtual void linkTo(QWidget* w) //link to new view
	{
		IncRef(w);//DecRef(w);//just update ref widget
	}


//	virtual void accept() { done(1); } // this only hide dialog
//	virtual void reject() { done(0); } // this only hide dialog
//	virtual void done(int);          // this really close/delete dialog
};


#endif


