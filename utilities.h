#ifndef UTILITIES_H
#define UTILITIES_H

#include <QList>
#include <QString>
#include <QChar>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QRegExp>

const QString DEFAULT_AFR_TABLE_FILEPATH = ":/AFR_TABLE.csv";
const QString DEFAULT_LABEL_LIST_FILEPATH = ":/LABEL_LIST.csv";
const QString DEFAULT_LOGO_FILEPATH = ":/SupermileageLogo.png";

bool loadCSV(QString filename, QList<QList<QString>>& allFields);

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

#endif // UTILITIES_H
