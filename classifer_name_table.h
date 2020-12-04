#pragma once

#include <QWidget>

#include "classifier_definitions.h"

namespace Ui {
class Classifer_Name_Table;
}

class Classifer_Name_Table : public QWidget
{
    Q_OBJECT

public:
    explicit Classifer_Name_Table(QWidget *parent = nullptr);
    ~Classifer_Name_Table();

    void setClassifierData(const QString& name,
                           const Classifiers::SqlTableDescriptor& classifierDef,
                           int position);

private:
    Ui::Classifer_Name_Table *ui;
};

