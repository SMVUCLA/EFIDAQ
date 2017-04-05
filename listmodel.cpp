#include "listmodel.h"

#include <QFile>
#include <QIODevice>
#include <QModelIndex>
#include <QString>
#include <QStringList>

ListModel::ListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return QVariant();
}

int ListModel::rowCount(const QModelIndex& parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return items.length();
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return items[index.row()];

    return QVariant();
}

Qt::ItemFlags ListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QString ListModel::getItemAt(int row) const
{
    if (row >= 0 && row < items.length())
        return items[row];
    return "***ITEM OUT OF BOUNDS***";
}

bool ListModel::loadCSV(QString filename)
{
    QFile inputFile(filename);
    inputFile.open(QIODevice::ReadOnly);

    // Check to make sure the file was opened properly
    if (inputFile.isOpen())
    {
        QString input = inputFile.readAll();
        items = input.split(",");
        return true;
    }
    return false;
}
