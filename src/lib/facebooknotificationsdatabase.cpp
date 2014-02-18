/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Antti Seppälä <antti.seppala@jollamobile.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "facebooknotificationsdatabase.h"
#include "abstractsocialcachedatabase_p.h"
#include "socialsyncinterface.h"

#include <QtSql/QSqlError>
#include <QtCore/QtDebug>

static const char *DB_NAME = "facebookNotifications.db";
static const int VERSION = 1;

struct FacebookNotificationPrivate
{
    explicit FacebookNotificationPrivate(const QString &fbId, const QString &from, const QString &to,
                                         const QString &createdTime, const QString &updatedTime,
                                         const QString &title, const QString &link,
                                         int accountId);

    QString m_fbId;
    QString m_from;
    QString m_to;
    QString m_createdTime;
    QString m_updatedTime;
    QString m_title;
    QString m_link;
    int m_accountId;
};

FacebookNotificationPrivate::FacebookNotificationPrivate(const QString &fbId, const QString &from, const QString &to,
                                                         const QString &createdTime, const QString &updatedTime,
                                                         const QString &title, const QString &link,
                                                         int accountId)
    : m_fbId(fbId)
    , m_from(from)
    , m_to(to)
    , m_createdTime(createdTime)
    , m_updatedTime(updatedTime)
    , m_title(title)
    , m_link(link)
    , m_accountId(accountId)
{
}

FacebookNotification::FacebookNotification(const QString &fbId, const QString &from, const QString &to,
                              const QString &createdTime, const QString &updatedTime,
                              const QString &title, const QString &link,
                              int accountId)
    : d_ptr(new FacebookNotificationPrivate(fbId, from, to, createdTime, updatedTime, title, link, accountId))
{
}

FacebookNotification::Ptr FacebookNotification::create(const QString &fbId, const QString &from, const QString &to,
                                                       const QString &createdTime, const QString &updatedTime,
                                                       const QString &title, const QString &link,
                                                       int accountId)
{
    return FacebookNotification::Ptr(new FacebookNotification(fbId, from, to, createdTime, updatedTime, title, link, accountId));
}

FacebookNotification::~FacebookNotification()
{
}

QString FacebookNotification::fbId() const
{
    Q_D(const FacebookNotification);
    return d->m_fbId;
}

QString FacebookNotification::from() const
{
    Q_D(const FacebookNotification);
    return d->m_from;
}

QString FacebookNotification::to() const
{
    Q_D(const FacebookNotification);
    return d->m_to;
}

QString FacebookNotification::createdTime() const
{
    Q_D(const FacebookNotification);
    return d->m_createdTime;
}

QString FacebookNotification::updatedTime() const
{
    Q_D(const FacebookNotification);
    return d->m_updatedTime;
}

QString FacebookNotification::title() const
{
    Q_D(const FacebookNotification);
    return d->m_title;
}

QString FacebookNotification::link() const
{
    Q_D(const FacebookNotification);
    return d->m_link;
}

int FacebookNotification::accountId() const
{
    Q_D(const FacebookNotification);
    return d->m_accountId;
}

class FacebookNotificationsDatabasePrivate: public AbstractSocialCacheDatabasePrivate
{
public:
    explicit FacebookNotificationsDatabasePrivate(FacebookNotificationsDatabase *q);

    QMap<int, QList<FacebookNotification::ConstPtr> > insertNotifications;
    QMap<int, QList<FacebookNotification::ConstPtr> > removeNotifications;

    struct {
        QMap<int, QList<FacebookNotification::ConstPtr> > insertNotifications;
        QMap<int, QList<FacebookNotification::ConstPtr> > removeNotifications;
    } queue;
};

FacebookNotificationsDatabasePrivate::FacebookNotificationsDatabasePrivate(FacebookNotificationsDatabase *q)
    : AbstractSocialCacheDatabasePrivate(
            q,
            SocialSyncInterface::socialNetwork(SocialSyncInterface::Facebook),
            SocialSyncInterface::dataType(SocialSyncInterface::Notifications),
            QLatin1String(DB_NAME),
            VERSION)
{
}

FacebookNotificationsDatabase::FacebookNotificationsDatabase()
    : AbstractSocialCacheDatabase(*(new FacebookNotificationsDatabasePrivate(this)))
{
}

FacebookNotificationsDatabase::~FacebookNotificationsDatabase()
{
    wait();
}

void FacebookNotificationsDatabase::addFacebookNotification(const QString &id, const QString &from, const QString &to,
                                                            const QString &createdTime, const QString &updatedTime,
                                                            const QString &title, const QString &link,
                                                            int accountId)
{
    Q_D(FacebookNotificationsDatabase);
    d->insertNotifications[accountId].append(FacebookNotification::create(id, from, to, createdTime, updatedTime, title, link, accountId));
}

void FacebookNotificationsDatabase::removeNotifcations(int accountId)
{
    Q_UNUSED(accountId);
    // TODO...
}

void FacebookNotificationsDatabase::sync()
{
    Q_D(FacebookNotificationsDatabase);

    {
        QMutexLocker locker(&d->mutex);
        Q_FOREACH(int accountId, d->insertNotifications.keys()) {
            d->queue.insertNotifications.insert(accountId, d->insertNotifications.take(accountId));
        }
        Q_FOREACH(int accountId, d->removeNotifications.keys()) {
            d->queue.removeNotifications.insert(accountId, d->removeNotifications.take(accountId));
        }
    }

    executeWrite();
}

