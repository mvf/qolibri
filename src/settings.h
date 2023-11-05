#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

// QSettings whose INI codec defaults to UTF-8
class Settings : public QSettings
{
public:
    explicit Settings(const QString &application);
};

#endif // SETTINGS_H
