#pragma once

#include <QApplication>

#include "settings.h"
#include "sqlsharedutils.h"

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int &argc, char **argv, int = ApplicationFlags);
    auto settings() -> Settings {
        return {};
    }
    static inline auto app() -> Application*
    {
        return static_cast<Application*>(qApp);
    }
private:
};

