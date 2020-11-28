#pragma once

#include <QWidget>

#include "utils/sqlviewgenerator.h"

namespace Ui {
class ViewSettings;
}

class AsyncSqlComboBox;

class ViewSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ViewSettings(QWidget *parent = nullptr);
    ~ViewSettings();

    void setSqlSettings(const SqlSettings& newSqlSettings);
    auto sqlSettings() -> SqlSettings;

    auto viewSchemaCb() -> AsyncSqlComboBox*;

public slots:
    void connectSettingsChanged();
    void disconnectSettingsChanged();

    void emitSqlSettings();

signals:

    void sqlSettingsChanged(const SqlSettings& newSqlSettings);

private:
    Ui::ViewSettings *ui;
};
