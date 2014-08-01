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

#include "genericimagesdatabase.h"
#include "abstractsocialcachedatabase.h"
#include "socialsyncinterface.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QFile>

#include <QtDebug>

static const int VERSION = 1;

struct GenericImagePrivate
{
    explicit GenericImagePrivate(const QString &imageUrl, const QDateTime &createdTime,
                                 const QDateTime &updatedTime, const QString &imageName,
                                 int width, int height, const QString &thumbnailUrl,
                                 const QString &thumbnailFile,
                                 const QString &imageFile, int accountId = -1);
    QString imageUrl;
    QDateTime createdTime;
    QDateTime updatedTime;
    QString imageName;
    int width;
    int height;
    QString thumbnailUrl;
    QString thumbnailFile;
    QString imageFile;
    int accountId;
};

GenericImagePrivate::GenericImagePrivate(const QString &imageUrl, const QDateTime &createdTime,                                           const QDateTime &updatedTime, const QString &imageName,
                                           int width, int height, const QString &thumbnailUrl,
                                           const QString &thumbnailFile,
                                           const QString &imageFile, int accountId)
    : imageUrl(imageUrl)
    , createdTime(createdTime), updatedTime(updatedTime), imageName(imageName)
    , width(width), height(height), thumbnailUrl(thumbnailUrl)
    , thumbnailFile(thumbnailFile), imageFile(imageFile), accountId(accountId)
{
}

GenericImage::GenericImage(const QString &imageUrl, const QDateTime &createdTime,
                           const QDateTime &updatedTime, const QString &imageName,
                           int width, int height, const QString &thumbnailUrl,
                           const QString &thumbnailFile,
                           const QString &imageFile, int accountId)
    : d_ptr(new GenericImagePrivate(imageUrl, createdTime,
                                    updatedTime, imageName, width, height,
                                    thumbnailUrl, thumbnailFile,
                                    imageFile, accountId))
{
}

GenericImage::~GenericImage()
{
}

GenericImage::Ptr GenericImage::create(const QString &imageUrl, const QDateTime &createdTime,
                                         const QDateTime &updatedTime, const QString &imageName,
                                         int width, int height, const QString &thumbnailUrl,
                                         const QString &thumbnailFile,
                                         const QString &imageFile, int accountId)
{
    return GenericImage::Ptr(new GenericImage(imageUrl, createdTime,
                                                updatedTime, imageName, width, height,
                                                thumbnailUrl, thumbnailFile,
                                                imageFile, accountId));
}

QString GenericImage::imageUrl() const
{
    Q_D(const GenericImage);
    return d->imageUrl;
}

QDateTime GenericImage::createdTime() const
{
    Q_D(const GenericImage);
    return d->createdTime;
}

QDateTime GenericImage::updatedTime() const
{
    Q_D(const GenericImage);
    return d->updatedTime;
}

QString GenericImage::imageName() const
{
    Q_D(const GenericImage);
    return d->imageName;
}

int GenericImage::width() const
{
    Q_D(const GenericImage);
    return d->width;
}

int GenericImage::height() const
{
    Q_D(const GenericImage);
    return d->height;
}

QString GenericImage::thumbnailUrl() const
{
    Q_D(const GenericImage);
    return d->thumbnailUrl;
}

QString GenericImage::thumbnailFile() const
{
    Q_D(const GenericImage);
    return d->thumbnailFile;
}

QString GenericImage::imageFile() const
{
    Q_D(const GenericImage);
    return d->imageFile;
}

int GenericImage::accountId() const
{
    Q_D(const GenericImage);
    return d->accountId;
}

class GenericImagesDatabasePrivate: public AbstractSocialCacheDatabasePrivate
{
public:
    enum QueryType {
        Images,
    };

    explicit GenericImagesDatabasePrivate(const QLatin1String &databaseName, GenericImagesDatabase *q);
    ~GenericImagesDatabasePrivate();

private:
    Q_DECLARE_PUBLIC(GenericImagesDatabase)

