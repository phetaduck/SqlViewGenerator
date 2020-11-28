#pragma once

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QSqlRelation>

#include <memory>

#include "sqlcombobox.h"

class AsyncSqlComboBox : public QWidget
{
    Q_OBJECT
public:
    explicit AsyncSqlComboBox(QWidget* parent = nullptr);

    auto comboBox() const -> QComboBox*;

    auto sqlComboBox() const -> SqlComboBox*;

    auto waitLabel() const -> QLabel*;

    auto clearButton() const -> QToolButton*;

    auto reloadButton() const -> QToolButton*;

    void setSqlModel(AsyncSqlTableModel* model);

    auto sqlRelation() const -> const QSqlRelation&;
    void setSqlRelation(const QSqlRelation& relation);

    bool showClearButton() const;
    void setShowClearButton(bool showClearButton);

    bool showReloadButton() const;
    void setShowReloadButton(bool showReloadButton);

public slots:
    void setData(const QVariant& data);

private slots:
    void onSyncFinished();
    void onSyncStarted();
signals:

private:
    std::unique_ptr<SqlComboBox> m_sqlComboBox = nullptr;
    std::unique_ptr<QLabel> m_waitLabel = nullptr;
    std::unique_ptr<QToolButton> m_reloadButton = nullptr;
    std::unique_ptr<QToolButton> m_clearButton = nullptr;

    QVariant m_selectedData;

    bool m_showClearButton = false;
    bool m_showReloadButton = false;
};
