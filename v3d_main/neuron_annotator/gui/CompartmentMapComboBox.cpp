#include "CompartmentMapComboBox.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QStylePainter>

// item delegate
class CompartmentMapComboBoxDelegate : public QItemDelegate
{
public:
    CompartmentMapComboBoxDelegate(QObject *parent) : QItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        //Get item data
        bool value = index.data(Qt::UserRole).toBool();
        QString text = index.data(Qt::DisplayRole).toString();

        // fill style options with item data
        const QStyle *style = QApplication::style();
        QStyleOptionButton opt;
        opt.state |= value ? QStyle::State_On : QStyle::State_Off;
        opt.state |= QStyle::State_Enabled;
        opt.text = text;
        opt.rect = option.rect;

        // draw item data as CheckBox
        style->drawControl(QStyle::CE_CheckBox, &opt, painter);
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // create check box as our editor
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        //set editor data
        QCheckBox *myEditor = static_cast<QCheckBox*>(editor);
        myEditor->setText(index.data(Qt::DisplayRole).toString());
        myEditor->setChecked(index.data(Qt::UserRole).toBool());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        //get the value from the editor (CheckBox)
        QCheckBox *myEditor = static_cast<QCheckBox*>(editor);
        bool value = myEditor->isChecked();

        //set model data
        QMap<int,QVariant> data;
        data.insert(Qt::DisplayRole,myEditor->text());
        data.insert(Qt::UserRole,value);
        model->setItemData(index,data);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }
};

//
CompartmentMapComboBox::CompartmentMapComboBox(QWidget *widget ): QComboBox(widget),m_DisplayText("CompartmentMap")
{
    // set delegate items view
    view()->setItemDelegate(new CompartmentMapComboBoxDelegate(this));

    // Enable editing on items view
    view()->setEditTriggers(QAbstractItemView::CurrentChanged);

    // set event filter for items view
    view()->viewport()->installEventFilter(this);

    // defualt
    view()->setAlternatingRowColors(true);
    indexSelected = -1; // no selection

}


CompartmentMapComboBox::~CompartmentMapComboBox()
{}

bool CompartmentMapComboBox::eventFilter(QObject *object, QEvent *event)
{
    if(object==view()->viewport())
    {
        QMouseEvent *m = static_cast<QMouseEvent *>(event);

        if(event->type() == QEvent::MouseButtonRelease
           && view()->rect().contains(m->pos())
           && view()->currentIndex().isValid()
           && (view()->currentIndex().flags() & Qt::ItemIsEnabled)
           && m->button() == Qt::LeftButton)
        {
            indexSelected = view()->currentIndex().row();
        }

        if(event->type() == QEvent::MouseButtonRelease
           && view()->rect().contains(m->pos())
           && view()->currentIndex().isValid()
           && (view()->currentIndex().flags() & Qt::ItemIsEnabled)
           && m->button() == Qt::LeftButton)
        {
            if(view()->currentIndex().row() == indexSelected)
            {
                emit currentIndexChanged(view()->currentIndex().row());
            }
            return true;
        }
    }

    return QComboBox::eventFilter(object,event);
}


void CompartmentMapComboBox::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    // if no display text been set , use "..." as default
    if(m_DisplayText.isNull())
        opt.currentText = "...";
    else
        opt.currentText = m_DisplayText;

    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void CompartmentMapComboBox::wheelEvent(QWheelEvent *e)
{
}


void CompartmentMapComboBox::SetDisplayText(QString text)
{
    m_DisplayText = text;
}

QString CompartmentMapComboBox::GetDisplayText() const
{
    return m_DisplayText;
}
