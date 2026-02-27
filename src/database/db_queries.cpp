#include "db_queries.h"
#include "db_connection.h"
#include "../logging/logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QVariant>

bool DBQueries::selectAllPlates(QList<ne_plate>& plates)
{
    QSqlDatabase db = DBConnection::instance().getConnection();
    if (!db.isOpen()) {
        Logger::instance().error("Database not connected");
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM ne_plate_type")) {
        Logger::instance().error(QString("Query failed: %1").arg(query.lastError().text()));
        return false;
    }

    plates.clear();
    while (query.next()) {
        ne_plate plate;
        plate.pk_id = query.value("pk_id").toInt();
        plate.sn = query.value("sn").toString();
        plate.plate_type = query.value("plate_type").toString();
        plate.ip = query.value("ip").toString();
        plate.port = query.value("port").toInt();
        plate.timeout = query.value("timeout").toInt();
        plate.retry = query.value("retry").toInt();
        plates.append(plate);
    }

    Logger::instance().info(QString("Loaded %1 plates from database").arg(plates.size()));
    return true;
}

bool DBQueries::selectAllMetaInfo(QList<ne_md_info>& metaInfo)
{
    QSqlDatabase db = DBConnection::instance().getConnection();
    if (!db.isOpen()) {
        Logger::instance().error("Database not connected");
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM ne_md_info")) {
        Logger::instance().error(QString("Query failed: %1").arg(query.lastError().text()));
        return false;
    }

    metaInfo.clear();
    while (query.next()) {
        ne_md_info md;
        md.pk_id = query.value("pk_id").toInt();
        md.plate_id = query.value("plate_id").toInt();
        md.md_name = query.value("md_name").toString();
        md.md_type = query.value("md_type").toString();
        md.md_unit = query.value("md_unit").toString();
        md.min_value = query.value("min_value").toInt();
        md.max_value = query.value("max_value").toInt();
        md.current_value = query.value("current_value").toInt();

        if (query.record().indexOf("curValue_str") >= 0) {
            md.current_value_str = query.value("curValue_str").toString();
        } else if (query.record().indexOf("current_value_str") >= 0) {
            md.current_value_str = query.value("current_value_str").toString();
        }

        metaInfo.append(md);
    }

    Logger::instance().info(QString("Loaded %1 metadata from database").arg(metaInfo.size()));
    return true;
}

bool DBQueries::selectAllFlowInfo(QList<ne_flow_info>& flowInfo)
{
    QSqlDatabase db = DBConnection::instance().getConnection();
    if (!db.isOpen()) {
        Logger::instance().error("Database not connected");
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM ne_flow_info")) {
        Logger::instance().error(QString("Query failed: %1").arg(query.lastError().text()));
        return false;
    }

    flowInfo.clear();
    while (query.next()) {
        ne_flow_info flow;
        flow.pk_id = query.value("pk_id").toInt();
        flow.flow_name = query.value("flow_name").toString();
        flow.flow_type = query.value("flow_type").toString();
        flow.plate_id = query.value("plate_id").toInt();
        flowInfo.append(flow);
    }

    Logger::instance().info(QString("Loaded %1 flow info from database").arg(flowInfo.size()));
    return true;
}

bool DBQueries::updateMetadataValue(int metadataId, int value)
{
    return updateMetadataValues({qMakePair(metadataId, value)});
}

bool DBQueries::updateMetadataValues(const QList<QPair<int, int>>& updates)
{
    if (updates.isEmpty()) {
        return true;
    }

    QSqlDatabase db = DBConnection::instance().getConnection();
    if (!db.isOpen()) {
        Logger::instance().error("Database not connected");
        return false;
    }

    const bool hasTx = db.driver() && db.driver()->hasFeature(QSqlDriver::Transactions);
    if (hasTx) {
        db.transaction();
    }

    bool allOk = true;
    for (const auto& item : updates) {
        const int metadataId = item.first;
        const int value = item.second;

        QSqlQuery query(db);
        query.prepare("UPDATE ne_md_info SET current_value = ? WHERE pk_id = ?");
        query.addBindValue(value);
        query.addBindValue(metadataId);

        if (!query.exec()) {
            Logger::instance().error(QString("Update failed for md=%1: %2")
                                         .arg(metadataId)
                                         .arg(query.lastError().text()));
            allOk = false;
            break;
        }
    }

    if (hasTx) {
        if (allOk) {
            db.commit();
        } else {
            db.rollback();
        }
    }

    return allOk;
}
