#include "viewsettings.h"
#include "ui_viewsettings.h"

ViewSettings::ViewSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewSettings)
{
    ui->setupUi(this);
    connectSettingsChanged();
}

ViewSettings::~ViewSettings()
{
    delete ui;
}

void ViewSettings::setSqlSettings(const SqlSettings& newSqlSettings)
{
    disconnectSettingsChanged();
    ui->cb_Owner->setCurrentText(newSqlSettings.owner);
    ui->cb_SortOrder->setCurrentText(newSqlSettings.sortOrder);

    ui->cb_TableFields->clear();
    for (const auto& value: newSqlSettings.fieldOrder) {
        ui->cb_TableFields->addItem(value.toString());
    }
    ui->cb_TableFields->setCurrentText(newSqlSettings.sortColumn);

    ui->cb_ViewSchema->setData(newSqlSettings.viewSchema);

    ui->le_ViewComment->setText(newSqlSettings.comment);
    ui->chb_DropView->setChecked(newSqlSettings.dropView);
    ui->chb_DropFunction->setChecked(newSqlSettings.dropFunctions);
    ui->chb_Sort->setChecked(newSqlSettings.sortEnabled);
    connectSettingsChanged();
}

auto ViewSettings::sqlSettings() -> SqlSettings
{
    SqlSettings newSqlSettings;
    newSqlSettings.comment = ui->le_ViewComment->text();
    newSqlSettings.dropFunctions = ui->chb_DropFunction->isChecked();
    newSqlSettings.dropView = ui->chb_DropView->isChecked();
    QVariantList fieldOrder;
    for (int i = 0; i < ui->cb_TableFields->count(); i++) {
        fieldOrder << ui->cb_TableFields->itemText(i);
    }
    newSqlSettings.fieldOrder = fieldOrder;
    newSqlSettings.owner = ui->cb_Owner->currentText();
    newSqlSettings.primaryKey = "id";
    newSqlSettings.sortColumn = ui->cb_TableFields->currentText();
    newSqlSettings.viewSchema = ui->cb_ViewSchema->sqlComboBox()->currentText();
    newSqlSettings.sortOrder = ui->cb_SortOrder->currentText();
    newSqlSettings.sortEnabled = ui->chb_Sort->isChecked();
    return newSqlSettings;
}

auto ViewSettings::viewSchemaCb() -> AsyncSqlComboBox*
{
    return ui->cb_ViewSchema;
}

void ViewSettings::connectSettingsChanged()
{
    connect(ui->cb_Owner, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    connect(ui->cb_TableFields, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    connect(ui->cb_SortOrder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    connect(ui->cb_ViewSchema->sqlComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    connect(ui->chb_DropView, &QCheckBox::stateChanged,
            this, &ViewSettings::emitSqlSettings);
    connect(ui->chb_DropFunction, &QCheckBox::stateChanged,
            this, &ViewSettings::emitSqlSettings);
    connect(ui->chb_Sort, &QCheckBox::stateChanged,
            this, &ViewSettings::emitSqlSettings);
    connect(ui->le_ViewComment, &QLineEdit::textEdited,
            this, &ViewSettings::emitSqlSettings);
}

void ViewSettings::disconnectSettingsChanged()
{
    disconnect(ui->cb_Owner, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->cb_TableFields, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->cb_SortOrder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->cb_ViewSchema->sqlComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->chb_DropFunction, &QCheckBox::stateChanged,
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->chb_DropFunction, &QCheckBox::stateChanged,
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->chb_Sort, &QCheckBox::stateChanged,
            this, &ViewSettings::emitSqlSettings);
    disconnect(ui->le_ViewComment, &QLineEdit::textEdited,
            this, &ViewSettings::emitSqlSettings);
}

void ViewSettings::emitSqlSettings()
{
    emit sqlSettingsChanged(sqlSettings());
}
