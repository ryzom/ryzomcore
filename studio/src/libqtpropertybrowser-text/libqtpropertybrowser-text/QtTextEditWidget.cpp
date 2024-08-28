#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QToolButton>

#include <libqtpropertybrowser-text/QtTextEditWidget.h>

#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif


QtTextEditWidget::QtTextEditWidget(QWidget *parent) :
    QWidget(parent),
    m_lineEdit(new QLineEdit),
    m_defaultButton(new QToolButton),
    m_button(new QToolButton)
{
    QHBoxLayout *lt = new QHBoxLayout(this);
    lt->setContentsMargins(0, 0, 0, 0);
    lt->setSpacing(0);
    lt->addWidget(m_lineEdit);
    m_lineEdit->setReadOnly(true);

    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    m_button->setText(tr("..."));
    m_button->installEventFilter(this);

    setFocusProxy(m_button);
    setFocusPolicy(m_button->focusPolicy());

    m_defaultButton->setIcon(QIcon(":/trolltech/qtpropertybrowser/images/resetproperty.png"));
    m_defaultButton->setMaximumWidth(16);

    connect(m_button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(m_defaultButton, SIGNAL(clicked()), this, SIGNAL(resetProperty()));
    lt->addWidget(m_button);
    lt->addWidget(m_defaultButton);
    m_defaultButton->setEnabled(false);
}

void QtTextEditWidget::setValue(const QString &value)
{
    if (m_lineEdit->text() != value)
        m_lineEdit->setText(value);
}

void QtTextEditWidget::setStateResetButton(bool enabled)
{
    m_defaultButton->setEnabled(enabled);
}

void QtTextEditWidget::buttonClicked()
{
    QGridLayout *gridLayout;
    QPlainTextEdit *plainTextEdit;
    QDialogButtonBox *buttonBox;
    QDialog *dialog;

    dialog = new QDialog(this);
    dialog->resize(400, 300);
    gridLayout = new QGridLayout(dialog);
    plainTextEdit = new QPlainTextEdit(dialog);

    gridLayout->addWidget(plainTextEdit, 0, 0, 1, 1);

    buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    gridLayout->addWidget(buttonBox, 1, 0, 1, 1);

    QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    plainTextEdit->textCursor().insertText(m_lineEdit->text());

    dialog->setModal(true);
    dialog->show();
    int result = dialog->exec();

    if (result == QDialog::Accepted)
    {
        QString newText = plainTextEdit->document()->toPlainText();

        setValue(newText);
        if (plainTextEdit->document()->isModified())
            Q_EMIT valueChanged(newText);
    }

    delete dialog;
}

bool QtTextEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_button) {
        switch (ev->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
            switch (static_cast<const QKeyEvent*>(ev)->key()) {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->ignore();
                return true;
            default:
                break;
            }
        }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif
