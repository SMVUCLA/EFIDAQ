#ifndef MAINRUNTEST_H
#define MAINRUNTEST_H

#include <QMainWindow>

namespace Ui {
class MAINRUNTEST;
}

class RUNTEST;

class MAINRUNTEST : public QMainWindow
{
    Q_OBJECT

public:
    MAINRUNTEST(QWidget *parent = 0);
    ~MAINRUNTEST();

    void closeEvent(QCloseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void setResizeable(bool resizeable);
    bool isFilteringByNumFields() const;
    bool isFilteringByContent() const;
    int collectionMethod() const;
    bool isShowingValues() const;
    bool isAutoScrolling() const;

private slots:
    bool saveData();
    bool clearData();
    void dataLockChanged(bool checked);
    void handleParametersEditTriggered();
    void handleProtocolSerialBlocksTriggered(bool);
    void handleProtocolUDPPacketsTriggered(bool);
    void handleActionCommand_Window_triggered();

private:
    Ui::MAINRUNTEST *ui;

    RUNTEST* rt;
};

#endif // MAINRUNTEST_H
