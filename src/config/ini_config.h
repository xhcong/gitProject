#ifndef INI_CONFIG_H
#define INI_CONFIG_H

#include <QString>
#include "config_info.h"

/**
 * @brief INI configuration file parser
 */
class IniConfig
{
public:
    /**
     * @brief Load configuration from INI file
     */
    static bool loadConfig(const QString& filePath, IniConfigInfo& config);

    /**
     * @brief Save configuration to INI file
     */
    static bool saveConfig(const QString& filePath, const IniConfigInfo& config);

private:
    IniConfig() = delete;
};

#endif // INI_CONFIG_H
