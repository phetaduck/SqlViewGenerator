#include "classifier_definitions.h"

auto Classifiers::LoadSqlTableDescriptors(const QString& filePath) -> TableDescriptorListType&
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
        JSON::fromJsonValue(doc.array(), Classifiers::KnownSqlTableDescriptors);
    }
    return Classifiers::KnownSqlTableDescriptors;
}

void Classifiers::SaveSqlTableDescriptors(const QString& filePath)
{
    /// @note temporary
    QJsonDocument doc;
    doc.setArray(JSON::toJsonValue(Classifiers::KnownSqlTableDescriptors).toArray());
    auto settings = Application::app()->settings();
    settings.setClassifiersPath(filePath);
    QFile file{filePath};
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts{&file};
        ts << QString::fromLocal8Bit(doc.toJson());
        file.close();
    }
}

auto Classifiers::GetSqlTableDescriptors() -> TableDescriptorListType&
{
    if (Classifiers::KnownSqlTableDescriptors.empty()) {
        auto settings = Application::app()->settings();
        return LoadSqlTableDescriptors(settings.getClassifiersPath());
    }
    return Classifiers::KnownSqlTableDescriptors;
}

void Classifiers::SaveSqlTableDescriptors(const Classifiers::TableDescriptorListType& descriptors,
                                          const QString& filePath)
{
    /// @note temporary
    QJsonDocument doc;
    doc.setArray(JSON::toJsonValue(descriptors).toArray());
    auto settings = Application::app()->settings();
    QFile file{filePath};
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts{&file};
        ts << QString::fromLocal8Bit(doc.toJson());
        file.close();
    }
}
