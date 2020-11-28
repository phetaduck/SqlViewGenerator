#include "sqlviewgenerator.h"


QString SqlViewGenerator::viewSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString fullTableName = settings.tableSchema + "." + settings.table;
    QString out;
    QTextStream out_ts (&out);
    if (settings.dropView) {
        out_ts << "--";
    }
    out_ts << "DROP VIEW IF EXISTS " << viewName << ";\n\n";
    out_ts << "CREATE OR REPLACE VIEW " << viewName << " AS\n" << "SELECT\n";

    for (int i = 0; i < settings.fieldOrder.size(); i++) {
        auto fieldName = settings.fieldOrder.at(i).toString();
        auto field = settings.record.field(fieldName);
        if (field.name().isEmpty()) continue;
        if (field.name() == "deleted_at") {
            out_ts << "\t" << "(" << settings.table << "." << field.name() <<
                      " IS NOT NULL" << ")" << " AS deleted";
            if (i < settings.fieldOrder.count() - 1) {
                out_ts << ",";
            }
            out_ts << "\n";
        } else {
            out_ts << "\t" << settings.table << "." << field.name();
            if (i < settings.fieldOrder.count() - 1) {
                out_ts << ",";
            }
            out_ts << "\n";
        }
    }

    out_ts << "FROM " << fullTableName;
    if (settings.sortEnabled && !settings.sortColumn.isEmpty()) {
        out_ts << "\n" << "ORDER BY " << settings.sortColumn << " ";
        out_ts << settings.sortOrder;
    }
    out_ts << ";\n\n";

    if (!settings.comment.isEmpty()) {
        out_ts << "COMMENT ON VIEW " << viewName << "\n" <<
                  "\t" << "IS '" << settings.comment << "';\n\n";
    }

    if (!settings.owner.isEmpty()) {
        out_ts << "ALTER VIEW " << viewName << "\n" <<
                  "\t" << "OWNER TO " << settings.owner << ";\n\n";
    }

    out_ts << "GRANT SELECT ON TABLE " << viewName << " TO project_admin;\n";
    out_ts << "GRANT ALL ON TABLE " << viewName << " TO admin;\n";
    out_ts << "GRANT SELECT, DELETE ON TABLE " << viewName << " TO db_delete;\n";
    out_ts << "GRANT INSERT, SELECT, UPDATE ON TABLE " << viewName << " TO catalog_admin;\n\n";

    return out;
}

QString SqlViewGenerator::insetFunctionSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString fullTableName = settings.tableSchema + "." + settings.table;
    QString insertFunctionText;
    QTextStream ifsText (&insertFunctionText);
    QString insertFunctionName = viewName + "_insert ()";

    if (settings.dropFunctions) {
        ifsText << "--";
    }
    ifsText << "DROP FUNCTION IF EXISTS " << insertFunctionName << ";\n\n";
    ifsText << "CREATE OR REPLACE FUNCTION " << insertFunctionName << "\n";
    ifsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n" <<
               "DECLARE new_id integer;\n" <<
               "DECLARE new_row record;\n";
    ifsText << "BEGIN" << "\n";
    ifsText << "\t" << "INSERT INTO " << fullTableName << "\n\t(\n";

    for (int i = 0; i < settings.fieldOrder.size(); i++) {
        auto fieldName = settings.fieldOrder.at(i).toString();
        auto field = settings.record.field(fieldName);
        if (field.name().isEmpty()) continue;

        if (field.name() != settings.primaryKey && field.name() != "deleted_at") {
            ifsText << "\t" << "\t" << field.name();
            if (i < settings.fieldOrder.count() - 1
                    && !(i+1 == settings.fieldOrder.count() - 1
                         && settings.fieldOrder.at(i+1) == "deleted_at"))
            {
                ifsText << ",";
            }
            ifsText << "\n";
        }
    }

    ifsText << "\t" << ") VALUES (\n";

    for (int i = 0; i < settings.fieldOrder.size(); i++) {
        auto fieldName = settings.fieldOrder.at(i).toString();
        auto field = settings.record.field(fieldName);
        if (field.name().isEmpty()) continue;

        if (field.name() != settings.primaryKey && field.name() != "deleted_at") {
            ifsText << "\t" << "\t" << "NEW." << field.name();
            if (i < settings.fieldOrder.count() - 1
                    && !(i+1 == settings.fieldOrder.count() - 1
                         && settings.fieldOrder.at(i+1) == "deleted_at")) {
                ifsText << ",";
            }
            ifsText << "\n";
        }
    }

    ifsText << "\t" << ")\n" << "\t" << "RETURNING " << settings.primaryKey << " INTO new_id;\n\n";
    ifsText << "SELECT * INTO new_row FROM " << viewName << " WHERE " <<
               settings.primaryKey << " = new_id;\n\n";
    ifsText << "RETURN new_row;\n\nEXCEPTION WHEN unique_violation THEN\n" <<
               "RAISE EXCEPTION 'record already exists';\n\n";
    ifsText << "END;\n" << "$BODY$;" << "\n\n";

    if (!settings.owner.isEmpty()) {
        ifsText << "ALTER FUNCTION " << insertFunctionName << "\n" <<
                   "\t" << "OWNER TO " << settings.owner << ";\n\n";
    }

    return insertFunctionText;
}

QString SqlViewGenerator::insetTriggerSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString iTrgText;
    QTextStream iTrgTS (&iTrgText);
    QString iTrgName = settings.table + "_insert_trg";
    QString insertFunctionName = viewName + "_insert ()";

    iTrgTS << "CREATE TRIGGER " << iTrgName << "\n";
    iTrgTS << "\t"  << "INSTEAD OF INSERT" << "\n";
    iTrgTS << "\t"  << "ON " << viewName << "\n";
    iTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    iTrgTS << "\t"  << "EXECUTE PROCEDURE " << insertFunctionName << ";" << "\n" << "\n";

    return iTrgText;
}

