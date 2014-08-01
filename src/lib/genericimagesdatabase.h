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

#ifndef GENERICIMAGESDATABASE_H
#define GENERICIMAGESDATABASE_H

#include "abstractsocialcachedatabase_p.h"
#include <QtCore/QDateTime>
#include <QtCore/QStringList>

class GenericImagePrivate;
class GenericImage
{
public:
    typedef QSharedPointer<GenericImage> Ptr;
    typedef QSharedPointer<const GenericImage> ConstPtr;

    virtual ~GenericImage();

    static GenericImage::Ptr create(const QString & imageUrl, const QDateTime & createdTime,
                                     const QDateTime &updatedTime, const QString &imageName,
                                     int width, int height, const QString & thumbnailUrl,
                                     const QString & thumbnailFile, const QString & imageFile,
                                     int accountId = -1);
    QDateTime createdTime() const;
    QDateTime updatedTime() const;
    QString imageName() const;
    int width() const;
    int height() const;
    QString thumbnailUrl() const;
    QString imageUrl() const;
    QString thumbnailFile() const;
    QString imageFile() const;
    int accountId() const;

protected:
    QScopedPointer<GenericImagePrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(GenericImage)
    explicit GenericImage(const QString & imageUrl, const QDateTime & createdTime,
                           const QDateTime & updatedTime, const QString & imageName,
                           int width, int height, const QString & thumbnailUrl,
                           const QString & thumbnailFile,
                           const QString & imageFile, int accountId = -1);
};

bool operator==(const GenericImage::ConstPtr &image1, const GenericImage::ConstPtr &image2);

class GenericImagesDatabasePrivate;
class GenericImagesDatabase: public AbstractSocialCacheDatabase
{
    Q_OBJECT
public:
    explicit GenericImagesDatabase(const QLatin1String &databaseName);
    ~GenericImagesDatabase();

    // Account manipulation
    void purgeAccount(int accountId);

    // Images cache manipulation
    QStringList imageUrls(int accountId, bool *ok = 0) const;
    GenericImage::ConstPtr image(int accountId, const QString &imageUrl) const;

    void addImage(const QString & imageUrl, const QDateTime & createdTime,
                  const QDateTime & updatedTime, const QString & imageName,
                  int width, int height, const QString & thumbnailUrl,
                  int accountId);
    void updateImageThumbnail(const QString &GenericImageId, const QString &thumbnailFile);
    void updateImageFile(const QString &GenericImageId, const QString &imageFile);
    void removeImage(const QString &imageUrl);
    void removeImages(const QStringList &imageUrls);
    void queryImages(int accountId);

    void commit();

    QList<GenericImage::ConstPtr> images() const;

Q_SIGNALS:
    void queryFinished();

protected:
    bool read();
    void readFinished();

    bool write();
    bool createTables(QSqlDatabase database) const;
    bool dropTables(QSqlDatabase database) const;


private:
    Q_DECLARE_PRIVATE(GenericImagesDatabase)
};

#endif // GENERICIMAGESDATABASE_H
