#pragma once

#include <QMainWindow>
#include <unordered_map>
#include <QSqlDatabase>
#include <QLocalSocket>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TableColumnView;
class SqlTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::unordered_map<QObject*, std::shared_ptr<TableColumnView>> m_columnViews;
    std::unique_ptr<QLocalSocket> m_pipeConnection = nullptr;
    SqlTableModel* m_relationModel;


    void updateSqlScript(const QString& table);

    QSqlDatabase dbconn;
};
