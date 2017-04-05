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

    void closeEvent(QCloseEvent *event) override;
    void loadTable();
    bool saveTable();

private slots:
    // pushButton clicked handlers
    void handle_updateControllerButton_clicked();
    void handle_updateTableButton_clicked();

    // Menu action handlers
    void handle_actionSaveAs();
    void handle_actionLoad_Table();

private:
    Ui::AFRTABLE *ui;

    QVector<QVector<int>> requestedCells;
    QVector<QVector<float>> changedCellVals;

    AFR_TABLE_MODEL* m_tmodel;
    Signals* transceiver;

signals:
    void closing();
};

#endif // AFRTABLE_H
