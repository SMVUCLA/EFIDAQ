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

#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QQueue>
#include <QVector>
#include <QTime>

#include "wrappingqvector.h"
#include "qcustomplot.h"

namespace Ui {
class PlotWindow;
}

const int DEFAULT_MAX_PLOTTED_POINTS = 1000;
const int DEFAULT_FRAME_RATE = 100;

class DAQWindow;
class QTimer;

class PlotWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit PlotWindow(QWidget* parent = 0, DAQWindow* rparent = 0);
  ~PlotWindow();

  void setupPlayground(QCustomPlot *customPlot);
  void addData(QVector<double>& X, QVector<double>& Y);
  void setup(QCustomPlot *customPlot);

  bool setXLabel(QString xLabel, int xLabelIndex);
  bool setYLabel(QString yLabel, int yLabelIndex);
  int getXLabelIndex() const;
  int getYLabelIndex() const;

private slots:
  void handleActionDataPointsTriggered();
  void handleActionClearTriggered();
  void handleActionConnectPointsTriggered(bool connect);
  void handleActionSavePlotTriggered();
  void handleActionFrameRateTriggered();

  void handleRefreshTimerTimeout();

private:
  void closeEvent(QCloseEvent *event);

  Ui::PlotWindow *ui;
  QTimer dataTimer;

  QTime* time;
  QTimer* timer;
  unsigned long long int lastTime;
  double secPerFrame;
  int frameRate;
  WrappingQVector<double> measuredFrameRate;
  bool changed;

  WrappingQVector<double> m_xData;
  WrappingQVector<double> m_yData;
  QString xLabel;
  int xLabelIndex;
  QString yLabel;
  int yLabelIndex;

  DAQWindow* DAQparent;
};

#endif // PLOTWINDOW_H
