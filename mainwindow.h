#pragma once

#include <QMainWindow>
#include <unordered_map>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TableColumnView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::unordered_map<QObject*, std::shared_ptr<TableColumnView>> m_columnViews;
    void updateSqlScript();
    QSqlDatabase dbconn;
};
