#include "tablecolumnview.h"
#include "ui_tablecolumnview.h"

TableColumnView::TableColumnView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableColumnView)
{
    ui->setupUi(this);
    connect(ui->tb_Add, &QToolButton::clicked,
            this, [this] ()
    {
        if (ui->tb_Add->text() == "+") {
            ui->tb_Add->setText("-");
            emit add(ui->le_Column->text());
        } else if (ui->tb_Add->text() == "-") {
            ui->tb_Add->setText("+");
            emit remove(ui->le_Column->text());
        }
    });
    connect(ui->le_Column, &QLineEdit::textEdited,
            this, [this] (const QString& text)
    {
        emit columnChanged(text);
    });
    connect(ui->le_Type, &QLineEdit::textEdited,
            this, [this] (const QString& text)
    {
        emit typeChanged(text);
    });
}

TableColumnView::~TableColumnView()
{
    delete ui;
}

QString TableColumnView::column() const
{
    return ui->le_Column->text();
}

QString TableColumnView::type() const
{
    return ui->le_Type->text();
}

void TableColumnView::setColumn(const QString& column)
{
    ui->le_Column->setText(column);
}

void TableColumnView::setType(const QString& column)
{
    ui->le_Type->setText(column);
}

bool TableColumnView::shouldAdd() const
{
    return ui->tb_Add->text() != "+" && !column().isEmpty() && !type().isEmpty();
}