QList<FacebookNotification::ConstPtr> FacebookNotificationsDatabase::notifications()
{
    QList<FacebookNotification::ConstPtr> data;

    QSqlQuery query;
    query = prepare(QStringLiteral(
                "SELECT fbId, accountId, fromStr, toStr, createdTime, updatedTime, title, link FROM notifications "));

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Failed to query events" << query.lastError().text();
        return data;
    }

    while (query.next()) {
        data.append(FacebookNotification::create(query.value(0).toString(),    // fbId
                                                 query.value(2).toString(),    // from
                                                 query.value(3).toString(),    // to
                                                 query.value(4).toString(),    // createdTime
                                                 query.value(5).toString(),    // updatedTime
                                                 query.value(6).toString(),    // title
                                                 query.value(7).toString(),    // link
                                                 query.value(1).toInt()));     // accountId
    }

    return data;
}

void FacebookNotificationsDatabase::readFinished()
{
    qDebug("ANTTI: FacebookNotificationsDatabase::readFinished");
    emit notificationsChanged();
}

bool FacebookNotificationsDatabase::write()
{
    Q_D(FacebookNotificationsDatabase);

    qDebug("ANTTI: FacebookNotificationsDatabase::write() (115)");

    QMutexLocker locker(&d->mutex);

    const QMap<int, QList<FacebookNotification::ConstPtr> > insertNotifications = d->queue.insertNotifications;
    const QMap<int, QList<FacebookNotification::ConstPtr> > removeNotifications = d->queue.removeNotifications;

    d->queue.insertNotifications.clear();
    d->queue.removeNotifications.clear();

    locker.unlock();

    bool success = true;
    QSqlQuery query;

    if (!removeNotifications.isEmpty()) {
/*        QVariantList accountIds;
        QVariantList gcalEventIds;

        Q_FOREACH (const QList<GoogleEvent::ConstPtr> &events, removeEvents) {
            Q_FOREACH (const GoogleEvent::ConstPtr &event, events) {
                accountIds.append(event->accountId());
                gcalEventIds.append(event->gcalEventId());
            }
        }

        query = prepare(QStringLiteral(
                    "DELETE FROM events "
                    "WHERE accountId = :accountId AND gcalEventId = :gcalEventId"));
        query.bindValue(QStringLiteral(":accountId"), accountIds);
        query.bindValue(QStringLiteral(":gcalEventId"), gcalEventIds);
        executeBatchSocialCacheQuery(query); */
    }

    if (!insertNotifications.isEmpty()) {
        QVariantList fbIds;
        QVariantList accountIds;
        QVariantList fromStrings;
        QVariantList toStrings;
        QVariantList createdTimes;
        QVariantList updatedTimes;
        QVariantList titles;
        QVariantList links;

        Q_FOREACH (const QList<FacebookNotification::ConstPtr> &notifications, insertNotifications) {
            Q_FOREACH (const FacebookNotification::ConstPtr &notification, notifications) {
                qDebug("***** ANTTI: notif when writing to db:");
                qDebug(" ANTTI: fbID: %s", qPrintable(notification->fbId()));
                qDebug(" ANTTI: from: %s", qPrintable(notification->from()));
                qDebug(" ANTTI: to: %s", qPrintable(notification->to()));
                qDebug(" ANTTI: createdTime: %s", qPrintable(notification->createdTime()));
                qDebug(" ANTTI: updatedTime: %s", qPrintable(notification->updatedTime()));
                qDebug(" ANTTI: title: %s", qPrintable(notification->title()));
                qDebug(" ANTTI: link: %s", qPrintable(notification->link()));

                fbIds.append(notification->fbId());
                accountIds.append(notification->accountId());
                fromStrings.append(notification->from());
                toStrings.append(notification->to());
                createdTimes.append(notification->createdTime());
                updatedTimes.append(notification->updatedTime());
                titles.append(notification->title());
                links.append(notification->link());
            }
        }

        query = prepare(QStringLiteral(
                    "INSERT OR REPLACE INTO notifications ("
                    " fbId, accountId, fromStr, toStr, createdTime, updatedTime, title, link) "
                    "VALUES("
                    " :fbId, :accountId, :fromStr, :toStr, :createdTime, :updatedTime, :title, :link)"));
        query.bindValue(QStringLiteral(":fbId"), fbIds);
        query.bindValue(QStringLiteral(":accountId"), accountIds);
        query.bindValue(QStringLiteral(":fromStr"), fromStrings);
        query.bindValue(QStringLiteral(":toStr"), toStrings);
        query.bindValue(QStringLiteral(":createdTime"), createdTimes);
        query.bindValue(QStringLiteral(":updatedTime"), updatedTimes);
        query.bindValue(QStringLiteral(":title"), titles);
        query.bindValue(QStringLiteral(":link"), links);
        executeBatchSocialCacheQuery(query);
    }

    return success;
}

bool FacebookNotificationsDatabase::createTables(QSqlDatabase database) const
{
    QSqlQuery query(database);
    qDebug("ANTTI: FacebookNotificationsDatabase::createTables (105)");

    // create the Facebook notification db tables
    // notifications = fbId, accountId, from, to, createdTime, updatedTime, title, link
    query.prepare("CREATE TABLE IF NOT EXISTS notifications ("
                  "fbId TEXT UNIQUE PRIMARY KEY,"
                  "accountId INTEGER,"
                  "fromStr TEXT,"
                  "toStr TEXT,"
                  "createdTime TEXT,"
                  "updatedTime TEXT,"
                  "title TEXT,"
                  "link TEXT)");
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Unable to create notifications table: " << query.lastError().text();
        return false;
    }

    return true;
}

bool FacebookNotificationsDatabase::dropTables(QSqlDatabase database) const
{
    QSqlQuery query(database);

    if (!query.exec(QStringLiteral("DROP TABLE IF EXISTS notifications"))) {
        qWarning() << Q_FUNC_INFO << "Unable to delete notifications table: " << query.lastError().text();
        return false;
    }

    return true;
}
