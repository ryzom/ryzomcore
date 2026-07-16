#ifndef LIBQTPROPERTYMANAGER_TEXT_EDIT_WIDGET_H
#define LIBQTPROPERTYMANAGER_TEXT_EDIT_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtTextEditWidget : public QWidget
{
	Q_OBJECT

public:
	QtTextEditWidget(QWidget *parent);

	bool eventFilter(QObject *obj, QEvent *ev);

public Q_SLOTS:
	void setValue(const QString &value);
	void setStateResetButton(bool enabled);

private Q_SLOTS:
	void buttonClicked();

Q_SIGNALS:
	void valueChanged(const QString &value);
	void resetProperty();

private:
	QLineEdit *m_lineEdit;
	QToolButton *m_defaultButton;
	QToolButton *m_button;
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif // LIBQTPROPERTYMANAGER_TEXT_EDIT_WIDGET_H
