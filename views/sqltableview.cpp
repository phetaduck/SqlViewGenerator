#include "sqltableview.h"
#include <QMenu>
#include <QKeyEvent>
#include <QHeaderView>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QSqlRecord>
#include <QDateTime>

#include "item_delegates/comboboxitemdelegate.h"
#include "modelmanager.h"

void SqlTableView::createActions()
{
    m_insertAboveAct = new QAction(this);
    m_insertBelowAct = new QAction(this);
    m_restoreAct = new QAction(this);
    m_revertAct = new QAction(this);
    m_deleteAct = new QAction(this);
    m_commitAct = new QAction(this);

    m_revertAct->setText("Отменить изменения");
}

void SqlTableView::init(
        const TableViewSettings& settings,
        QSqlDatabase db)
{
    auto model = ModelManager::sharedSqlTableModel<AsyncSqlTableModel>(settings.TableName);
    model->setTable(settings.TableName);
    model->setEditStrategy(settings.EditStrategy);
    model->setSort(
                model->fieldIndex(settings.Sorting.first),
                settings.Sorting.second);
    setModel(model);

    auto modelRecord = model->record();
    for (int column = 0; column < model->columnCount(); ++column) {
        auto oldDelegate = itemDelegateForColumn(column);
        auto columnName = modelRecord.fieldName(column);
        if (settings.Relations.count(columnName)) {
            auto delegate = new ComboBoxItemDelegate(this);
            delegate->setDb(db);
            delegate->setRelation(settings.Relations[columnName]);
            setItemDelegateForColumn(column, delegate);
        } else {
            auto delegate = new StyledItemDelegate(this);
            setItemDelegateForColumn(column, delegate);
        }
        if (oldDelegate) {
            oldDelegate->deleteLater();
        }
    }

    for (auto it = settings.Headers.begin(); it != settings.Headers.end(); it++) {
        const auto& column = it.key();
        const auto& header = it.value();
        model->setHeaderData(model->fieldIndex(column), Qt::Horizontal, header);
    }

    for (auto hiddenColumn : settings.HiddenColumns) {
        hideColumn(model->fieldIndex(hiddenColumn));
    }
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    setTextElideMode(Qt::ElideNone);
    setWordWrap(true);
    setAlternatingRowColors(true);
}

bool SqlTableView::showMenu() const
{
    return m_showMenu;
}

void SqlTableView::setShowMenu(bool showMenu)
{
    m_showMenu = showMenu;
}

int SqlTableView::selectedRow() const
{
    return selectionModel()->currentIndex().row();
}

void SqlTableView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
}

void SqlTableView::setModel(SearchableSqlTableModel* model)
{
    if (m_sqlModel == model) { return; }
    QTableView::setModel(model);
    m_sqlModel = model;
    if (m_sqlModel) {
        if (!m_sqlModel->isSelectedAtLeastOnce())
            m_sqlModel->select();
        connectContextMenuActions();
        m_sqlModel->setCanEdit(m_canEdit);
        auto columnIndex = m_sqlModel->fieldIndex(m_deletedFieldName);
        m_sqlModel->alwaysEditableColumns().push_back(columnIndex);
    } else {
        disconnectContextMenuActions();
    }
}

void SqlTableView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_sqlModel || !m_showMenu) return;

    QMenu menu{this};

    auto index = indexAt(event->pos());
    if (model()->rowCount()) {
        m_lastSelectedRow = index.row();
        if (m_lastSelectedRow> -1)
        {
            auto sRowNumber = QString::number(m_lastSelectedRow + 1);

            if (m_canInsert)
            {
                m_insertAboveAct->setText(
                            QString{"Вставить строку выше"});
                menu.addAction(m_insertAboveAct);

                m_insertBelowAct->setText(
                            QString{"Вставить строку ниже"});
                menu.addAction(m_insertBelowAct);
            }
            m_restoreAct->setText(
                        QString{"Восстановить строку №"} +
                        sRowNumber);
            m_deleteAct->setText(
                        QString{"Удалить строку №"} +
                        sRowNumber);
        }
        else
        {
            if (m_canInsert)
            {
                m_insertAboveAct->setText("Добавить новую строку");
                menu.addAction(m_insertAboveAct);
            }

            m_deleteAct->setText("Удалить последнюю строку");
        }

        if (m_canDelete)
        {
            menu.addAction(m_deleteAct);
        }
        else if (m_canRestore)
        {
            menu.addAction(m_restoreAct);
        }
        menu.addSeparator();

        if (m_sqlModel->isDirty())
        {
            m_revertAct->setText("Отменить изменения");
            m_commitAct->setText("Сохранить изменения");
            menu.addAction(m_revertAct);
            menu.addAction(m_commitAct);
        }
    }
    else
    {
        m_insertAboveAct->setText(tr("&Добавить новую строку"));
        menu.addAction(m_insertAboveAct);
    }

    menu.setWindowFlags(menu.windowFlags() | Qt::FramelessWindowHint);
    menu.exec(event->globalPos());
}

