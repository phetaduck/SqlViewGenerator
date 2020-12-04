#include "classifier_list.h"

#include "classifer_name_table.h"

#include <QBoxLayout>
#include <QScrollArea>

Classifier_List::Classifier_List(QWidget *parent)
    : QGroupBox(parent)
{
    setLayout(new QVBoxLayout());

    sArea = new QScrollArea();
    sArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    sArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    sArea->setWidgetResizable( true );
    sArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    auto frame = new QWidget();
    frame->setLayout(new QVBoxLayout());

    sArea->setWidget(frame);

    layout()->addWidget(sArea);
}

void Classifier_List::setClassifierList(
        const Classifiers::TableDescriptorListType& list)
{
    for (const auto& [key, widget] : m_widgets) {
        (void) key;
        sArea->widget()->layout()->removeWidget(widget.get());
    }
    m_widgets.clear();
    int pos = 0;
    for (const auto& value : list) {
        auto widget = std::make_shared<Classifer_Name_Table>();
        widget->setClassifierData(value.first, value.second, pos++);
        m_widgets[value.first] = widget;
        sArea->widget()->layout()->addWidget(widget.get());
    }
}
