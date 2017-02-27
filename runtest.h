#ifndef RUNTEST_H
#define RUNTEST_H

#include <QWidget>
#include <QVector>
#include <QByteArray>
#include <QTimer>

#include "tmodels.h"
#include "utilities.h"
#include "datainterpreter.h"
#include "signals.h"

namespace Ui {
class RUNTEST;
}

class PlotWindow;
class SERIALREADER;
class UDPReader;
class AFRTABLE;
class Highlighter;
class MAINRUNTEST;

class RUNTEST : public QWidget
{
    Q_OBJECT

public:
    RUNTEST(MAINRUNTEST* mrtparent, QWidget *parent = 0);
    ~RUNTEST();

    int saveData();
    int clearData();
    void setDataLocked(bool yes = true);
    void stopPlotting(PlotWindow* sender);
    bool isCollectingData() const;
    Signals* getTransceiver();

public slots:
    void hitDataTimer();
    void xItemChanged(QModelIndex xindex);
    void yItemChanged(QModelIndex yindex);
    void on_StartDCButton_clicked();
    void on_EndDCButton_clicked();
    void on_OpenAFRTableButton_clicked();
    void on_PlotDataButton_clicked();
    void on_setSerialPortButton_clicked();
    void handleIPAddressEditFinished();
    void handlePortNoEditFinished();
    void handleStreamRefrTimeout();

private slots:
    void on_sampleRateSlider_sliderMoved(int position);
    void on_sampleRateEdit_editingFinished();

private:
    void addData();

    Ui::RUNTEST *ui;
    LIST_CHOICES_MODEL* m_xlmodel;
    LIST_CHOICES_MODEL* m_ylmodel;
    AFRTABLE* m_afrtable;

    Highlighter* highlighter;

    QTimer* m_dataRefrTimer;
    QTimer m_streamRefrTimer;
    unsigned long long int m_ndp;
    QVector<PlotWindow*> pw;
    SERIALREADER* m_serialReader;
    Signals* transceiver;

    UDPReader* m_udpReader;
    QByteArray* m_bytebuf;
    int expectedNumFields;
    ByteFilter filter;

    MAINRUNTEST* mrtparent;

    QVector<QVector<double>> m_xData;
    QVector<QVector<double>> m_yData;
    std::pair<QString, int> m_xLabel;
    std::pair<QString, int> m_yLabel;

    QByteArray buffer;
};

#endif // RUNTEST_H
