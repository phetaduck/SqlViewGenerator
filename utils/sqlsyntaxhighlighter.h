#pragma once

#include <QSyntaxHighlighter>
#include <QTextEdit>

class SQLSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    using QSyntaxHighlighter::QSyntaxHighlighter;

    virtual void highlightBlock (const QString& text);
};
