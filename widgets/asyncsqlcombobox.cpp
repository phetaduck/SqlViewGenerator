#include "asyncsqlcombobox.h"

#include "utils/sqlsharedutils.h"

#include <QMovie>
#include <QBoxLayout>
#include <QLineEdit>

AsyncSqlComboBox::AsyncSqlComboBox(QWidget* parent) :
    QWidget(parent)
{
    m_sqlComboBox = std::make_unique<SqlComboBox>();
    m_waitLabel = std::make_unique<QLabel>();
    m_clearButton = std::make_unique<QToolButton>();
    m_reloadButton = std::make_unique<QToolButton>();

    m_clearButton->setIcon(DecorationCommon::closeIcon());
    m_clearButton->setIconSize({16, 16});
    m_clearButton->setFixedSize({24,
                                24});

    m_reloadButton->setIcon(DecorationCommon::closeIcon());
    m_reloadButton->setIconSize({16, 16});
    m_reloadButton->setFixedSize({24,
                                24});

    auto movie = new QMovie(DecorationCommon::waitLoader(), {}, waitLabel());
    waitLabel()->setMovie(movie);

    auto hLayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
    hLayout->setSpacing(0);
    setLayout(hLayout);

    layout()->addWidget(comboBox());
    layout()->addWidget(reloadButton());
    layout()->addWidget(clearButton());
    layout()->addWidget(waitLabel());

    if (!m_showClearButton) {
        m_clearButton->hide();
    }
    if (!m_showReloadButton) {
        m_clearButton->hide();
    }
    m_waitLabel->hide();

    connect(clearButton(), &QToolButton::clicked,
            this, [this]()
    {
        m_sqlComboBox->setCurrentIndex(-1);
    });

}

auto AsyncSqlComboBox::comboBox() const -> QComboBox*
{
    return sqlComboBox();
}

auto AsyncSqlComboBox::sqlComboBox() const -> SqlComboBox*
{
    return m_sqlComboBox.get();
}

auto AsyncSqlComboBox::waitLabel() const -> QLabel*
{
    return m_waitLabel.get();
}

auto AsyncSqlComboBox::clearButton() const -> QToolButton*
{
    return m_clearButton.get();
}

auto AsyncSqlComboBox::reloadButton() const -> QToolButton*
{
    return m_reloadButton.get();
}

auto AsyncSqlComboBox::sqlRelation() const -> const QSqlRelation& {
    return sqlComboBox()->sqlRelation();
}

void AsyncSqlComboBox::setSqlRelation(const QSqlRelation& relation) {
    auto model = ModelManager::sharedSqlTableModel<AsyncSqlTableModel>(relation.tableName());
    connect(model, &AsyncSqlTableModel::selectStarted,
            this, &AsyncSqlComboBox::onSyncStarted);
    connect(model, &AsyncSqlTableModel::selectFinished,
            this, &AsyncSqlComboBox::onSyncFinished);
    connect(model, &AsyncSqlTableModel::submitAllStarted,
            this, &AsyncSqlComboBox::onSyncStarted);
    connect(model, &AsyncSqlTableModel::submitAllFinished,
            this, &AsyncSqlComboBox::onSyncFinished);
    m_sqlComboBox->setSqlData(model, relation);
    if (!model->isSelectedAtLeastOnce()) {
        model->select();
    }
}

bool AsyncSqlComboBox::showClearButton() const
{
    return m_showClearButton;
}

void AsyncSqlComboBox::setShowClearButton(bool showClearButton)
{
    m_showClearButton = showClearButton;
    if (m_showClearButton) {
        m_clearButton->show();
    } else {
        m_clearButton->hide();
    }
}

void AsyncSqlComboBox::onSyncFinished()
{
    m_waitLabel->movie()->stop();
    m_sqlComboBox->show();
    if (m_showClearButton) {
        m_clearButton->show();
    }
    m_waitLabel->hide();
}

void AsyncSqlComboBox::onSyncStarted()
{
    m_sqlComboBox->hide();
    m_clearButton->hide();
    m_waitLabel->show();
    m_waitLabel->movie()->start();
}
