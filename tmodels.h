#ifndef TMODELS_H
#define TMODELS_H

#include <QAbstractTableModel>
#include <QAbstractListModel>
#include <QList>
#include <QString>

class AFR_TABLE_MODEL : public QAbstractTableModel
{
    Q_OBJECT
public:
    AFR_TABLE_MODEL(QObject *parent);
    AFR_TABLE_MODEL(QObject *parent, QString filename);

    // Determines the number of rows in the table.
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;

    // Determines the number of columns in the table.
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    // Determines the text in each cell in the table when it updates.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    // Determines the initial headers over each column and to the left of each row.
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    // Allows the user to enter text into the table.
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    // Flags that change properties about specific cells such as which cells can be edited.
    Qt::ItemFlags flags(const QModelIndex & index) const ;

    bool loadTable(QString filename);

    // Returns the data stored in the table as a QVector of Qvectors of QStrings
    QList<QList<QString>> getTable() const;

    const QVector<QVector<float>>& getChangedCells() const;

    bool setVal(const QVector<QVector<int>>& coords, QVector<float> vals);
private:
    // holds text entered into the table cells
    QList<QList<QString>> m_gridData;
    QList<QString> m_rowHeaders;
    QList<QString> m_colHeaders;
    QVector<QVector<float>> changedCellVals;
signals:
    void editCompleted(const QString &);
};

class LIST_CHOICES_MODEL : public QAbstractListModel
{
    Q_OBJECT
public:
    LIST_CHOICES_MODEL(QObject *parent);
    ~LIST_CHOICES_MODEL();

    // Determines the number of rows in the list.
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;

    // Determines the text in each cell of the list every time it updates the list.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    // Determines the initial headers over each column and to the left of each row.
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    // Returns the String at a specific index in the list data.
    // If there is no item at the specified index, returns false and item is unchanged.
    bool getItemAt(int row, QString& item);

    // Allows the user to enter text into the table.
    //bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    // Flags that change properties about specific cells such as which cells can be edited.
    //Qt::ItemFlags flags(const QModelIndex & index) const ;

private:
    // holds text entered into the table cells
    QList<QString> m_listData;
signals:
};

#endif // TMODELS_H