    static void clearCachedImages(QSqlQuery &query);

    QList<int> queryAccounts();
    QList<GenericImage::ConstPtr> queryImages(int accountId);

    struct {
        QList<int> purgeAccounts;
        QStringList removeImages;
        QMap<QString, GenericImage::ConstPtr> insertImages;
        QMap<QString, QString> updateThumbnailFiles;
        QMap<QString, QString> updateImageFiles;
    } queue;

    struct {
        int accountId;
        QList<GenericImage::ConstPtr> images;
    } query;

    struct {
        QList<GenericImage::ConstPtr> images;
    } result;
};

GenericImagesDatabasePrivate::GenericImagesDatabasePrivate(const QLatin1String &databaseName, GenericImagesDatabase *q)
    : AbstractSocialCacheDatabasePrivate(
            q,
            SocialSyncInterface::socialNetwork(SocialSyncInterface::Generic),
            SocialSyncInterface::dataType(SocialSyncInterface::Images),
            QLatin1String(databaseName),
            VERSION)
{
}

GenericImagesDatabasePrivate::~GenericImagesDatabasePrivate()
{
}

void GenericImagesDatabasePrivate::clearCachedImages(QSqlQuery &query)
{
    while (query.next()) {
        QString thumb = query.value(0).toString();
        QString image = query.value(1).toString();

        if (!thumb.isEmpty()) {
            QFile thumbFile (thumb);
            if (thumbFile.exists()) {
                thumbFile.remove();
            }
        }

        if (!image.isEmpty()) {
            QFile imageFile (image);
            if (imageFile.exists()) {
                imageFile.remove();
            }
        }
    }
}

QList<int> GenericImagesDatabasePrivate::queryAccounts()
{
    return QList<int>();
}

QList<GenericImage::ConstPtr> GenericImagesDatabasePrivate::queryImages(int accountId)
{
    Q_Q(GenericImagesDatabase);

    QList<GenericImage::ConstPtr> data;

    QString queryString = QLatin1String("SELECT imageUrl, "\
                                        "createdTime, "\
                                        "updatedTime, imageName, width, "\
                                        "height, thumbnailUrl, "\
                                        "thumbnailFile, imageFile, "\
                                        "accountId "\
                                        "FROM images "\
                                        "WHERE accountId = :accountId ORDER BY updatedTime");

    QSqlQuery query = q->prepare(queryString);
    query.bindValue(QStringLiteral(":accountId"), accountId);

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Failed to query images:" << query.lastError().text();
        return data;
    }

    while (query.next()) {
        data.append(GenericImage::create(query.value(0).toString(),                       // imageUrl
                                         QDateTime::fromTime_t(query.value(1).toUInt()),  // createdTime
                                         QDateTime::fromTime_t(query.value(2).toUInt()),  // updatedTime
                                         query.value(3).toString(),                       // imageName
                                         query.value(4).toInt(),                          // width
                                         query.value(5).toInt(),                          // height
                                         query.value(6).toString(),                       // thumbnailUrl
                                         query.value(7).toString(),                       // thumbnailFile
                                         query.value(8).toString(),                       // imageFile
                                         query.value(9).toInt()));                        // accountId
    }

    return data;
}

bool operator==(const GenericImage::ConstPtr &image1, const GenericImage::ConstPtr &image2)
{
    return image1->imageUrl() == image2->imageUrl() && image1->accountId() == image2->accountId();
}

// Note
//
// Insertion operations needs to use write(), while Delete
// operations are automatically using transactions and
// don't need write().

GenericImagesDatabase::GenericImagesDatabase(const QLatin1String &databaseName)
    : AbstractSocialCacheDatabase(*(new GenericImagesDatabasePrivate(databaseName, this)))
{
}

GenericImagesDatabase::~GenericImagesDatabase()
{
    wait();
}

