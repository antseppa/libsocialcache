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

#ifndef GENERICIMAGECACHEMODEL_H
#define GENERICIMAGECACHEMODEL_H

#include "abstractsocialcachemodel.h"
#include "genericimagedownloader.h"

class GenericImageCacheModelPrivate;
class GenericImageCacheModel: public AbstractSocialCacheModel
{
    Q_OBJECT
    Q_PROPERTY(GenericImageDownloader * downloader READ downloader WRITE setDownloader
               NOTIFY downloaderChanged)
    Q_PROPERTY(int accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)

    Q_ENUMS(GenericImageRole)

public:
    enum GenericImageRole {
        ImageUrl = 0,
        CreatedTime,
        UpdatedTime,
        ImageName,
        Width,
        Height,
        ThumbnailUrl,
        ImageFile,
        AccountId
    };

    explicit GenericImageCacheModel(QObject *parent = 0);
    ~GenericImageCacheModel();

    QHash<int, QByteArray> roleNames() const;

    // properties
    GenericImageDownloader *downloader() const;
    void setDownloader(GenericImageDownloader *downloader);
    int accountId() const;
    void setAccountId(int accountId);

    // from AbstractListModel
    QVariant data(const QModelIndex &index, int role) const;

public Q_SLOTS:
    void loadImages();
    void refresh();

Q_SIGNALS:
    void typeChanged();
    void downloaderChanged();
    void accountIdChanged();

private Q_SLOTS:
    void queryFinished();
    void imageDownloaded(const QString &url, const QString &path, const QVariantMap &imageData);

private:
    Q_DECLARE_PRIVATE(GenericImageCacheModel)
    friend class GenericImageDownloader;
};

#endif // GENERICIMAGECACHEMODEL_H
