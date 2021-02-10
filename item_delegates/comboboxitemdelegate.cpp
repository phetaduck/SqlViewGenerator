#include "comboboxitemdelegate.h"

#include "widgets/sqlcombobox.h"

#include <QApplication>
#include <QPainter>
#include <QSqlQueryModel>
#include <functional>

QString ComboBoxItemDelegate::getText(const QModelIndex& index) const
{
    QString out;
    auto relationIndex = m_sqlModel->findIndex(
                             m_relation.displayColumn(),
                             m_relation.indexColumn(),
                             index.data());
    if (relationIndex.isValid()) {
        auto data = relationIndex.data();
        out = data.toString();
    }
    return out;
}

QWidget* ComboBoxItemDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto editor = new SqlComboBox(parent);
    return editor;
}

void ComboBoxItemDelegate::setEditorData(QWidget* editor,
                                         const QModelIndex& index) const
{
    auto comboBox = qobject_cast<SqlComboBox*>(editor);
    if ( comboBox) {
        comboBox->setSqlData(m_sqlModel, m_relation);
        auto data = index.data();
        comboBox->setSelectedIndex(data);
        auto workaroundLambda = std::bind(this->m_commitLambda, comboBox);
        connect(comboBox,
                QOverload<int>::of(&SqlComboBox::currentIndexChanged),
                this, workaroundLambda);
    }
}

void ComboBoxItemDelegate::setModelData(QWidget* editor,
                                        QAbstractItemModel* model,
                                        const QModelIndex& index) const
{
    auto comboBox = qobject_cast<SqlComboBox*>(editor);
    if (comboBox)
    {
        model->setData(index, comboBox->data());
    }
}

QSize ComboBoxItemDelegate::sizeHint(
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{
    return option.fontMetrics.boundingRect(getText(index)).size();
}

void ComboBoxItemDelegate::setRelation(QSqlRelation relation)
{
    m_relation = std::move(relation);
    m_commitLambda = [this](QWidget* editor)
    {
        // workaround for gcc 6.6
        emit this->commitData(editor);
    };
    if (!m_sqlModel)
    {
        m_sqlModel = new SqlTableModel(this, m_db);
        m_sqlModel->setTable(m_relation.tableName());
        m_sqlModel->select();
    }
}

QSqlDatabase ComboBoxItemDelegate::getDb() const
{
    return m_db;
}

SqlTableModel* ComboBoxItemDelegate::getSqlModel() const
{
    return m_sqlModel;
}

void ComboBoxItemDelegate::setSqlModel(SqlTableModel* sqlModel)
{
    m_sqlModel = sqlModel;
}

void ComboBoxItemDelegate::setDb(QSqlDatabase db)
{
    m_db = db;
}
