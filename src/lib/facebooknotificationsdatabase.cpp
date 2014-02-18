/*
 * Copyright (C) 2014 Jolla Ltd.
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
    explicit FacebookNotificationPrivate(const QString &facebookId, const QString &from, const QString &to,
                                         const QString &createdTime, const QString &updatedTime,
                                         const QString &title, const QString &link,
                                         const QString &application, const QString &object,
                                         int accountId, const QString &clientId);

    QString m_facebookId;
    QString m_from;
    QString m_to;
    QString m_createdTime;
    QString m_updatedTime;
    QString m_title;
    QString m_link;
    QString m_application;
    QString m_object;
    int m_accountId;
    QString m_clientId;
};

FacebookNotificationPrivate::FacebookNotificationPrivate(const QString &facebookId, const QString &from, const QString &to,
                                                         const QString &createdTime, const QString &updatedTime,
                                                         const QString &title, const QString &link,
                                                         const QString &application, const QString &object,
                                                         int accountId, const QString &clientId)
    : m_facebookId(facebookId)
    , m_from(from)
    , m_to(to)
    , m_createdTime(createdTime)
    , m_updatedTime(updatedTime)
    , m_title(title)
    , m_link(link)
    , m_application(application)
    , m_object(object)
    , m_accountId(accountId)
    , m_clientId(clientId)
{
}

FacebookNotification::FacebookNotification(const QString &facebookId, const QString &from, const QString &to,
                                           const QString &createdTime, const QString &updatedTime,
                                           const QString &title, const QString &link,
                                           const QString &application, const QString &object,
                                           int accountId, const QString &clientId)
    : d_ptr(new FacebookNotificationPrivate(facebookId, from, to, createdTime, updatedTime, title, link,
                                            application, object, accountId, clientId))
{
}

FacebookNotification::Ptr FacebookNotification::create(const QString &facebookId, const QString &from, const QString &to,
                                                       const QString &createdTime, const QString &updatedTime,
                                                       const QString &title, const QString &link,
                                                       const QString &application, const QString &object,
                                                       int accountId, const QString &clientId)
{
    return FacebookNotification::Ptr(new FacebookNotification(facebookId, from, to, createdTime, updatedTime, title, link,
                                                              application, object, accountId, clientId));
}

FacebookNotification::~FacebookNotification()
{
}

QString FacebookNotification::facebookId() const
{
    Q_D(const FacebookNotification);
    return d->m_facebookId;
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

QString FacebookNotification::application() const
{
    Q_D(const FacebookNotification);
    return d->m_application;
}

QString FacebookNotification::object() const
{
    Q_D(const FacebookNotification);
    return d->m_object;
}

int FacebookNotification::accountId() const
{
    Q_D(const FacebookNotification);
    return d->m_accountId;
}

QString FacebookNotification::clientId() const
{
    Q_D(const FacebookNotification);
    return d->m_clientId;
}

class FacebookNotificationsDatabasePrivate: public AbstractSocialCacheDatabasePrivate
{
public:
    explicit FacebookNotificationsDatabasePrivate(FacebookNotificationsDatabase *q);

    QMap<int, QList<FacebookNotification::ConstPtr> > insertNotifications;
    QList<int> removeNotificationsFromAccounts;

    struct {
        QMap<int, QList<FacebookNotification::ConstPtr> > insertNotifications;
        QList<int> removeNotificationsFromAccounts;
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

void FacebookNotificationsDatabase::addFacebookNotification(const QString &facebookId, const QString &from, const QString &to,
                                                            const QString &createdTime, const QString &updatedTime,
                                                            const QString &title, const QString &link,
                                                            const QString &application, const QString &object,
                                                            int accountId, const QString &clientId)
{
    Q_D(FacebookNotificationsDatabase);
    d->insertNotifications[accountId].append(FacebookNotification::create(facebookId, from, to, createdTime, updatedTime, title, link,
                                                                          application, object, accountId, clientId));
}

void FacebookNotificationsDatabase::removeNotifications(int accountId)
{
    Q_D(FacebookNotificationsDatabase);

    QMutexLocker locker(&d->mutex);
    if (!d->queue.removeNotificationsFromAccounts.contains(accountId)) {
        d->queue.removeNotificationsFromAccounts.append(accountId);
    }
}

void FacebookNotificationsDatabase::sync()
{
    Q_D(FacebookNotificationsDatabase);

    {
        QMutexLocker locker(&d->mutex);
        Q_FOREACH(int accountId, d->insertNotifications.keys()) {
            d->queue.insertNotifications.insert(accountId, d->insertNotifications.take(accountId));
        }
        while (d->removeNotificationsFromAccounts.count()) {
            d->queue.removeNotificationsFromAccounts.append(d->removeNotificationsFromAccounts.takeFirst());
        }
    }

    executeWrite();
}

QList<FacebookNotification::ConstPtr> FacebookNotificationsDatabase::notifications()
{
    QList<FacebookNotification::ConstPtr> data;

    QSqlQuery query;
    query = prepare(QStringLiteral(
                "SELECT facebookId, accountId, fromStr, toStr, createdTime, updatedTime, title, link, application," \
                "objectStr, clientId FROM notifications ORDER BY updatedTime DESC"));

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Failed to query events" << query.lastError().text();
        return data;
    }

    while (query.next()) {
        data.append(FacebookNotification::create(query.value(0).toString(),     // facebookId
                                                 query.value(2).toString(),     // from
                                                 query.value(3).toString(),     // to
                                                 query.value(4).toString(),     // createdTime
                                                 query.value(5).toString(),     // updatedTime
                                                 query.value(6).toString(),     // title
                                                 query.value(7).toString(),     // link
                                                 query.value(8).toString(),     // application
                                                 query.value(9).toString(),     // object
                                                 query.value(1).toInt(),        // accountId
                                                 query.value(10).toString()));  // clientId
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
    const QList<int> removeNotificationsFromAccounts = d->queue.removeNotificationsFromAccounts;

    d->queue.insertNotifications.clear();
    d->queue.removeNotificationsFromAccounts.clear();

    locker.unlock();

    bool success = true;
    QSqlQuery query;

    if (!removeNotificationsFromAccounts.isEmpty()) {
        QVariantList accountIds;

        Q_FOREACH (const int accountId, removeNotificationsFromAccounts) {
            qDebug("ANTTI: REMOVING NOTIFICATIONS FOR ACCOUNT %d", accountId);
            accountIds.append(accountId);
        }

        query = prepare(QStringLiteral("DELETE FROM notifications WHERE accountId = :accountId"));
        query.bindValue(QStringLiteral(":accountId"), accountIds);
        executeBatchSocialCacheQuery(query);
    }

    if (!insertNotifications.isEmpty()) {
        QVariantList facebookIds;
        QVariantList accountIds;
        QVariantList fromStrings;
        QVariantList toStrings;
        QVariantList createdTimes;
        QVariantList updatedTimes;
        QVariantList titles;
        QVariantList links;
        QVariantList applications;
        QVariantList objects;
        QVariantList clientIds;

        Q_FOREACH (const QList<FacebookNotification::ConstPtr> &notifications, insertNotifications) {
            Q_FOREACH (const FacebookNotification::ConstPtr &notification, notifications) {
                qDebug("***** ANTTI: notif when writing to db:");
                qDebug(" ANTTI: facebookId: %s", qPrintable(notification->facebookId()));
                qDebug(" ANTTI: from: %s", qPrintable(notification->from()));
                qDebug(" ANTTI: to: %s", qPrintable(notification->to()));
                qDebug(" ANTTI: createdTime: %s", qPrintable(notification->createdTime()));
                qDebug(" ANTTI: updatedTime: %s", qPrintable(notification->updatedTime()));
                qDebug(" ANTTI: title: %s", qPrintable(notification->title()));
                qDebug(" ANTTI: link: %s", qPrintable(notification->link()));
                qDebug(" ANTTI: application: %s", qPrintable(notification->application()));
                qDebug(" ANTTI: object: %s", qPrintable(notification->object()));
                qDebug(" ANTTI: clientId: %s", qPrintable(notification->clientId()));

                facebookIds.append(notification->facebookId());
                accountIds.append(notification->accountId());
                fromStrings.append(notification->from());
                toStrings.append(notification->to());
                createdTimes.append(notification->createdTime());
                updatedTimes.append(notification->updatedTime());
                titles.append(notification->title());
                links.append(notification->link());
                applications.append(notification->application());
                objects.append(notification->object());
                clientIds.append(notification->clientId());
            }
        }

        query = prepare(QStringLiteral(
                    "INSERT OR REPLACE INTO notifications ("
                    " facebookId, accountId, fromStr, toStr, createdTime, updatedTime, title, link, application, objectStr, clientId) "
                    "VALUES("
                    " :facebookId, :accountId, :fromStr, :toStr, :createdTime, :updatedTime, :title, :link, :application, :objectStr, :clientId)"));
        query.bindValue(QStringLiteral(":facebookId"), facebookIds);
        query.bindValue(QStringLiteral(":accountId"), accountIds);
        query.bindValue(QStringLiteral(":fromStr"), fromStrings);
        query.bindValue(QStringLiteral(":toStr"), toStrings);
        query.bindValue(QStringLiteral(":createdTime"), createdTimes);
        query.bindValue(QStringLiteral(":updatedTime"), updatedTimes);
        query.bindValue(QStringLiteral(":title"), titles);
        query.bindValue(QStringLiteral(":link"), links);
        query.bindValue(QStringLiteral(":application"), applications);
        query.bindValue(QStringLiteral(":objectStr"), objects);
        query.bindValue(QStringLiteral(":clientId"), clientIds);

        executeBatchSocialCacheQuery(query);
    }

    return success;
}

bool FacebookNotificationsDatabase::createTables(QSqlDatabase database) const
{
    QSqlQuery query(database);
    qDebug("ANTTI: FacebookNotificationsDatabase::createTables (106)");

    // create the Facebook notification db tables
    // notifications = facebookId, accountId, from, to, createdTime, updatedTime, title, link, application, objectStr, clientId
    query.prepare("CREATE TABLE IF NOT EXISTS notifications ("
                  "facebookId TEXT UNIQUE PRIMARY KEY,"
                  "accountId INTEGER,"
                  "fromStr TEXT,"
                  "toStr TEXT,"
                  "createdTime TEXT,"
                  "updatedTime TEXT,"
                  "title TEXT,"
                  "link TEXT,"
                  "application TEXT,"
                  "objectStr TEXT,"
                  "clientId TEXT)");
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
