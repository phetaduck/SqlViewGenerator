#include "sqlsyntaxhighlighter.h"

void SQLSyntaxHighlighter::highlightBlock(const QString& text)
{
    int pos = 0;

    static QRegExp rgx ("^\\s*(--)");

    if ((pos = rgx.indexIn (text, pos)) != -1)
    {
        setFormat (pos, text.length(), Qt::darkGreen);
        return;
    }

    setFormat (0, text.length(), Qt::black);

    static QRegExp rw_rgx ("\\b(?:select|from|where|and|case|when|then|else|distinct|all|null|"
                           "is|like|between|not|count|group|by|having|order|inner|outer|right|left|"
                           "begin|end|return|set|new|old|returns|language|cost|or|create|replace|view|table"
                           "public|declare|values|record|exception|raise|alter|function|to|owner|execute|"
                           "procedure|trigger|for|row|each|drop|if|elsif|instead|of|returning|database|user|"
                           "join|on|using|union|exists|in|as|intersect|except|coalesce|insert|delete|into|update)\\b",
                           Qt::CaseInsensitive);

    pos = 0;
    while ((pos = rw_rgx.indexIn (text, pos)) != -1)
    {
        setFormat (pos, rw_rgx.matchedLength(), Qt::blue);
        pos += rw_rgx.matchedLength();
    }

    static QRegExp d_rgx ("(\\$.*\\$)", Qt::CaseInsensitive);

    pos = 0;
    while ((pos = d_rgx.indexIn (text, pos)) != -1)
    {
        setFormat (pos, d_rgx.matchedLength(), Qt::darkMagenta);
        pos += d_rgx.matchedLength();
    }

    static QRegExp l_rgx ("('.*')", Qt::CaseInsensitive);

    pos = 0;
    while ((pos = l_rgx.indexIn (text, pos)) != -1)
    {
        setFormat (pos, l_rgx.matchedLength(), Qt::darkYellow);
        pos += l_rgx.matchedLength();
    }

    static QRegExp o_rgx (":(\\w*)", Qt::CaseInsensitive);

    pos = 0;
    while ((pos = o_rgx.indexIn (text, pos)) != -1)
    {
        setFormat (pos, o_rgx.matchedLength(), Qt::red);
        pos += o_rgx.matchedLength();
    }
}
