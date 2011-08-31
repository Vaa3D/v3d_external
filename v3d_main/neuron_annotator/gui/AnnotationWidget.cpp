#include "AnnotationWidget.h"
#include "ui_AnnotationWidget.h"
#include "ontology_tree/TreeModel.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.h"
#include <QtGui>

AnnotationWidget::AnnotationWidget(QWidget *parent) : QFrame(parent), ui(new Ui::AnnotationWidget)
{
    ui->setupUi(this);
    ui->ontologyTreeTitle->setText(QString("Loading..."));
}

void AnnotationWidget::setOntology(Ontology *ontology) {

    // Clean up memory from previous ontology if necessary
    if (this->ontology != NULL) delete this->ontology;

    this->ontology = ontology;

    if (ontology->root() != NULL && ontology->root()->name != NULL) {
        ui->ontologyTreeTitle->setText(*ontology->root()->name);
    }
    else {
        ui->ontologyTreeTitle->setText("Error loading ontology");
    }

    TreeModel *treeModel = new TreeModel(ontology);
    ui->ontologyTreeView->setModel(treeModel);
    ui->ontologyTreeView->expandAll();
    ui->ontologyTreeView->setVisible(true);
    ui->ontologyTreeView->header()->moveSection(1,0);
    ui->ontologyTreeView->resizeColumnToContents(0);
    ui->ontologyTreeView->resizeColumnToContents(1);
}

AnnotationWidget::~AnnotationWidget()
{
    delete ui;
}

bool AnnotationWidget::eventFilter(QObject* watched_object, QEvent* event)
{
    bool filtered = false;
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int keyInt = keyEvent->key();
        Qt::Key key = static_cast<Qt::Key>(keyInt);

        // Qt doesn't understand this key code
        if (key == Qt::Key_unknown)
        {
            qDebug() << "Unknown key:"<<key;
            return false;
        }

        // Ignore this event if only a special control key was pressed
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta)
        {
            return false;
        }

        // Create key sequence
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        if (modifiers & Qt::ShiftModifier) keyInt += Qt::SHIFT;
        if (modifiers & Qt::ControlModifier) keyInt += Qt::CTRL;
        if (modifiers & Qt::AltModifier) keyInt += Qt::ALT;
        if (modifiers & Qt::MetaModifier) keyInt += Qt::META;
        QKeySequence keySeq(keyInt);
        qDebug() << "Key sequence:"<<keySeq.toString(QKeySequence::NativeText) << "Portable:"<< keySeq.toString(QKeySequence::PortableText);

        QMap<QKeySequence, qint64>::const_iterator i = ontology->keyBindMap()->constBegin();
        while (i != ontology->keyBindMap()->constEnd())
        {
            QKeySequence keyBind = i.key();
            if (keyBind.matches(keySeq) == QKeySequence::ExactMatch)
            {
                Entity *termEntity = ontology->getTermById(i.value());
                qDebug() << "  Annotate with:"<<(termEntity == NULL ? "" : *termEntity->name) << " (id="<< i.value()<< ")";
            }
            ++i;
        }

        filtered = true;
    }
    return filtered;
}
