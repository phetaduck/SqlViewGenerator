#include "sqllistwidget.h"

#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

void SqlListWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_setAsRelation) {
        m_setAsRelation = new QAction(this);
        m_setAsRelation->setText("Set As Relationtable");
        connect(m_setAsRelation, &QAction::triggered,
                this, [this]()
        {
            emit setAsRelation(m_currentTable);
        });
    }
    if (!m_addToWatchList) {
        m_addToWatchList = new QAction(this);
        m_addToWatchList->setText("Add to Watch List");
        connect(m_addToWatchList, &QAction::triggered,
                this, [this]()
        {
            emit addToWatchList(m_currentTable);
        });
    }
    auto item = itemAt(event->pos());
    if (item) {
        m_currentTable = item->data(Qt::DisplayRole).toString();
        QMenu menu{this};
        menu.addActions({m_setAsRelation, m_addToWatchList});
        menu.setWindowFlags(menu.windowFlags() | Qt::FramelessWindowHint);
        menu.exec(event->globalPos());
    }
}
