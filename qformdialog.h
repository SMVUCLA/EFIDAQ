#ifndef QFORMDIALOG_H
#define QFORMDIALOG_H

#include "serialreader.h"

#include <QDialog>
#include <QString>
#include <QVector>
#include <QSet>

namespace Ui {
class QFormDialog;
}

class QLabel;
class QLineEdit;

class QFormDialog : public QDialog
{
    Q_OBJECT

public:
    QFormDialog(QWidget *parent = 0, SERIALREADER* serialport= nullptr);
    ~QFormDialog();

    void addPair(QString label, QString startVal = QString());
    void addPair(std::pair<QString, QString> pair);
    QSet<std::pair<QString, QString>> getPairs() const;
    QVector<QString> getArduinoValues() const;

private slots:
    void handleUpdateButtonClicked(bool clicked);
    void handleCancelButtonClicked(bool clicked);

private:
    Ui::QFormDialog* ui;
    QSet<std::pair<QString, QString>> m_pairs;
    QList<std::pair<QLabel*, QLineEdit*>> m_deletionList;
    SERIALREADER* m_serialport;
    QVector<QString> m_arduinoValues;
};

#endif // QFORMDIALOG_H
