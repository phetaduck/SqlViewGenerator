#pragma once

#include <QListWidget>

class SqlListWidget : public QListWidget
{
    Q_OBJECT
public:
    using QListWidget::QListWidget;

#ifndef QT_NO_CONTEXTMENU
    /** @brief вызывается при запросе контекстного меню */
    void contextMenuEvent(QContextMenuEvent* event) override;
#endif // QT_NO_CONTEXTMENU

signals:
    void setAsRelation(QString relatonTable);
    void addToWatchList(QString table);

private:
    QAction* m_addToWatchList = nullptr;
    QAction* m_setAsRelation = nullptr;

    QString m_currentTable;
};
