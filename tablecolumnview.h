#pragma once

#include <QWidget>

namespace Ui {
class TableColumnView;
}

class TableColumnView : public QWidget
{
    Q_OBJECT

public:
    explicit TableColumnView(QWidget *parent = nullptr);
    ~TableColumnView();

    QString column() const;
    QString type() const;

    void setColumn(const QString& column);
    void setType(const QString& column);

    bool shouldAdd() const;

signals:
    void add(const QString& column);
    void remove(const QString& column);

    void columnChanged(const QString& column);
    void typeChanged(const QString& column);

private:
    Ui::TableColumnView *ui;
};
