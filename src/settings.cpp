#include "settings.h"
#include "configure.h"

Settings::Settings(const QString &application)
: QSettings(CONF->settingOrg, application)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    setIniCodec("UTF-8");
#endif
}
