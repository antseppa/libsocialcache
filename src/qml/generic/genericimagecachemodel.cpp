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

#include "genericimagecachemodel.h"
#include "abstractsocialcachemodel_p.h"
#include "genericimagesdatabase.h"

#include "genericimagedownloader_p.h"
#include "genericimagedownloaderconstants_p.h"

#include <QtCore/QThread>
#include <QtCore/QStandardPaths>

#include <QtDebug>

// Note:
//
// When querying photos, the nodeIdentifier should be either
// - nothing: query all photos
// - user-USER_ID: query all photos for the given user
// - album-ALBUM_ID: query all photos for the given album

static const char *URL_KEY = "url";
static const char *ROW_KEY = "row";
static const char *MODEL_KEY = "model";

#define SOCIALCACHE_GENERIC_IMAGE_DIR   PRIVILEGED_DATA_DIR + QLatin1String("/Images/")

class GenericImageCacheModelPrivate : public AbstractSocialCacheModelPrivate
{
public:
    GenericImageCacheModelPrivate(GenericImageCacheModel *q);
    ~GenericImageCacheModelPrivate();


    void queue(
            int row,
            GenericImageDownloader::ImageType imageType,
            const QString &identifier,
            const QString &url);

    GenericImageDownloader *downloader;
    GenericImagesDatabase *database;
    int accountId;
};

GenericImageCacheModelPrivate::GenericImageCacheModelPrivate(GenericImageCacheModel *q)
    : AbstractSocialCacheModelPrivate(q), downloader(0), accountId(-1)
{
    database = new GenericImagesDatabase(QLatin1String("FOO")); // FIX
}

GenericImageCacheModelPrivate::~GenericImageCacheModelPrivate()
{
    delete database;  // GET RID OF THIS...
}

void GenericImageCacheModelPrivate::queue(
        int row,
        GenericImageDownloader::ImageType imageType,
        const QString &identifier,
        const QString &url)
{
    GenericImageCacheModel *modelPtr = qobject_cast<GenericImageCacheModel*>(q_ptr);
    if (downloader) {
        QVariantMap metadata;
        metadata.insert(QLatin1String(TYPE_KEY), imageType);
        metadata.insert(QLatin1String(IDENTIFIER_KEY), identifier);
        metadata.insert(QLatin1String(URL_KEY), url);
        metadata.insert(QLatin1String(ROW_KEY), row);
        metadata.insert(QLatin1String(MODEL_KEY), QVariant::fromValue<void*>((void*)modelPtr));

        downloader->queue(url, metadata);
    }
}

GenericImageCacheModel::GenericImageCacheModel(QObject *parent)
    : AbstractSocialCacheModel(*(new GenericImageCacheModelPrivate(this)), parent)
{
    Q_D(const GenericImageCacheModel);
    connect(d->database, &GenericImagesDatabase::queryFinished,
            this, &GenericImageCacheModel::queryFinished);
}

GenericImageCacheModel::~GenericImageCacheModel()
{
    Q_D(GenericImageCacheModel);
    if (d->downloader) {
        d->downloader->removeModelFromHash(this);
    }
}

QHash<int, QByteArray> GenericImageCacheModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(ImageUrl, "imageUrl");
    roleNames.insert(CreatedTime, "createdTime");
    roleNames.insert(UpdatedTime, "updatedTime");
    roleNames.insert(ImageName, "imageName");
    roleNames.insert(Width, "width");
    roleNames.insert(Height, "height");
    roleNames.insert(ThumbnailUrl, "thumbnailUrl");
    roleNames.insert(ImageFile, "imageFile");
    roleNames.insert(AccountId, "accountId");
    return roleNames;
}

GenericImageDownloader * GenericImageCacheModel::downloader() const
{
    Q_D(const GenericImageCacheModel);
    return d->downloader;
}

void GenericImageCacheModel::setDownloader(GenericImageDownloader *downloader)
{
    Q_D(GenericImageCacheModel);
    if (d->downloader != downloader) {
        if (d->downloader) {
            // Disconnect worker object
            disconnect(d->downloader);
            d->downloader->removeModelFromHash(this);
        }

        d->downloader = downloader;
        d->downloader->addModelToHash(this);
        emit downloaderChanged();
    }
}

int GenericImageCacheModel::accountId() const
{
    Q_D(const GenericImageCacheModel);
    return d->accountId;
}

