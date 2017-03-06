#include "tmodels.h"
#include "utilities.h"
#include <QAbstractItemModel>
#include <QBrush>
#include <QFont>
#include <QTextStream>
#include <QFile>
#include <stdio.h>
#include <QRegularExpression>

//================================//
// AFR_TABLE_MODEL IMPLEMENTATION //
//================================//

AFR_TABLE_MODEL::AFR_TABLE_MODEL(QObject *parent)
    :QAbstractTableModel(parent)
{

}

AFR_TABLE_MODEL::AFR_TABLE_MODEL(QObject *parent, QString filename)
    :QAbstractTableModel(parent)
{
    if (!loadTable(filename))
    {
        notify("Unable to load specified file.");
    }
}

// Determines the number of rows in the table.
int AFR_TABLE_MODEL::rowCount(const QModelIndex & /*parent*/) const
{
    return m_gridData.length();
}

// Determines the number of columns in the table.
int AFR_TABLE_MODEL::columnCount(const QModelIndex & /*parent*/) const
{
    if (!m_gridData.isEmpty())
        return m_gridData[0].length();
    else
        return 0;
}

// Sets the parameters that determine what and how data is displayed in the cells
// of the table. The top left corner of the table is row = 0 and column = 0 while the
// bottom right corner of the table is row = rowCount() - 1 and column = columnCount() - 1.
QVariant AFR_TABLE_MODEL::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    //notify(QString("data (%1,%2)").arg(row).arg(col));

    switch(role){
    case Qt::DisplayRole: // Sets the text at the point (row,column) in the table.
        return m_gridData[row][col];
        break;
    case Qt::FontRole: // Sets the type of font at the point (row,column) in the table.
        /*if (row == 0 && col == 0) //change font only for cell(0,0)
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }*/
        break;
    case Qt::BackgroundRole:

        /*if (row == 1 && col == 2)  //change background only for cell(1,2)
        {
            QBrush redBackground(Qt::red);
            return redBackground;
        }*/
        break;
    case Qt::TextAlignmentRole:

        /*if (row == 1 && col == 1) //change text alignment only for cell(1,1)
        {
            return Qt::AlignRight + Qt::AlignVCenter;
        }*/
        break;
    case Qt::CheckStateRole:

        /*if (row == 1 && col == 0) //add a checkbox to cell(1,0)
        {
            return Qt::Checked;
        }*/
        break;
    }
    return QVariant();
}

// Controls the text displayed for the header over each column and row.
QVariant AFR_TABLE_MODEL::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) { // Headers for each column (appear above)
            return m_colHeaders[section];
        }
        else if (orientation == Qt::Vertical) { // Headers for each row (appear on the left)
            return m_rowHeaders[section];
        }
    }
    return QVariant();
}

// Allows the user to edit data in the table.
bool AFR_TABLE_MODEL::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        //save value from editor to member m_gridData
        if (value.toString() != m_gridData[index.row()][index.column()] &&
                value.canConvert<double>())
        {
            m_gridData[index.row()][index.column()] = value.toString();
            QVector<float> coord;
            coord.append(index.row());
            coord.append(index.column());
            changedCellVals.append(coord);
            changedCellVals[changedCellVals.length()-1].append(value.toFloat());

            // Need to emit this signal to tell the table to update its values.
            emit dataChanged(index, index);
        }
    }
    return true;
}

bool AFR_TABLE_MODEL::setVal(const QVector<QVector<int>>& coords, QVector<float> vals)
{
    for (int i = 0; i < vals.length(); i++)
    {
        m_gridData[coords[0][0]][coords[0][1]] = vals[i];
        emit dataChanged(QAbstractItemModel::createIndex(coords[0][0], coords[0][1]),
                QAbstractItemModel::createIndex(coords[0][0], coords[0][1]));
    }
    return true;
}