QString SqlViewGenerator::updateFunctionSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString fullTableName = settings.tableSchema + "." + settings.table;

    QString updateFunctionText;
    QTextStream ufsText (&updateFunctionText);
    QString updateFunctionName = viewName + "_update ()";

    if (settings.dropFunctions) {
        ufsText << "--";
    }
    ufsText << "DROP FUNCTION IF EXISTS " << updateFunctionName << ";\n\n";
    ufsText << "CREATE OR REPLACE FUNCTION " << updateFunctionName << "\n";
    ufsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n";

    if (settings.record.contains("deleted_at")) {
        ufsText << "DECLARE deleted_timestamp timestamp with time zone;\n";
    }
    ufsText << "BEGIN" << "\n";
    if (settings.record.contains("deleted_at")) {
        ufsText << "\t" << "IF NEW.deleted AND OLD.deleted THEN" << "\n"
                << "\t" << "\t"
                << "deleted_timestamp := (SELECT deleted_at FROM "
                << fullTableName << " WHERE " << settings.primaryKey
                << " = OLD.id);" << "\n"
                << "\t" << "ELSIF NEW.deleted THEN" << "\n"
                << "\t" << "\t" << "deleted_timestamp := NOW();"
                << "\n" << "\t" << "ELSE" << "\n"
                << "\t" << "\t" << "deleted_timestamp := NULL;"
                << "\n" << "\t" << "END IF;" << "\n\n";
    }
    ufsText << "\t" << "UPDATE "
            << fullTableName
            << "\n" << "\t" << " SET \n";

    for (int i = 0; i < settings.fieldOrder.size(); i++) {
        auto fieldName = settings.fieldOrder.at(i).toString();
        auto field = settings.record.field(fieldName);
        if (field.name().isEmpty()) continue;

        if (field.name() != settings.primaryKey) {
            ufsText << "\t" << "\t" << field.name() << " = ";
            if (field.name() == "deleted_at") {
                ufsText << "deleted_timestamp";
            } else {
                ufsText << "NEW." << field.name();
            }
            if (i < settings.fieldOrder.count() - 1) {
                ufsText << ",";
            }
            ufsText << "\n";
        }
    }

    ufsText << "\t" << "\n";
    ufsText << "\t" << "WHERE  "
            << settings.primaryKey
            << " = NEW." << settings.primaryKey << ";\n\n";
    ufsText << "\t" << "RETURN NEW;\n\n";
    ufsText << "END;\n" << "$BODY$;" << "\n\n";

    if (!settings.owner.isEmpty()) {
        ufsText << "ALTER FUNCTION " << updateFunctionName << "\n" <<
                   "\t" << "OWNER TO " << settings.owner << ";\n\n";
    }

    return updateFunctionText;
}

QString SqlViewGenerator::updateTriggerSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString uTrgText;
    QTextStream uTrgTS (&uTrgText);
    QString uTrgName = settings.table + "_update_trg";
    QString updateFunctionName = viewName + "_update ()";

    uTrgTS << "CREATE TRIGGER " << uTrgName << "\n";
    uTrgTS << "\t"  << "INSTEAD OF UPDATE" << "\n";
    uTrgTS << "\t"  << "ON " << viewName << "\n";
    uTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    uTrgTS << "\t"  << "EXECUTE PROCEDURE " << updateFunctionName << ";" << "\n" << "\n";

    return uTrgText;
}

QString SqlViewGenerator::deleteFunctionSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString fullTableName = settings.tableSchema + "." + settings.table;
    QString deleteFunctionText;
    QTextStream dfsText (&deleteFunctionText);
    QString deleteFunctionName = viewName + "_delete ()";

    if (settings.dropFunctions) {
        dfsText << "--";
    }
    dfsText << "DROP FUNCTION IF EXISTS " << deleteFunctionName << ";\n\n";
    dfsText << "CREATE OR REPLACE FUNCTION " << deleteFunctionName << "\n";
    dfsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n" <<
               "BEGIN" << "\n";

    if (settings.record.contains("deleted_at")) {
        dfsText << "\t" << "UPDATE " << fullTableName << " SET deleted_at = NOW()" <<
                   " WHERE " << settings.primaryKey << " = OLD." << settings.primaryKey << ";\n";
    } else {
        dfsText << "\t" << "DELETE FROM " << fullTableName <<
                   " WHERE " << settings.primaryKey << " = OLD." << settings.primaryKey << ";\n";
    }
    dfsText << "\t" << "RETURN NULL;\n\n";
    dfsText << "END;\n" << "$BODY$;" << "\n\n";
    if (!settings.owner.isEmpty()) {
        dfsText << "ALTER FUNCTION " << deleteFunctionName << "\n" <<
                   "\t" << "OWNER TO " << settings.owner << ";\n\n";
    }

    return deleteFunctionText;
}

QString SqlViewGenerator::deleteTriggerSql(const SqlSettings& settings)
{
    QString viewName = settings.viewSchema + "." + settings.table;
    QString deleteFunctionName = viewName + "_delete ()";

    QString dTrgText;
    QTextStream dTrgTS (&dTrgText);
    QString dTrgName = settings.table + "_delete_trg";

    dTrgTS << "CREATE TRIGGER " << dTrgName << "\n";
    dTrgTS << "\t"  << "INSTEAD OF DELETE" << "\n";
    dTrgTS << "\t"  << "ON " << viewName << "\n";
    dTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    dTrgTS << "\t"  << "EXECUTE PROCEDURE " << deleteFunctionName << ";" << "\n" << "\n";

    return dTrgText;
}
