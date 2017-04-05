#include "utilities.h"
#include <QChar>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QRegExp>

// Loads a comma separated file into a 2 Dimensional QList of QStrings
// Assumes that there are header lines.
bool loadCSV(QString filename, QList<QList<QString>>& allFields)
{
    QFile inputFile(filename);
    inputFile.open(QIODevice::ReadOnly);

    // Check to make sure the file was opened properly
    if (inputFile.isOpen())
    {
        const QChar delimiter = ',';
        const QChar newline = '\n';

        // Possibly need to check if input stream was created successfully?
        QTextStream in(&inputFile);

        // Read in all the bytes from the .csv file
        QString allText = in.readAll();

        // Split the files by line separators
        QList<QString> allLines = allText.split(newline);

        // Initialize 2D QList of QStrings
        for (int i = 0; i < allFields.length(); i++)
            allFields[i].clear();
        allFields.clear();

        // Split the lines by delimiter
        for (int i = 0; i < allLines.length(); i++)
            allFields.append(allLines[i].split(delimiter));

        // Remove empty parts.
        bool emptyLine;
        for (int i = 0; i < allFields.length(); i++)
        {
            emptyLine = true;
            for (int j = 0; j < allFields[i].length(); j++)
            {
                if (!allFields[i][j].isEmpty()) emptyLine = false;
                else allFields[i].removeAt(j);
            }
            if (emptyLine) allFields.removeAt(i);
        }

        // Check to make sure the fields aren't all empty.
        int length = 0;
        if (allFields.length() > 0)
            length = allFields[0].length() + 1;
        else
        {
            for (int i = 0; i < allFields.length(); i++)
                allFields[i].clear();
            allFields.clear();
            return false;
        }

        // Check to make sure the length of every column is the same.
        // If they are not, return false.
        for (int i = 1; i < allFields.length(); i++)
        {
            if (allFields[i].length() != length)
            {
                for (int i = 0; i < allFields.length(); i++)
                    allFields[i].clear();
                allFields.clear();
                return false;
            }
        }

        // Perform resource cleanup
        inputFile.close();

        // Return true on success
        return true;
    }
    // Return false on failure to open file
    return false;
}

double mean(QVector<double> in)
{
    double total = 0;
    for (int i = 0; i < in.size(); i++)
    {
        total += in[i];
    }
    return total / in.size();
}

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;


    //fieldFormat.setForeground(QBrush(QColor(50, 132, 191)));
    fieldFormat.setForeground(Qt::green);
    fieldFormat.setFontWeight(QFont::Bold);
    QString pattern("[0123456789.-+]");
    rule.pattern = QRegExp(pattern);
    rule.format = fieldFormat;
    highlightingRules.append(rule);

    delimiterFormat.setForeground(Qt::magenta);
    delimiterFormat.setFontWeight(QFont::Light);
    QStringList delimiterPatterns;
    delimiterPatterns << "[,;]";
    foreach (const QString &pattern, delimiterPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = delimiterFormat;
        highlightingRules.append(rule);
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}
