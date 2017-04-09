/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2016 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 13.09.16                                             **
**          Version: 2.0.0-beta                                           **
****************************************************************************/

/************************************************************************************************************
**                                                                                                         **
**  This is the example code for QCustomPlot.                                                              **
**                                                                                                         **
**  It demonstrates basic and some advanced capabilities of the widget. The interesting code is inside     **
**  the "setup(...)Demo" functions of PlotWindow.                                                          **
**                                                                                                         **
**  In order to see a demo in action, call the respective "setup(...)Demo" function inside the             **
**  PlotWindow constructor. Alternatively you may call setupDemo(i) where i is the index of the demo       **
**  you want (for those, see PlotWindow constructor comments). All other functions here are merely a       **
**  way to easily create screenshots of all demos for the website. I.e. a timer is set to successively     **
**  setup all the demos and make a screenshot of the window area and save it in the ./screenshots          **
**  directory.                                                                                             **
**                                                                                                         **
*************************************************************************************************************/

#include "ui_plotwindow.h"
#include "plotwindow.h"
#include "daqwindow.h"
#include "utilities.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>

PlotWindow::PlotWindow(QWidget* parent, DAQWindow* DAQparent) :
  QMainWindow(parent),
  ui(new Ui::PlotWindow),
  m_xData(DAQparent->getPlottingDefaultDataPoints()),
  m_yData(DAQparent->getPlottingDefaultDataPoints()),
  measuredFrameRate(DAQparent->getPlottingDefaultFrameRate())
{
  ui->setupUi(this);
  setWindowIcon(QIcon(DEFAULT_LOGO_FILEPATH));
  
  // Needed in order to set a flag in RUNTEST if the plotting window
  // is abruptly closed.
  this->DAQparent = DAQparent;

  // Connect the menu actions to their slots
  connect(ui->actionData_Points, SIGNAL(triggered()), SLOT(handleActionDataPointsTriggered()));
  connect(ui->actionClear, SIGNAL(triggered()), SLOT(handleActionClearTriggered()));
  connect(ui->actionConnect_Points, SIGNAL(triggered(bool)), SLOT(handleActionConnectPointsTriggered(bool)));
  connect(ui->actionSave_Plot, SIGNAL(triggered()), SLOT(handleActionSavePlotTriggered()));
  connect(ui->actionFrame_Rate, SIGNAL(triggered()), SLOT(handleActionFrameRateTriggered()));

  setup(ui->customPlot);
  statusBar()->clearMessage();
  ui->customPlot->replot();

  time = new QTime;
  time->start();
  lastTime = time->elapsed();
  frameRate = DAQparent->getPlottingDefaultFrameRate();
  secPerFrame = 1.0 / frameRate;

  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), SLOT(handleRefreshTimerTimeout()));
  timer->start(secPerFrame * 1000);
  changed = true;

  //setupDemo();
  // for making screenshots of the current demo or all demos (for website screenshots):
  //QTimer::singleShot(1500, this, SLOT(allScreenShots()));
  //QTimer::singleShot(4000, this, SLOT(screenShot()));
}

void PlotWindow::addData(QVector<double>& X, QVector<double>& Y)
{
    if (X.isEmpty() || Y.isEmpty() || X.size() != Y.size())
        return;

    m_xData.push(X);
    m_yData.push(Y);

    changed = true;
}

void PlotWindow::handleRefreshTimerTimeout()
{
    if (changed)
    {
        // add data to lines:
        ui->customPlot->graph(0)->setData(m_xData, m_yData);

        // rescale both axes to fit the current data
        ui->customPlot->graph(0)->rescaleAxes();
        bool foundRange = false;
        QCPRange xrange = ui->customPlot->graph(0)->getKeyRange(foundRange);
        if (foundRange)
        {
            ui->customPlot->xAxis->setRange(xrange);
        }

        foundRange = false;
        QCPRange yrange = ui->customPlot->graph(0)->getValueRange(foundRange);
        if (foundRange)
        {
            ui->customPlot->yAxis->setRange(yrange);
        }

        ui->customPlot->replot();
        double deltaTime = (time->elapsed() - lastTime) / 1000.0;
        lastTime = time->elapsed();

        measuredFrameRate.push(1 / deltaTime);
        double measure = mean(measuredFrameRate);
        ui->statusBar->showMessage(QString("Measured Frame Rate: %1").arg(measure));
        measuredFrameRate.setMaxSize((int) measure);

        // make key axis range scroll with the data (at a constant range size of 8):
        //ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);
        changed = false;
    }
}

void PlotWindow::setup(QCustomPlot* customPlot)
{
    // include this section to fully disable antialiasing for higher performance:

    customPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    customPlot->xAxis->setTickLabelFont(font);
    customPlot->yAxis->setTickLabelFont(font);
    customPlot->legend->setFont(font);

    // Creates the graph
    customPlot->addGraph();

    // Sets the points to blue
    customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    //QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    //timeTicker->setTimeFormat("%h:%m:%s");
    //customPlot->xAxis->setTicker(timeTicker);
    //customPlot->axisRect()->setupFullAxesBox();
    //customPlot->yAxis->setRange(-1.2, 1.2);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
}

void PlotWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

void PlotWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    DAQparent->stopPlotting(this);
}

PlotWindow::~PlotWindow()
{
    delete ui;
    delete time;
    delete timer;
}

void PlotWindow::handleActionDataPointsTriggered()
{
    int value = m_xData.maxSize();
    value = QInputDialog::getInt(
                this,
                QString("Data Point"),
                QString("Input the maximum number of data points that can be on the plot."),
                value,
                1,
                INT_MAX);
    m_xData.setMaxSize(value);
    m_yData.setMaxSize(value);
}

void PlotWindow::handleActionClearTriggered()
{
    m_xData.clear();
    m_yData.clear();
    ui->customPlot->graph(0)->setData(m_xData, m_yData);
    ui->customPlot->replot();
}

void PlotWindow::handleActionConnectPointsTriggered(bool connect)
{
    if (connect)
    {
        // Sets the line style to be connect data points by a line.
        ui->customPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsLine);

        // Sets the scatter style to none for lines.
        ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 6));
    }
    else
    {
        // Sets the line style to none, making it a scatter plot.
        ui->customPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsNone);

        // Sets the scatter style to be discs.
        ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
    }
    changed = true;
}

void PlotWindow::handleActionSavePlotTriggered()
{
    QMessageBox::warning(this,
                         "ERROR",
                         "Action not yet implemented."
                         );
}

void PlotWindow::handleActionFrameRateTriggered()
{
    int value = frameRate;
    value = QInputDialog::getInt(
                this,
                QString("Frame Rate"),
                QString("Input the desired frame rate for refreshing the plot."),
                value,
                1,
                INT_MAX);
    frameRate = value;
    secPerFrame = 1.0 / frameRate;
    timer->start(secPerFrame * 1000);
}

bool PlotWindow::setXLabel(QString xLabel, int xLabelIndex)
{
    this->xLabel = xLabel;
    this->xLabelIndex = xLabelIndex;
    ui->customPlot->xAxis->setLabel(xLabel);
    changed = true;
    return true;
}

bool PlotWindow::setYLabel(QString yLabel, int yLabelIndex)
{
    this->yLabel = yLabel;
    this->yLabelIndex = yLabelIndex;
    ui->customPlot->yAxis->setLabel(yLabel);
    changed = true;
    return true;
}

int PlotWindow::getXLabelIndex() const
{
    return xLabelIndex;
}

int PlotWindow::getYLabelIndex() const
{
    return yLabelIndex;
}





























