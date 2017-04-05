#include "qformdialog.h"
#include "ui_qformdialog.h"
#include "utilities.h"

#include <QSet>
#include <QLabel>
#include <QLineEdit>

QFormDialog::QFormDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QFormDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DEFAULT_LOGO_FILEPATH));

    connect(ui->okButton, SIGNAL(clicked(bool)), SLOT(handleOkButtonClicked(bool)));
    connect(ui->cancelButton, SIGNAL(clicked(bool)), SLOT(handleCancelButtonClicked(bool)));
}

QFormDialog::~QFormDialog()
{
    delete ui;
    for (auto it = m_deletionList.begin(); it != m_deletionList.end(); it++)
    {
        delete it->first;
        delete it->second;
    }
}

void QFormDialog::addPair(QString label, QString startVal)
{
    std::pair<QString, QString> pairIn;
    pairIn.first = label;
    pairIn.second = startVal;
    m_pairs.insert(pairIn);

    QLabel* qlabel = new QLabel(label, this);
    qlabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QLineEdit* qlineEdit = new QLineEdit(startVal, this);
    qlineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    std::pair<QLabel*, QLineEdit*> widgets;
    widgets.first = qlabel;
    widgets.second = qlineEdit;
    m_deletionList.push_back(widgets);

    ui->formLayout->addRow(qlabel, qlineEdit);
}

void QFormDialog::addPair(std::pair<QString, QString> pair)
{
    addPair(pair.first, pair.second);
}

QSet<std::pair<QString, QString>> QFormDialog::getPairs() const
{
    return m_pairs;
}

void QFormDialog::handleOkButtonClicked(bool)
{
    QSet<std::pair<QString, QString>> pairs;
    for (auto it = m_deletionList.begin(); it != m_deletionList.end(); it++)
    {
        std::pair<QString, QString> pair;
        pair.first = it->first->text();
        pair.second = it->second->text();
        pairs.insert(pair);
    }
    m_pairs = pairs;
    accept();
}

void QFormDialog::handleCancelButtonClicked(bool)
{
    reject();
}