QString SqlTableView::deletedFieldName() const
{
    return m_deletedFieldName;
}

void SqlTableView::setDeletedFieldName(const QString& deletedFieldName)
{
    m_deletedFieldName = deletedFieldName;
}

bool SqlTableView::canEdit() const
{
    return m_canEdit;
}

void SqlTableView::setCanEdit(bool canEdit)
{
    m_canEdit = canEdit;
    if (m_sqlModel) {
        m_sqlModel->setCanEdit(m_canEdit);
    }
}

bool SqlTableView::canRestore() const
{
    return m_canRestore;
}

void SqlTableView::setCanRestore(bool canRestore)
{
    m_canRestore = canRestore;
}

bool SqlTableView::canInsert() const
{
    return m_canInsert;
}

void SqlTableView::setCanInsert(bool canInsert)
{
    m_canInsert = canInsert;
}

bool SqlTableView::canDelete() const
{
    return m_canDelete;
}

void SqlTableView::setCanDelete(bool canDelete)
{
    m_canDelete = canDelete;
}

SearchableSqlTableModel* SqlTableView::sqlModel() const
{
    return m_sqlModel;
}

void SqlTableView::insertRow(
        int row,
        const QHash<int, QVariant>& defaultValues)
{
    QSqlRecord newRecord = m_sqlModel->record();
    for (auto it = defaultValues.begin(); it != defaultValues.end(); it++) {
        const auto& column = it.key();
        const auto& value = it.value();
        newRecord.setValue(column, value);
    }
    m_sqlModel->insertRecord(row, newRecord);
}

void SqlTableView::insertRow(
        int row,
        const QHash<QString, QVariant>& defaultValues)
{
    Q_UNUSED(row);
    QHash<int, QVariant> remappedDefaultValues;
    for (auto it = defaultValues.begin(); it != defaultValues.end(); it++) {
        const auto& field = it.key();
        const auto& value = it.value();
        auto column = m_sqlModel->fieldIndex(field);
        remappedDefaultValues[column] = value;
    }
    insertRow(row, remappedDefaultValues);
}
void SqlTableView::deleteRow(int row)
{
    if (row > -1) {
        m_sqlModel->removeRow(row);
    } else {
        m_sqlModel->removeRow(this->m_sqlModel->rowCount() - 1);
    }
}

void SqlTableView::deleteSelectedRow()
{
    deleteRow(selectedRow());
}

void SqlTableView::restoreRow(int row)
{
    auto model = this->sqlModel();
    auto markedForDeletionFieldIndex = model->fieldIndex(
                                           m_deletedFacilityFieldName);
    auto index = model->index(
                     row,
                     markedForDeletionFieldIndex);
    bool success = model->setData(
                       index,
                       QVariant{bool{false}});
    if (success) {
//        model->submitAll();
    } else {
        qDebug() << "Cannot set data: " << QVariant{bool{false}} << index;
    }
}

void SqlTableView::restoreSelectedRow()
{
    restoreRow(m_lastSelectedRow);
}

void SqlTableView::submit()
{
    if (!this->m_sqlModel->submitAll()) {
    }
}

void SqlTableView::connectContextMenuActions()
{
    disconnectContextMenuActions();
    createActions();
    connect(m_insertAboveAct, &QAction::triggered,
            [this] ()
    {
        this->insertRow(this->m_lastSelectedRow);
    }
    );
    connect(m_insertBelowAct, &QAction::triggered,
            [this] ()
    {
        this->insertRow(this->m_lastSelectedRow + 1);
    }
    );
    connect(m_restoreAct, &QAction::triggered,
            this, &SqlTableView::restoreSelectedRow);

    connect(m_deleteAct, &QAction::triggered,
            [this] ()
    {
        this->deleteRow(this->m_lastSelectedRow);
    }
    );
    connect(m_commitAct, &QAction::triggered,
            [this]()
    {
        submit();
    });
    connect(m_revertAct, &QAction::triggered,
            m_sqlModel, &SearchableSqlTableModel::revertAll);
}

void SqlTableView::disconnectContextMenuActions()
{
    if (m_insertBelowAct) {
        m_insertBelowAct->deleteLater();
        m_insertBelowAct = nullptr;
    }
    if (m_insertAboveAct) {
        m_insertAboveAct->deleteLater();
        m_insertAboveAct = nullptr;
    }
    if (m_restoreAct) {
        m_restoreAct->deleteLater();
        m_restoreAct = nullptr;
    }
    if (m_revertAct) {
        m_revertAct->deleteLater();
        m_revertAct = nullptr;
    }
    if (m_commitAct) {
        m_commitAct->deleteLater();
        m_commitAct = nullptr;
    }
    if (m_deleteAct) {
        m_deleteAct->deleteLater();
        m_deleteAct = nullptr;
    }
}
