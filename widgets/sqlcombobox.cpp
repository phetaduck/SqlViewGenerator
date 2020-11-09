#include "sqlcombobox.h"
#include <models/searchablesqltablemodel.h>
#include <utils/sqlsharedutils.h>

#include <QTimer>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>

void SqlComboBox::setModel(QAbstractItemModel* model)
{
    QComboBox::setModel(model);
    m_sqlModel = nullptr;
}

QModelIndex SqlComboBox::selectedIndex() const
{
    QModelIndex out;
    if (m_sqlModel) {
        out = selectedIndex(
                      m_sqlRelation.indexColumn());
    }
    return out;
}

QModelIndex SqlComboBox::selectedIndex(int column) const
{
    QModelIndex out;
    if (m_sqlModel) {
        out = m_sqlModel->index(currentIndex(), column);
    }
    return out;
}

QModelIndex SqlComboBox::selectedIndex(const QString& fieldName) const
{
    QModelIndex out;
    if (m_sqlModel) {
        out = m_sqlModel->index(currentIndex(), fieldName);
    }
    return out;
}

void SqlComboBox::setSelectedIndex(const QVariant& data)
{
    if (m_sqlModel)
    {
        auto newIndex = m_sqlModel->findIndex(
                            m_sqlRelation.indexColumn(),
                            data).row();
        if (newIndex != currentIndex()) {
            setCurrentIndex(newIndex);
        }
    } else {
        auto newText = data.toString();
        if (newText != currentText()) {
            setCurrentText(data.toString());
        }
    }
}

SearchableSqlTableModel* SqlComboBox::sqlModel()
{
    return m_sqlModel;
}

auto SqlComboBox::sqlRelation() const -> const QSqlRelation&
{
    return m_sqlRelation;
}


void SqlComboBox::setSqlData(SearchableSqlTableModel* sqlModel,
                const QSqlRelation& sqlRelation)
{
    m_sqlModel = sqlModel;
    m_sqlRelation = sqlRelation;
    int column = m_sqlModel->fieldIndex(
                     m_sqlRelation.displayColumn());
    QComboBox::setModel(m_sqlModel);
    setModelColumn(column);
}

QVariant SqlComboBox::data()
{
    return selectedIndex().data();
}

void SqlComboBox::setData(const QVariant& data)
{
    setSelectedIndex(data);
}

void SqlComboBox::setSqlRelation(const QSqlRelation& sqlRelation)
{
    m_sqlRelation = sqlRelation;
    if (!m_sqlModel) {
        m_sqlModel = ModelManager::sharedSqlTableModel<SearchableSqlTableModel>(m_sqlRelation.tableName());
        if (!m_sqlModel->isSelectedAtLeastOnce())
            m_sqlModel->select();
        int column = m_sqlModel->fieldIndex(
                         m_sqlRelation.displayColumn());
        QComboBox::setModel(m_sqlModel);
        setModelColumn(column);
    }
    emit sqlRelationChanged(m_sqlRelation);
}