// Flags for specific cells.
Qt::ItemFlags AFR_TABLE_MODEL::flags(const QModelIndex & /*index*/) const
{
    //int row = index.row();
    //int column = index.column();

    // This will make all cells in the table editable.
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

// Load a .csv file into the gridData
bool AFR_TABLE_MODEL::loadTable(QString csvfile)
{
    // Attempts to load the csvfile contents into the table.
    if (loadCSV(csvfile, m_gridData))
    {
        // Get the row headers
        for (int i = 1; i < m_gridData.length(); i++)
        {
            m_rowHeaders.append(m_gridData[i][0]);
            m_gridData[i].removeAt(0);
        }
        // Get the col headers
        m_colHeaders = m_gridData[0];
        m_gridData.removeAt(0);
        return true;
    }
    return false;
}

// Returns the coordinates and values of those coordinates that have been changed.
const QVector<QVector<float>>& AFR_TABLE_MODEL::getChangedCells() const
{
    return changedCellVals;
}

QList<QList<QString>> AFR_TABLE_MODEL::getTable() const
{
    return m_gridData;
}

//===================================//
// LIST_CHOICES_MODEL IMPLEMENTATION //
//===================================//

LIST_CHOICES_MODEL::LIST_CHOICES_MODEL(QObject *parent)
    :QAbstractListModel(parent)
{
    QList<QList<QString>> allFields;
    // Attempts to load the LABEL_LIST.csv contents into the table.
    // Need to make it so that the user can specify the name of the file
    // to load from. This will be the default value for now.
    if (loadCSV(":/LABEL_LIST.csv", allFields))
    {
        // Extracts the list labels from the first line of the .csv file.
        // Checks to make sure the fields aren't empty
        if (!allFields.isEmpty())
        {
            m_listData = allFields[0];

            // Delete any strings that are empty.
            for (int i = 0; i < m_listData.length(); i++)
            {
                // Remove any invalid characters in the list.
                m_listData[i].remove(QRegExp(QString("[\n\r]")));

                // Remove any empty list items.
                if (m_listData[i].isEmpty())
                {
                    m_listData.removeAt(i);
                }
            }
        }
    }
}

LIST_CHOICES_MODEL::~LIST_CHOICES_MODEL()
{

}

// Determines the number of rows in the list.
int LIST_CHOICES_MODEL::rowCount(const QModelIndex & /*parent*/) const
{
    return m_listData.length();
}

// Sets the parameters that determine what and how data is displayed in the cells
// of the table. The top left corner of the table is row = 0 and column = 0 while the
// bottom right corner of the table is row = rowCount() - 1 and column = columnCount() - 1.
QVariant LIST_CHOICES_MODEL::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    switch(role){
    case Qt::DisplayRole: // Sets the text at the point (row,column) in the table.
        return m_listData[row];
        break;
    case Qt::FontRole: // Sets the type of font at the point (row,column) in the table.
        // Change the font
        return QVariant();
        break;
    case Qt::BackgroundRole:
        // Controls the background color for each cell
        return QVariant();
        break;
    case Qt::TextAlignmentRole:
        // Controls the text alignment for each cell
        return QVariant();
        break;
    case Qt::CheckStateRole:
        // Controls the state of any cells that have checkboxes
        return QVariant();
        break;
    }
    // Return value for all "Roles" that do not have a switch statement to handle them.
    return QVariant();
}

// Controls the text displayed for the header for each row.
QVariant LIST_CHOICES_MODEL::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) { // Headers for each column (appear above)
            switch (section)
            {
            case 0:
                return QString("0 - 799");
            case 1:
                return QString("800 - 1599");
            case 2:
                return QString("1600 - 2399");
            case 3:
                return QString("2400 - 3199");
            case 4:
                return QString("3200 - 3999");
            case 5:
                return QString("4000 - 4799");
            case 6:
                return QString("4800 - 5599");
            case 7:
                return QString("5600 - 6399");
            case 8:
                return QString("6400 - 7199");
            case 9:
                return QString("7200 - 7999");
            }
        }
    }
    return QVariant();
}

// Returns the String at a specific index in the list data.
// If there is no item at the specified index, returns false and item is unchanged.
bool LIST_CHOICES_MODEL::getItemAt(int row, QString& item)
{
    if (row >= 0 && row < m_listData.length()) {
        item = m_listData[row];
        return true;
    }
    else return false;
}