void GenericImagesDatabase::purgeAccount(int accountId)
{
    Q_D(GenericImagesDatabase);
    QMutexLocker locker(&d->mutex);

    d->queue.purgeAccounts.append(accountId);
}

QStringList GenericImagesDatabase::imageUrls(int accountId, bool *ok) const
{
    if (ok) {
        *ok = false;
    }

    QStringList urls;
    QSqlQuery query = prepare(QStringLiteral(
                "SELECT DISTINCT imageUrl, updatedTime "
                "FROM images WHERE accountId = :accountId "
                "ORDER BY updatedTime DESC"));
    query.bindValue(":accountId", accountId);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Unable to fetch image urls" << query.lastError().text();
        return urls;
    }

    while (query.next()) {
        urls.append(query.value(0).toString());
    }

    if (ok) {
        *ok = true;
    }

    return urls;
}

GenericImage::ConstPtr GenericImagesDatabase::image(int accountId, const QString &imageUrl) const
{
    QSqlQuery query = prepare(
                "SELECT imageUrl, updatedTime, imageName, "
                "width, height, thumbnailUrl, thumbnailFile, imageFile, accountId "
                "FROM images WHERE imageUrl = :imageUrl AND accountId = :accountId");
    query.bindValue(":imageUrl", imageUrl);
    query.bindValue(":accountId", accountId);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Error reading from image table:" << query.lastError();
        return GenericImage::Ptr();
    }

    // If we have the image, we have a result, otherwise we won't have the image
    if (!query.next()) {
        return GenericImage::Ptr();
    }

    return GenericImage::create(query.value(0).toString(),                       // imageUrl
                                QDateTime::fromTime_t(query.value(1).toUInt()),  // createdTime
                                QDateTime::fromTime_t(query.value(2).toUInt()),  // updatedTime
                                query.value(3).toString(),                       // imageName
                                query.value(4).toInt(),                          // width
                                query.value(5).toInt(),                          // height
                                query.value(6).toString(),                       // thumbnailUrl
                                query.value(7).toString(),                       // thumbnailFile
                                query.value(8).toString(),                       // imageFile
                                query.value(9).toInt());                         // accountId
}

void GenericImagesDatabase::removeImage(const QString &imageUrl)
{
    Q_D(GenericImagesDatabase);
    QMutexLocker locker(&d->mutex);

    d->queue.removeImages.append(imageUrl);
}

void GenericImagesDatabase::removeImages(const QStringList &imageUrls)
{
    Q_D(GenericImagesDatabase);
    QMutexLocker locker(&d->mutex);

    d->queue.removeImages += imageUrls;
}

void GenericImagesDatabase::addImage(const QString &imageUrl, const QDateTime &createdTime,
                                     const QDateTime &updatedTime, const QString &imageName,
                                     int width, int height, const QString &thumbnailUrl,
                                     int accountId)
{
    Q_D(GenericImagesDatabase);
    GenericImage::Ptr image = GenericImage::create(imageUrl, createdTime, updatedTime,
                                                   imageName, width, height,
                                                   thumbnailUrl, imageUrl, QString(),
                                                   accountId);
    QMutexLocker locker(&d->mutex);

    d->queue.insertImages.insert(imageUrl, image);
}

void GenericImagesDatabase::queryImages(int accountId)
{
    Q_D(GenericImagesDatabase);
    {
        QMutexLocker locker(&d->mutex);
        d->query.accountId = accountId;
    }
    executeRead();
}

void GenericImagesDatabase::updateImageThumbnail(const QString &imageUrl,
                                                 const QString &thumbnailFile)
{
    Q_D(GenericImagesDatabase);

    QMutexLocker locker(&d->mutex);
    d->queue.updateThumbnailFiles.insert(imageUrl, thumbnailFile);
}

void GenericImagesDatabase::updateImageFile(const QString &imageUrl, const QString &imageFile)
{
    Q_D(GenericImagesDatabase);

    QMutexLocker locker(&d->mutex);
    d->queue.updateImageFiles.insert(imageUrl, imageFile);
}

