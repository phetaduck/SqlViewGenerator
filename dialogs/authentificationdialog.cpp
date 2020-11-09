#include "authentificationdialog.h"
#include "ui_authentificationdialog.h"
#include "application.h"
#include "settings.h"

AuthentificationDialog::AuthentificationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthentificationDialog)
{
    ui->setupUi(this);
    auto settings = Application::app()->settings();
    ui->le_Host->setText(settings.dbHostName());
    ui->le_User->setText(settings.lastLogin());
    ui->le_Password->setText(settings.dbPass());
    ui->sb_Port->setValue(settings.dbPort());
    ui->cb_Type->setCurrentText(settings.dbType());
}

AuthentificationDialog::~AuthentificationDialog()
{
    delete ui;
}

auto AuthentificationDialog::login() const -> QString
{
    return ui->le_User->text();
}

auto AuthentificationDialog::password() const -> QString
{
    return ui->le_Password->text();
}

auto AuthentificationDialog::host() const -> QString
{
    return ui->le_Host->text();
}

auto AuthentificationDialog::dbType() const -> QString
{
    return ui->cb_Type->currentText();
}

auto AuthentificationDialog::port() const -> int
{
    return ui->sb_Port->value();
}
