#pragma once

#include <QGroupBox>

#include <unordered_map>
#include <memory>

#include "classifier_definitions.h"

class QScrollArea;

class Classifier_List : public QGroupBox
{
    Q_OBJECT
public:
    explicit Classifier_List(QWidget *parent = nullptr);

    void setClassifierList(
            const Classifiers::TableDescriptorListType& list);
private:
    std::unordered_map<QString, std::shared_ptr<QWidget>> m_widgets;
    QScrollArea* sArea = nullptr;
};