void GenericImageCacheModel::setAccountId(int accountId)
{
    Q_D(GenericImageCacheModel);

    if (d->accountId != accountId) {
        d->accountId = accountId;
        emit accountIdChanged();
    }
}

QVariant GenericImageCacheModel::data(const QModelIndex &index, int role) const
{
    Q_D(const GenericImageCacheModel);
    int row = index.row();
    if (row < 0 || row >= d->m_data.count()) {
        return QVariant();
    }

    if (d->m_data.at(row).value(role).toString().isEmpty()) {
        // haven't downloaded the image yet.  Download it.
        if (d->database->images().size() > row) {
            GenericImage::ConstPtr imageData = d->database->images().at(row);
            GenericImageCacheModelPrivate *nonconstD = const_cast<GenericImageCacheModelPrivate*>(d);
            nonconstD->queue(row, GenericImageDownloader::FullImage,
                             QString::number(imageData->accountId()),
                             imageData->imageUrl());
        }
    }

    return d->m_data.at(row).value(role);
}

void GenericImageCacheModel::loadImages()
{
    refresh();
}

void GenericImageCacheModel::refresh()
{
     Q_D(GenericImageCacheModel);
     d->database->queryImages(d->accountId);
}

// NOTE: this is now called directly by GenericImageDownloader
// rather than connected to the imageDownloaded signal, for
// performance reasons.
void GenericImageCacheModel::imageDownloaded(
        const QString &, const QString &path, const QVariantMap &imageData)
{
    Q_UNUSED(path);
    Q_UNUSED(imageData);

    Q_D(GenericImageCacheModel);

    int row = imageData.value(ROW_KEY).toInt();
    if (row < 0 || row >= d->m_data.count()) {
        qWarning() << Q_FUNC_INFO
                   << "Invalid row:" << row
                   << "max row:" << d->m_data.count();
        return;
    }

    int type = imageData.value(TYPE_KEY).toInt();
    switch (type) {
    case GenericImageDownloader::ThumbnailImage:
        d->m_data[row].insert(GenericImageCacheModel::ThumbnailUrl, path);
        break;
    case GenericImageDownloader::FullImage:
        d->m_data[row].insert(GenericImageCacheModel::ImageUrl, path);
        break;
    }

    emit dataChanged(index(row), index(row));
}

void GenericImageCacheModel::queryFinished()
{
    Q_D(GenericImageCacheModel);

    QList<QVariantMap> thumbQueue;
    SocialCacheModelData data;

    QList<GenericImage::ConstPtr> imagesData = d->database->images();

    for (int i = 0; i < imagesData.count(); i++) {
        const GenericImage::ConstPtr & imageData = imagesData.at(i);
        QMap<int, QVariant> imageMap;
        imageMap.insert(GenericImageCacheModel::ImageUrl, imageData->imageUrl());
        if (imageData->thumbnailFile().isEmpty()) {
            QVariantMap thumbQueueData;
            thumbQueueData.insert("row", QVariant::fromValue<int>(i));
            thumbQueueData.insert("imageType", QVariant::fromValue<int>(GenericImageDownloader::ThumbnailImage));
            thumbQueueData.insert("identifier", imageData->imageUrl());
            thumbQueueData.insert("url", imageData->thumbnailUrl());
            thumbQueue.append(thumbQueueData);
        }
        // note: we don't queue the image file until the user explicitly opens that in fullscreen.
        imageMap.insert(GenericImageCacheModel::ThumbnailUrl, imageData->thumbnailUrl());
        imageMap.insert(GenericImageCacheModel::ImageFile, imageData->imageFile());
        imageMap.insert(GenericImageCacheModel::CreatedTime, imageData->createdTime());
        imageMap.insert(GenericImageCacheModel::Width, imageData->width());
        imageMap.insert(GenericImageCacheModel::Height, imageData->height());
        imageMap.insert(GenericImageCacheModel::AccountId, imageData->accountId());
        data.append(imageMap);
    }

    updateData(data);

    // now download the queued thumbnails.
    foreach (const QVariantMap &thumbQueueData, thumbQueue) {
        d->queue(thumbQueueData["row"].toInt(),
                 static_cast<GenericImageDownloader::ImageType>(thumbQueueData["imageType"].toInt()),
                 thumbQueueData["identifier"].toString(),
                 thumbQueueData["url"].toString());
    }
}
