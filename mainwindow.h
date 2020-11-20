#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <unordered_map>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TableColumnView;
class QSyntaxHighlighter;

template <typename T, typename K = QObject*>
using ObjectHash = std::unordered_map<K, std::shared_ptr<T>>;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void saveSqlSlot();
    void runSqlSlot();
    void openSqlSlot();

protected:
    virtual void initFields();
    virtual void connectSignals();

private:
    Ui::MainWindow *ui;
    ObjectHash<TableColumnView> m_columnViews;
    ObjectHash<QSyntaxHighlighter> m_syntaxHighlighters;

    void updateSqlScript(const QString& table);

    QSqlDatabase dbconn;
};
