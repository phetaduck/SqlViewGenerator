#pragma once

#include <QWidget>

namespace Ui {
class UpdateViews;
}

class UpdateViews : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateViews(QWidget *parent = nullptr);
    ~UpdateViews();

    void connectSignals();
public slots:
    void openJsonSlot();
    void saveJsonSlot();
    void openCsvSlot();

private:
    Ui::UpdateViews *ui;
};

