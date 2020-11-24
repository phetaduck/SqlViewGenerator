#pragma once

#include "baseitemdelegate.h"
#include "functional"

/**
 * @class StyledItemDelegate
 * Делегат с кастомным рисованием и валидацие
 */
class StyledItemDelegate : public BaseItemDelegate
{
    Q_OBJECT
public:
    /**
     * @brief унаследованные конструкторы */
    using BaseItemDelegate::BaseItemDelegate;

    /** перегруженный метод базового класса */
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    std::function<bool(const QModelIndex&)> IsValid; ///< Если вернет false - нарисует красным фон
    std::function<QString(const QVariant& value)> Format; ///< Лямбда форматирования

    int textAlignmentFlags() const;
    void setTextAlignmentFlags(int textAlignmentFlags);

private:
    int m_textAlignmentFlags = Qt::AlignLeft |
                               Qt::AlignVCenter |
                               Qt::TextWordWrap; ///< Флаги выравнивание текста
};

