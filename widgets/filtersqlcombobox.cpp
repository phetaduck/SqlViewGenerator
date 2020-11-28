#include "filtersqlcombobox.h"

#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>

#include "models/sqltablemodel.h"

FilterSqlComboBox::FilterSqlComboBox(QWidget *parent)
    : SqlComboBox(parent)
{
    setInsertPolicy(QComboBox::NoInsert);
    setEditable(true);
    lineEdit()->setPlaceholderText("Поиск");

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(model());
    m_proxyModel->setFilterKeyColumn(modelColumn());
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_proxyModel1 = new QSortFilterProxyModel(this);
    m_proxyModel1->setSourceModel(model());
    m_proxyModel1->setFilterKeyColumn(modelColumn());
    m_proxyModel1->setFilterCaseSensitivity(Qt::CaseInsensitive);

    QComboBox::setModel(m_proxyModel1);

    m_completer = new QCompleter(this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setModel(m_proxyModel);
    m_completer->setCompletionColumn(modelColumn());
    m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setCompleter(m_completer);

    setModelColumn(0);

    connect(this, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        if (!text.isEmpty())
            m_proxyModel->setFilterFixedString(text);
    });
}

void FilterSqlComboBox::setModelColumn(int visibleColumn)
{
    QComboBox::setModelColumn(visibleColumn);
    m_proxyModel->setFilterKeyColumn(visibleColumn);
    m_proxyModel1->setFilterKeyColumn(visibleColumn);
    m_completer->setCompletionColumn(visibleColumn);
}

void FilterSqlComboBox::setSqlData(SqlTableModel *model,
                                   const QSqlRelation& sqlRelation)
{
    m_sqlModel = model;
    m_proxyModel->setSourceModel(m_sqlModel);
    m_proxyModel1->setSourceModel(m_sqlModel);
    m_sqlRelation = sqlRelation;
    int column = m_sqlModel->fieldIndex(
                     m_sqlRelation.displayColumn());
    m_proxyModel1->setFilterKeyColumn(column);
    m_proxyModel1->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(column);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    QComboBox::setModel(m_proxyModel1);
    setModelColumn(column);
}

QModelIndex FilterSqlComboBox::selectedIndex() const
{
    QModelIndex out;
    if (m_sqlModel) {
        if (!m_sqlRelation.tableName().isEmpty()) {
            out = m_sqlModel->index(currentIndex(),
                                      m_sqlModel->fieldIndex(
                                          m_sqlRelation.indexColumn()));
        } else {
            out = model()->index(currentIndex(), modelColumn());
        }
    }
    return out;
}

void FilterSqlComboBox::setPlaceholderText(const QString &placeholderText)
{
    lineEdit()->setPlaceholderText(placeholderText);
}