void GenericImagesDatabase::commit()
{
    executeWrite();
}

QList<GenericImage::ConstPtr> GenericImagesDatabase::images() const
{
    return d_func()->result.images;
}

bool GenericImagesDatabase::read()
{
    Q_D(GenericImagesDatabase);
    QMutexLocker locker(&d->mutex);

    locker.unlock();
    QList<GenericImage::ConstPtr> images = d->queryImages(d->query.accountId);
    locker.relock();
    d->query.images = images;
    return true;
}

void GenericImagesDatabase::readFinished()
{
    Q_D(GenericImagesDatabase);
    {
        QMutexLocker locker(&d->mutex);
        d->result.images = d->query.images;
        d->query.images.clear();
    }
    emit queryFinished();
}

bool GenericImagesDatabase::write()
{
    Q_D(GenericImagesDatabase);
    QMutexLocker locker(&d->mutex);

    qWarning() << "Queued images being saved:" << d->queue.insertImages.count();
    qWarning() << "Queued thumbnail files being updated:" << d->queue.updateThumbnailFiles.count();
    qWarning() << "Queued image files being updated:" << d->queue.updateImageFiles.count();

    const QList<int> purgeAccounts = d->queue.purgeAccounts;
    const QStringList removeImages = d->queue.removeImages;
    const QMap<QString, GenericImage::ConstPtr> insertImages = d->queue.insertImages;
    const QMap<QString, QString> updateThumbnailFiles = d->queue.updateThumbnailFiles;
    const QMap<QString, QString> updateImageFiles = d->queue.updateImageFiles;

    d->queue.purgeAccounts.clear();
    d->queue.removeImages.clear();
    d->queue.insertImages.clear();
    d->queue.updateThumbnailFiles.clear();
    d->queue.updateImageFiles.clear();

    locker.unlock();

    bool success = true;
    QSqlQuery query;

    if (!purgeAccounts.isEmpty()) {
        QVariantList accountIds;
        Q_FOREACH (const int accountId, purgeAccounts) {
            accountIds.append(accountId);
        }

        query = prepare(QStringLiteral(
                        "DELETE FROM images "
                        "WHERE accountId = :accountId"));
        query.bindValue(QStringLiteral(":accountId"), accountIds);
        executeBatchSocialCacheQuery(query);
    }

    if (!removeImages.isEmpty()) {
        QVariantList imageUrls;

        query = prepare(QStringLiteral(
                    "SELECT thumbnailFile, imageFile "
                    "FROM images "
                    "WHERE imageUrl = :imageUrl"));
        Q_FOREACH (const QString &imageUrl, removeImages) {
            imageUrls.append(imageUrl);

            query.bindValue(QStringLiteral(":fbImageUrl"), imageUrl);
            if (!query.exec()) {
                qWarning() << Q_FUNC_INFO << "Failed to exec cached images selection query:"
                           << query.lastError().text();
            } else {
                d->clearCachedImages(query);
            }
        }

        query = prepare(QStringLiteral(
                    "DELETE FROM images "
                    "WHERE imageUrl = :imageUrl"));
        query.bindValue(QStringLiteral(":imageUrl"), imageUrls);
        executeBatchSocialCacheQuery(query);
    }

    if (!insertImages.isEmpty()) {
        QVariantList imageUrls, createdTimes, updatedTimes;
        QVariantList imageNames, widths, heights;
        QVariantList accountIds, thumbnailUrls;
        QVariantList thumbnailFiles, imageFiles;

        Q_FOREACH (const GenericImage::ConstPtr &image, insertImages) {
            imageUrls.append(image->imageUrl());
            createdTimes.append(image->createdTime().toTime_t());
            updatedTimes.append(image->updatedTime().toTime_t());
            imageNames.append(image->imageName());
            widths.append(image->width());
            heights.append(image->height());
            accountIds.append(image->accountId());
            thumbnailUrls.append(image->thumbnailUrl());
            thumbnailFiles.append(image->thumbnailFile());
            imageFiles.append(image->imageFile());
        }

        query = prepare(QStringLiteral(
                    "INSERT OR REPLACE INTO images ("
                    " imageUrl, createdTime, updatedTime, "
                    " width, height, thumbnailUrl, thumbnailFile, imageFile, accountId) "
                    "VALUES ("
                    " :imageUrl, :createdTime, :updatedTime, "
                    " :width, :height, :thumbnailUrl, :thumbnailFile, :imageFile, :accountId)"));
        query.bindValue(QStringLiteral(":imageUrl"), imageUrls);
        query.bindValue(QStringLiteral(":createdTime"), createdTimes);
        query.bindValue(QStringLiteral(":updatedTime"), updatedTimes);
        query.bindValue(QStringLiteral(":width"), widths);
        query.bindValue(QStringLiteral(":height"), heights);
        query.bindValue(QStringLiteral(":thumbnailUrl"), thumbnailUrls);
        query.bindValue(QStringLiteral(":thumbnailFile"), thumbnailFiles);
        query.bindValue(QStringLiteral(":imageFile"), imageFiles);
        query.bindValue(QStringLiteral(":accountId"), accountIds);
        executeBatchSocialCacheQuery(query);
    }

    if (!updateThumbnailFiles.isEmpty()) {
        QVariantList imageUrls;
        QVariantList thumbnailFiles;

        for (QMap<QString, QString>::const_iterator it = updateThumbnailFiles.begin();
                it != updateThumbnailFiles.end();
                ++it) {
            imageUrls.append(it.key());
            thumbnailFiles.append(it.value());
        }

        query = prepare(QStringLiteral(
                    "UPDATE images "
                    "SET thumbnailFile = :thumbnailFile "
                    "WHERE imageUrl = :imageUrl"));
        query.bindValue(QStringLiteral(":thumbnailFile"), thumbnailFiles);
        query.bindValue(QStringLiteral(":imageUrl"), imageUrls);
        executeBatchSocialCacheQuery(query);
    }

    if (!updateImageFiles.isEmpty()) {
        QVariantList imageUrls;
        QVariantList imageFiles;

        for (QMap<QString, QString>::const_iterator it = updateImageFiles.begin();
                it != updateImageFiles.end();
                ++it) {
            imageUrls.append(it.key());
            imageFiles.append(it.value());
        }

        query = prepare(QStringLiteral(
                    "UPDATE images "
                    "SET imageFile = :imageFile "
                    "WHERE imageUrl = :iamgeUrl"));
        query.bindValue(QStringLiteral(":imageFile"), imageFiles);
        query.bindValue(QStringLiteral(":imageUrl"), imageUrls);
        executeBatchSocialCacheQuery(query);
    }

    return success;
}

bool GenericImagesDatabase::createTables(QSqlDatabase database) const
{
    // create the younited image db tables
    // images = imageUrl, createdTime, updatedTime, width, height,
    //          accountId, thumbnailUrl, thumbnailFile, imageFile
    QSqlQuery query(database);
    query.prepare( "CREATE TABLE IF NOT EXISTS images ("
                   "imageUrl TEXT UNIQUE PRIMARY KEY,"
                   "createdTime INTEGER,"
                   "updatedTime INTEGER,"
                   "imageName TEXT,"
                   "width INTEGER,"
                   "height INTEGER,"
                   "accountId INTEGER,"
                   "thumbnailUrl TEXT,"
                   "thumbnailFile TEXT,"
                   "imageFile TEXT)");
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Unable to create images table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GenericImagesDatabase::dropTables(QSqlDatabase database) const
{
    QSqlQuery query(database);
    query.prepare("DROP TABLE IF EXISTS images");
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Failed to delete images table:" << query.lastError().text();
        return false;
    }

    return true;
}
