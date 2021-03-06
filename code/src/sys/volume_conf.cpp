
#include "onyx/base/base.h"
#include "onyx/sys/volume_conf.h"
#include "onyx/sys/platform.h"


namespace sys
{


VolumeConfig::VolumeConfig()
{
}

VolumeConfig::~VolumeConfig()
{
}

bool VolumeConfig::makeSureTableExist(QSqlDatabase &database)
{
    QSqlQuery query(database);
    bool ok = query.exec("create table if not exists volume ("
                         "key text primary key, "
                         "value text "
                         ")");
    if (ok)
    {
        return query.exec("create index if not exists key_index on volume (key) ");
    }
    return false;
}

int VolumeConfig::volume(QSqlDatabase &database)
{
    QSqlQuery query(database);
    query.prepare( "select value from volume where key = ? ");
    query.addBindValue("volume");
    int volume = -1;
    if (query.exec() && query.next())
    {
        volume = query.value(0).toInt();
    }
    return volume;
}

bool VolumeConfig::setVolume(QSqlDatabase &database, int volume)
{
    QSqlQuery query(database);
    query.prepare( "INSERT OR REPLACE into volume (key, value) values(?, ?)");
    query.addBindValue("volume");
    query.addBindValue(volume);
    return query.exec();
}

QVector<int> VolumeConfig::volumes()
{
    static QVector<int> VOLUMES;
    if (VOLUMES.size() <= 0)
    {
        if(sys::soundModule().compare("snd_soc_imx_3stack_wm8976") == 0 ||
           sys::soundModule().compare("snd-soc-booxe") == 0)
        {
            qDebug()<<"******************** M90/M91 platform *****************************"<<__TIME__;
            VOLUMES.push_back(0);
            VOLUMES.push_back(15);
            VOLUMES.push_back(22);
            VOLUMES.push_back(29);
            VOLUMES.push_back(36);
            VOLUMES.push_back(43);
            VOLUMES.push_back(50);
            VOLUMES.push_back(59);
            VOLUMES.push_back(68);
            VOLUMES.push_back(77);
            VOLUMES.push_back(85);
        }
        else
        {
            qDebug()<<"******************** other platform *****************************"<<__TIME__;
            VOLUMES.push_back(0);
            VOLUMES.push_back(60);
            VOLUMES.push_back(64);
            VOLUMES.push_back(68);
            VOLUMES.push_back(72);
            VOLUMES.push_back(76);
            VOLUMES.push_back(80);
            VOLUMES.push_back(85);
            VOLUMES.push_back(90);
            VOLUMES.push_back(95);
            VOLUMES.push_back(100);
        }
    }

    return VOLUMES;
}

int VolumeConfig::minVolume()
{
    return 0;
}

int VolumeConfig::maxVolume()
{
    return 100;
}

bool VolumeConfig::mute(QSqlDatabase &database, bool m)
{
    QSqlQuery query(database);
    query.prepare( "INSERT OR REPLACE into volume (key, value) values(?, ?)");
    query.addBindValue("mute");
    query.addBindValue(m);
    return query.exec();
}

bool VolumeConfig::isMute(QSqlDatabase &database)
{
    QSqlQuery query(database);
    query.prepare( "select value from volume where key = ? ");
    query.addBindValue("mute");
    bool mute = false;
    if (query.exec() && query.next())
    {
        mute = query.value(0).toBool();
    }
    return mute;
}

}
