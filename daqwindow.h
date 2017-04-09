#ifndef DAQWINDOW_H
#define DAQWINDOW_H

#include "listmodel.h"
#include "signals.h"
#include "afrtable.h"

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QTimer>
#include <QByteArray>

namespace Ui {
class DAQWindow;
}

class PlotWindow;
class SerialHandler;
class Highlighter;

const int MINIMUM_COLLECTION_REFRESH_TIME = 1;
const int MAXIMUM_COLLECTION_REFRESH_TIME = 1000;
//const int DEFAULT_COLLECTION_REFRESH_TIME = 1;
const int MINIMUM_COLLECTION_WINDOW_FRAME_RATE = 1;
const int MAXIMUM_COLLECTION_WINDOW_FRAME_RATE = 1000;
//const int DEFAULT_COLLECTION_WINDOW_FRAME_RATE = 100;

const QString DEFAULT_XLABEL_LIST_FILENAME = ":/LABEL_LIST.csv";
const QString DEFAULT_YLABEL_LIST_FILENAME = ":/LABEL_LIST.csv";

class DAQWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DAQWindow(QWidget *parent = 0);
    ~DAQWindow();

    void updatePlotData();
    void stopPlotting(PlotWindow* sender);
    void setResizeable(bool resizeable);
    bool saveAs();
    bool clear();


    void closeEvent(QCloseEvent *event) override;

private slots:
    // Serial Handlers
    void handle_connectionTerminated();

    // Other signal handlers
    void handle_AFRTableClosing_emitted();

    // Timer timeout handlers
    void handle_timer_checkForInput_timeout();
    void handle_timer_refreshCollectionWindow_timeout();

    // PushButton handlers
    void handle_pushButton_Plot_Selections_clicked();
    void handle_pushButton_Connect_To_USB_Port_clicked();
    void handle_pushButton_Toggle_Collection_clicked();

    // Menu action handlers
    void handle_actionSave_As();
    void handle_actionClear();
    void handle_actionAutoscroll();
    void handle_actionDisplay_Values();
    void handle_actionCommand_Window();
    void handle_actionPlotting_Defaults();
    void handle_actionCollection_Defaults();
    void handle_actionRefresh_Rate();
    void handle_actionPlot_Anti_aliasing();
    void handle_actionAFR_Feedback_K_d();
    void handle_actionAFR_Feedback_K_p();
    void handle_actionRPM_Feedback_K_p();
    void handle_actionRPM_Feedback_K_d();
    void handle_actionAFR_Table();
    void handle_actionCold_Start_Adjustment();
    void handle_actionCollection_Window_Autoscroll();
    void handle_actionCollection_Window_Display_Values();
    void handle_actionPlotting_Anti_aliasing();
    void handle_actionPlotting_Frame_Rate();
    void handle_actionCollection_Window_Frame_Rate();
    void handle_actionPredefined_Commands_Reset();
    void handle_actionStart_Data_Transmission();
    void handle_actionEnd_Data_Transmission();
    void handle_actionCollection_Window_Syntax_Highlighting();
    void xItemChanged(QModelIndex xindex);
    void yItemChanged(QModelIndex yindex);

private:
    // Interface
    Ui::DAQWindow *ui;

    // Plotting
    QVector<PlotWindow*> pw;
    QVector<QVector<double>> plot_xData;
    QVector<QVector<double>> plot_yData;
    int current_xLabelIndex;
    int current_yLabelIndex;
    QString current_xLabel;
    QString current_yLabel;
    ListModel xLabelModel;
    ListModel yLabelModel;

    // Serial
    SerialHandler* serialHandler;

    // Transceiver
    Signals* transceiver;

    // Data Collection
    Highlighter* syntaxHighlighter;
    QByteArray dataBuffer;
    QByteArray carryoverByteBuffer;
    int numDataPoints;
    QString collectionWindowBuffer;
    QTimer timer_checkForInput;
    QTimer timer_refreshCollectionWindow;
    unsigned int current_Collection_Refresh_Time;
    unsigned int current_Collection_Window_Frame_Rate;

    // AFRTABLE
    AFRTABLE* afrTable;

public:
    bool savePlottingAndCollectionDefaults();
    void loadPlottingAndCollectionDefaults();

    int getPlottingDefaultFrameRate();
    int getPlottingDefaultDataPoints();

    bool getCollectionDefaultAutoscroll();
    bool getCollectionDefaultDisplayValues();
    int getCollectionDefaultFrameRate();
    int getCollectionDefaultRefreshRate();
    bool getCollectionDefaultSyntaxHighlighting();

private:
    struct
    {
        int frameRate = 100;
        int dataPoints = 1000;
        bool antiAliasing = false;
    } plottingDefaults;

    struct
    {
        bool autoscroll = false;
        bool displayValues = false;
        int frameRate = 100;
        int refreshRate = 1;
        bool syntaxHighlighting = false;
    } collectionDefaults;
};

#endif // DAQWINDOW_H
