#ifndef __V3D_SIMPLE_PARAMETER_DIALOG_H__
#define __V3D_SIMPLE_PARAMETER_DIALOG_H__

#include <QtGui>

struct _simpleQtInputLine
{
	QString labelText;
	void * qtObj;
	_simpleQtInputLine() {labelText=""; qtObj=0;}
	~_simpleQtInputLine() {if (qtObj) {delete qtObj; qtObj=0;} }
};

class V3DSimpleParameterDialog: QDialog
{
public:
    enum RowInputMode {
        TextInput,
        IntInput,
        DoubleInput,
		BoolInput,
		ComboInput
    };
	
	bool addInputRow(QString & labelText, RowInputMode m, QVariant & v)
	{
		_simpleQtInputLine s;
		switch (m)
		{
			case TextInput:
				s.labelText = labelText;
				s.qtObj = (void *) new Q
				break;
				
			case IntInput:
				break;

			case DoubleInput:
				s.labelText = labelText;
				s.qtObj = (void *) new QComboBox(this);
				((QComboBox *)s.qtObj)->addItems( v.toStringList() );
				break;
				
			case BoolInput:
				s.labelText = labelText;
				s.qtObj = (void *) new QCheckBox(labelText, this);
				((QCheckBox *)s.qtObj)->setCheckState( (v.toBool() ) ? Qt::Checked : Qt::Unchecked() );
				break;
				
			case ComboInput:
				s.labelText = labelText;
				s.qtObj = (void *) new QComboBox(this);
				((QComboBox *)s.qtObj)->addItems( v.toStringList() );
				break;
				
			default:
				return false;
				break;
		}
		return false;
	}
	
	QVariant getValue(QString & variableName)
	{
	}

protected:
	QList <_simpleQtInputLine> entryList;
	QMap <QString, _simpleQtInputLine> map;
};

#endif

