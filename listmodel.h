#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class ListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ListModel(QObject* parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Editable:
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // For accessing items
    QString getItemAt(int row) const;

    // Load CSV file as a Qt List
    bool loadCSV(QString filename);
private:
    QStringList items;
};

#endif // LISTMODEL_H
