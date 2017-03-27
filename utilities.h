#ifndef UTILITIES_H
#define UTILITIES_H

#include <QList>
#include <QString>
#include <QChar>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QRegExp>

namespace efidaq
{
const int SUCCESS = 0;
const int CANCELLED = 1;
const int FAILED_CURRENTLY_COLLECTING = 2;
const int OPEN_FILE_FAILED = 3;
const int WRITE_FILE_FAILED = 4;

const int DEFAULT_BAUD_RATE = 250000;//860400;
const int COLLECTION_BY_SERIAL = 0;
const int COLLECTION_BY_UDP = 1;

const int DEFAULT_MAX_PLOTTED_POINTS = 100;
const int DEFAULT_FRAME_RATE = 30;

const int FILTER_NONE = 0;
const int FILTER_BY_NUM_FIELDS = 1;
const int FILTER_BY_CONTENT = 2;

const int DEFAULT_PORTNO = 50000;
const QString DEFAULT_IPADDRESS = "0.0.0.0";

const QString DEFAULT_AFR_TABLE_FILEPATH = ":/AFR_TABLE.csv";
const QString DEFAULT_LABEL_LIST_FILEPATH = ":/LABEL_LIST.csv";
const QString DEFAULT_LOGO_FILEPATH = ":/SupermileageLogo.png";
}

unsigned int numfields(const QChar delimiter, const QString input);

bool loadCSV(QString filename, QList<QList<QString>>& allFields);

void notify(QString msg);
void notify(QList<QString> msg);
void notify(QList<QList<QString>> msg);

double mean(QVector<double> in);

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp delimiterExpression;
    QTextCharFormat delimiterFormat;

    QRegExp fieldExpression;
    QTextCharFormat fieldFormat;
};

class ByteFilter
{
public:
    ByteFilter();
    ByteFilter(const QVector<QRegExp>& filterExpressions);
    ByteFilter(const QList<QRegExp>& filterExpressions);

    void pass(QString& string);
    void pass(QList<QString>& stringList);
    void pass(QVector<QString>& stringVector);

    bool addFilter(QString filterExp);
    bool addFilter(QRegExp filterExp);
private:
    QVector<QRegExp> filterExpressions;

};

#endif // UTILITIES_H
