#pragma once

#include <QStyledItemDelegate>

class QTableView;

/**
 * @class BaseItemDelegate
 * Базовый клас для кастомных делегатов
 */
class BaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    /**
     * @brief унаследованные конструкторы */
    using QStyledItemDelegate::QStyledItemDelegate;

    /** перегруженный метод базового класса */
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    QSize sizeHint(
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

protected:
    /** @brief Получает текст для отрисовки. Нужен для перегрузки в классах наследниках
         * @param index индекс для получения данных из модели
         * @return строку для отрисовки
 */
    virtual QString getText(const QModelIndex& index) const;
};
