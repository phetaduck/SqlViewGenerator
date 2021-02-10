#pragma once

#include <QComboBox>
#include <QSqlRelation>
#include "utils/modelmanager.h"

class SqlTableModel;

class SqlComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QVariant data READ data WRITE setData);
    Q_PROPERTY(QSqlRelation sqlRelation READ sqlRelation WRITE setSqlRelation NOTIFY sqlRelationChanged)
public:

    using QComboBox::QComboBox;

    void setModel(QAbstractItemModel* model);

    virtual QModelIndex selectedIndex() const;

    virtual QModelIndex selectedIndex(int column) const;

    virtual QModelIndex selectedIndex(const QString& fieldName) const;

    void setSelectedIndex(const QVariant& data);

    void setCurrentIndex(int index);
    void setCurrentText(const QString &text);

    virtual SqlTableModel* sqlModel();

    virtual auto sqlRelation() const -> const QSqlRelation&;

    virtual void setSqlData(SqlTableModel* sqlModel,
                    const QSqlRelation& sqlRelation);

    virtual QVariant data();
    virtual void setData(const QVariant& data);

    void setSqlModel(SqlTableModel* sqlModel);

public slots:
    void setSqlRelation(const QSqlRelation& sqlRelation);

signals:
    void sqlRelationChanged(QSqlRelation sqlRelation);

protected:

    QSqlRelation m_sqlRelation;
    SqlTableModel* m_sqlModel = nullptr;

    int m_lastSelectedIndex = -1;
    QString m_lastSelectedItem = {};
    QVariant m_lastSelectedData = {};
};
