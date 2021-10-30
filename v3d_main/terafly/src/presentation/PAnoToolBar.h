#ifndef PANOTOOLBAR_H
#define PANOTOOLBAR_H

#include "theader.h"

#include "../control/CPlugin.h"

class terafly::PAnoToolBar : public QWidget
{
    Q_OBJECT

    private:

        QToolBar* toolBar;                      //tool bar with buttons
        QToolButton *buttonMarkerCreate;        //button1
        QToolButton *buttonMarkerCreate2;       //button1
        QToolButton *buttonMarkerDelete;        //button2
        QToolButton *buttonMarkerRoiDelete;     //button3
        QToolButton *buttonMarkerRoiView;       //button4
		QToolButton* buttonFragmentTrace;       //button5 --> new button added for fragmented auto-tracing feature, MK, Dec 2018

        QToolButton *buttonUndo;                //undo button
        QToolButton *buttonRedo;                //redo button

        QToolButton* buttonOptions;             //options button

        static const int autosaveInterval = 300000;     // save annotations every 'autosaveInterval' ms
        QTimer autosaveTimer;                   // timer for autosave function

        QPoint dragPosition;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PAnoToolBar* uniqueInstance;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "instance(...)"
        **********************************************************************************/
        static PAnoToolBar* instance(QWidget *parent=0)
        {
            if (uniqueInstance == 0)
                uniqueInstance = new PAnoToolBar(parent);
            return uniqueInstance;
        }
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        static bool isInstantiated(){return uninstance != 0;}
        PAnoToolBar(QWidget *parent = 0);
        ~PAnoToolBar();

        /**********************************************************************************
        * Intercepts global key pressed events
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);

        /**********************************************************************************
        * Release currently activated tools, if any
        ***********************************************************************************/
        void releaseTools();

        /**********************************************************************************
        * Refresh currently activated tools, if any
        ***********************************************************************************/
        void refreshTools();

        friend class PMain;
        friend class CViewer;
        friend class QUndoMarkerCreate;
        friend class QUndoMarkerDelete;
        friend class QUndoMarkerDeleteROI;
        friend class TeraFly;

    
    signals:
    
    public slots:

        void buttonMarkerCreateChecked(bool checked);
        void buttonMarkerCreate2Checked(bool checked);
        void buttonMarkerDeleteChecked(bool checked);
        void buttonMarkerRoiDeleteChecked(bool checked);
        void buttonMarkerRoiViewChecked(bool checked);
		void buttonFragmenTraceChecked(bool checked); // MK, Dec 2018
        void buttonUndoClicked();
        void buttonRedoClicked();

        void saveAnnotations();
        void autosaveAnnotations();

    
};

#endif // PANOTOOLBAR_H
