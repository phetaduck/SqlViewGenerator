#include "classifier_definitions.h"

auto Classifiers::LoadClassifiers(const QString& filePath) -> decltype (Classifiers::KnownClassifiers)&
{
    auto settings = Application::app()->settings();
    settings.setClassifiersPath(filePath);
    QFile file{filePath};
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts{&file};
        ts.setCodec("UTF-8");
        auto all = ts.readAll();
        auto data = all.toUtf8();
        auto doc = QJsonDocument::fromJson(data);
        file.close();
        JSON::fromJsonValue(doc.array(), Classifiers::KnownClassifiers);
    }
    return Classifiers::KnownClassifiers;
}

void Classifiers::SaveClassifiers(const QString& filePath)
{
    /// @note temporary
    QJsonDocument doc;
    doc.setArray(JSON::toJsonValue(Classifiers::KnownClassifiers).toArray());
    auto settings = Application::app()->settings();
    settings.setClassifiersPath(filePath);
    QFile file{filePath};
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts{&file};
        ts << QString::fromLocal8Bit(doc.toJson());
        file.close();
    }
}

auto Classifiers::GetClassifiers() -> decltype (Classifiers::KnownClassifiers)&
{
    if (Classifiers::KnownClassifiers.empty()) {
        auto settings = Application::app()->settings();
        return LoadClassifiers(settings.getClassifiersPath());
    }
    return Classifiers::KnownClassifiers;
}
