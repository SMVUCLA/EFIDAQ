#ifndef AFRTABLE_H
#define AFRTABLE_H

#include <QMainWindow>
#include <serialreader.h>

namespace Ui {
class AFRTABLE;
}

class AFR_TABLE_MODEL;
class Signals;

class AFRTABLE : public QMainWindow
{
    Q_OBJECT

public:
    AFRTABLE(QWidget *parent, Signals* tranceiver);
    ~AFRTABLE();

private slots:
    void loadTable();
    void handle_updateControllerButton_clicked();
    void handle_updateTableButton_clicked();

private:
    Ui::AFRTABLE *ui;

    QVector<QVector<int>> changedCells;
    QVector<QVector<float>> changedCellVals;

    AFR_TABLE_MODEL* m_tmodel;
    Signals* transceiver;
};

#endif // AFRTABLE_H
