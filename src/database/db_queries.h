#ifndef DB_QUERIES_H
#define DB_QUERIES_H

#include <QString>
#include <QList>
#include "data_structures.h"

/**
 * @brief Database query operations
 */
class DBQueries
{
public:
    /**
     * @brief Load all plates from database
     */
    static bool selectAllPlates(QList<ne_plate>& plates);

    /**
     * @brief Load all metadata info from database
     */
    static bool selectAllMetaInfo(QList<ne_md_info>& metaInfo);

    /**
     * @brief Load all flow info from database
     */
    static bool selectAllFlowInfo(QList<ne_flow_info>& flowInfo);

    /**
     * @brief Update metadata value in database
     */
    static bool updateMetadataValue(int metadataId, int value);

    /**
     * @brief Batch update metadata values in database
     */
    static bool updateMetadataValues(const QList<QPair<int, int>>& updates);

private:
    DBQueries() = delete;
};

#endif // DB_QUERIES_H
