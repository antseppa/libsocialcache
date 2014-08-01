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

#ifndef GENERICIMAGEDOWNLOADER_H
#define GENERICIMAGEDOWNLOADER_H

#include <QtCore/QObject>

#include "abstractimagedownloader.h"

class GenericImageCacheModel;
class GenericImageDownloaderWorkerObject;
class GenericImageDownloaderPrivate;
class GenericImageDownloader : public AbstractImageDownloader
{
    Q_OBJECT
public:
    enum ImageType {
        ThumbnailImage,
        FullImage
    };

    explicit GenericImageDownloader(QObject *parent = 0);
    virtual ~GenericImageDownloader();

    // tracking object lifetime of models connected to this downloader.
    void addModelToHash(GenericImageCacheModel *model);
    void removeModelFromHash(GenericImageCacheModel *model);

protected:
    QString outputFile(const QString &url, const QVariantMap &data) const;

    void dbQueueImage(const QString &url, const QVariantMap &data, const QString &file);
    void dbWrite();

private Q_SLOTS:
    void invokeSpecificModelCallback(const QString &url, const QString &path, const QVariantMap &metadata);

private:
    Q_DECLARE_PRIVATE(GenericImageDownloader)
};

#endif // GENERICIMAGEDOWNLOADER_H
