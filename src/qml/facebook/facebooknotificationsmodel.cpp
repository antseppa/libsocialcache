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

#include "facebooknotificationsmodel.h"
#include "abstractsocialcachemodel_p.h"
#include "facebooknotificationsdatabase.h"
#include <QtCore/QDebug>
//#include "postimagehelper_p.h"

class FacebookNotificationsModelPrivate : public AbstractSocialCacheModelPrivate
{
public:
    explicit FacebookNotificationsModelPrivate(FacebookNotificationsModel *q);

    FacebookNotificationsDatabase database;

private:
    Q_DECLARE_PUBLIC(FacebookNotificationsModel)
};

FacebookNotificationsModelPrivate::FacebookNotificationsModelPrivate(FacebookNotificationsModel *q)
    : AbstractSocialCacheModelPrivate(q)
{
}

FacebookNotificationsModel::FacebookNotificationsModel(QObject *parent)
    : AbstractSocialCacheModel(*(new FacebookNotificationsModelPrivate(this)), parent)
{
    Q_D(FacebookNotificationsModel);

    connect(&d->database, SIGNAL(notificationsChanged()), this, SLOT(notificationsChanged()));
}

QHash<int, QByteArray> FacebookNotificationsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(FacebookId, "facebookId");
    roleNames.insert(From, "from");
    roleNames.insert(To, "to");
    roleNames.insert(CreatedTime, "createdTime");
    roleNames.insert(UpdatedTime, "updatedTime");
    roleNames.insert(Title, "title");
    roleNames.insert(Link, "link");
    return roleNames;
}

void FacebookNotificationsModel::refresh()
{
    notificationsChanged();
}

void FacebookNotificationsModel::notificationsChanged()
{
    Q_D(FacebookNotificationsModel);

    qDebug("********** ANTTI: FacebookNotificationsModel::notificationsChanged");
    qDebug("DATABASE HAS: %d notifications", d->database.notifications().count());

    SocialCacheModelData data;
//    QMap<int, QVariant> eventMap;
/*    eventMap.insert(FacebookNotificationsModel::FacebookId, 1);
    eventMap.insert(FacebookNotificationsModel::From, "");
    eventMap.insert(FacebookNotificationsModel::To, "");
    eventMap.insert(FacebookNotificationsModel::CreatedTime, "2014-02-13T17:46:53+0000");
    eventMap.insert(FacebookNotificationsModel::UpdatedTime, "2014-02-15T14:27:24+0000");
    eventMap.insert(FacebookNotificationsModel::Title, "TESTING: Candy Crush Saga: Juha Rytkönen sent you a request.");
    eventMap.insert(FacebookNotificationsModel::Link, "http://apps.facebook.com/candycrush/?fb_source=notification&request_ids=267175756778812%2C210321122496548&ref=notif&app_request_type=user_to_user");
    data.append(eventMap); */

    QList<FacebookNotification::ConstPtr> notificationsData = d->database.notifications();
    Q_FOREACH (const FacebookNotification::ConstPtr &notification, notificationsData) {
        QMap<int, QVariant> eventMap;
        qDebug("-- ANTTI: notif when reading from db:");
        qDebug("!* ANTTI: fbID: %s", qPrintable(notification->fbId()));
        qDebug("!* ANTTI: from: %s", qPrintable(notification->from()));
        qDebug("!* ANTTI: to: %s", qPrintable(notification->to()));
        qDebug("!* ANTTI: createdTime: %s", qPrintable(notification->createdTime()));
        qDebug("!* ANTTI: updatedTime: %s", qPrintable(notification->updatedTime()));
        qDebug("!* ANTTI: title: %s", qPrintable(notification->title()));
        qDebug("!* ANTTI: link: %s", qPrintable(notification->link()));

        eventMap.insert(FacebookNotificationsModel::FacebookId, notification->fbId());
        eventMap.insert(FacebookNotificationsModel::From, notification->from());
        eventMap.insert(FacebookNotificationsModel::To, notification->to());
        eventMap.insert(FacebookNotificationsModel::CreatedTime, notification->createdTime());
        eventMap.insert(FacebookNotificationsModel::UpdatedTime, notification->updatedTime());
        eventMap.insert(FacebookNotificationsModel::Title, notification->title());
        eventMap.insert(FacebookNotificationsModel::Link, notification->link());
        data.append(eventMap);
    }

    updateData(data);
}
