#include "QMapPropsDialog.h"
#include "ui_QMapPropsDialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>

QMapPropsDialog::QMapPropsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QMapPropsDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(readProps()));
}

QMapPropsDialog::~QMapPropsDialog()
{
    delete ui;
}

const QVariantMap &QMapPropsDialog::props() const
{
    return m_props;
}

void QMapPropsDialog::setProps(const QVariantMap &props)
{
    m_props = props;
    QFont appFont(font());
    appFont.setPixelSize(16);
    for
    (
        QVariantMap::ConstIterator it = props.cbegin();
        it != props.cend();
        ++it
    )
    {
        QHBoxLayout * box = new QHBoxLayout();
        QLabel * l = new QLabel(it.key());
        l->setFont(appFont);
        box->addWidget(l);
        switch(it.value().type())
        {
        case QVariant::Double:
        {
            QDoubleSpinBox * sb = new QDoubleSpinBox;
            sb->setValue(it.value().toDouble());
            sb->setRange(-1.e9, 1.e9);
            box->addWidget(sb);
            m_widgets.push_back(sb);
            sb->setFont(appFont);
            break;
        }
        case QVariant::Int:
        {
            QSpinBox * sb = new QSpinBox;
            sb->setValue(it.value().toInt());
            sb->setRange(0, 1000000000);
            box->addWidget(sb);
            m_widgets.push_back(sb);
            sb->setFont(appFont);
            break;
        }
        default:
            QTextEdit * txt = new QTextEdit;
            txt->setText(it.value().toString());
            box->addWidget(txt);
            m_widgets.push_back(txt);
            txt->setFont(appFont);
            break;
        }
        ui->verticalLayout->insertLayout(0, box);
    }
    adjustSize();
    update();
}

void QMapPropsDialog::readProps()
{
    QVariantMap::Iterator it = m_props.begin();
    for(QObject* w: m_widgets)
    {
        switch(it.value().type())
        {
        case QVariant::Double:
        {
            it.value().setValue<double>
            (
                qobject_cast<QDoubleSpinBox*>(w)->value()
            );
            break;
        }
        case QVariant::Int:
        {
            it.value().setValue<int>
            (
                qobject_cast<QSpinBox*>(w)->value()
            );
            break;
        }
        default:
            it.value().setValue<QString>
            (
                qobject_cast<QTextEdit*>(w)->toPlainText()
            );
            break;
        }
        ++it;
    }
}
