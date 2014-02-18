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

#ifndef FACEBOOKNOTIFICATIONSDATABASE_H
#define FACEBOOKNOTIFICATIONSDATABASE_H

#include "abstractsocialcachedatabase.h"

#include <QtCore/QSharedPointer>

class FacebookNotificationPrivate;
class FacebookNotification
{
public:
    typedef QSharedPointer<FacebookNotification> Ptr;
    typedef QSharedPointer<const FacebookNotification> ConstPtr;
    virtual ~FacebookNotification();
    static FacebookNotification::Ptr create(const QString &fbId, const QString &from, const QString &to,
                                            const QString &createdTime, const QString &updatedTime,
                                            const QString &title, const QString &link,
                                            int accountId);
    QString fbId() const;
    QString from() const;
    QString to() const;
    QString createdTime() const;
    QString updatedTime() const;
    QString title() const;
    QString link() const;
    int accountId() const;
protected:
    QScopedPointer<FacebookNotificationPrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(FacebookNotification)
    explicit FacebookNotification(const QString &fbId, const QString &from, const QString &to,
                                  const QString &createdTime, const QString &updatedTime,
                                  const QString &title, const QString &link,
                                  int accountId);
};


class FacebookNotificationsDatabasePrivate;
class FacebookNotificationsDatabase: public AbstractSocialCacheDatabase
{
    Q_OBJECT

public:
    explicit FacebookNotificationsDatabase();
    ~FacebookNotificationsDatabase();

    void addFacebookNotification(const QString &id, const QString &from, const QString &to,
                                 const QString &createdTime, const QString &updatedTime,
                                 const QString &title, const QString &link, int accountId);
    void removeNotifcations(int accountId);

    void sync();

    QList<FacebookNotification::ConstPtr> notifications();

signals:
    void notificationsChanged();

protected:
    void readFinished();
    bool write();
    bool createTables(QSqlDatabase database) const;
    bool dropTables(QSqlDatabase database) const;

private:
    Q_DECLARE_PRIVATE(FacebookNotificationsDatabase)
};

#endif // FACEBOOKNOTIFICATIONSDATABASE_H
