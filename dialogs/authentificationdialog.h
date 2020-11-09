#pragma once

#include <QDialog>

namespace Ui {
class AuthentificationDialog;
}

class AuthentificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthentificationDialog(QWidget *parent = nullptr);
    ~AuthentificationDialog();

    auto login() const -> QString;
    auto password() const -> QString;
    auto host() const -> QString;
    auto dbType() const -> QString;
    auto port() const -> int;

private:
    Ui::AuthentificationDialog *ui;
};
