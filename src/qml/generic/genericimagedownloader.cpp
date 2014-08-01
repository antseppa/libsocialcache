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

#include "genericimagedownloader.h"
#include "genericimagedownloader_p.h"
#include "genericimagedownloaderconstants_p.h"

#include "genericimagecachemodel.h"

#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>

#include <QtDebug>

static const char *MODEL_KEY = "model";

GenericImageDownloaderPrivate::GenericImageDownloaderPrivate(GenericImageDownloader *q)
    : AbstractImageDownloaderPrivate(q)
{
    database = new GenericImagesDatabase(QLatin1String("FOO")); // FIXME
}

GenericImageDownloaderPrivate::~GenericImageDownloaderPrivate()
{
    delete database;
}

GenericImageDownloader::GenericImageDownloader(QObject *parent) :
    AbstractImageDownloader(*new GenericImageDownloaderPrivate(this), parent)
{
    connect(this, &AbstractImageDownloader::imageDownloaded,
            this, &GenericImageDownloader::invokeSpecificModelCallback);
}

GenericImageDownloader::~GenericImageDownloader()
{
}

void GenericImageDownloader::addModelToHash(GenericImageCacheModel *model)
{
    Q_D(GenericImageDownloader);
    d->m_connectedModels.insert(model);
}

void GenericImageDownloader::removeModelFromHash(GenericImageCacheModel *model)
{
    Q_D(GenericImageDownloader);
    d->m_connectedModels.remove(model);
}

/*
 * A GenericImageDownloader can be connected to multiple models.
 * Instead of connecting the imageDownloaded signal directly to the
 * model, we connect it to this slot, which retrieves the target model
 * from the metadata map and invokes its callback directly.
 * This avoids a possibly large number of signal connections + invocations.
 */
void GenericImageDownloader::invokeSpecificModelCallback(const QString &url, const QString &path, const QVariantMap &metadata)
{
    Q_D(GenericImageDownloader);
    GenericImageCacheModel *model = static_cast<GenericImageCacheModel*>(metadata.value(MODEL_KEY).value<void*>());

    // check to see if the model was destroyed in the meantime.
    // If not, we can directly invoke the callback.
    if (d->m_connectedModels.contains(model)) {
        model->imageDownloaded(url, path, metadata);
    }
}

QString GenericImageDownloader::outputFile(const QString &url,
                                                        const QVariantMap &data) const
{
    Q_UNUSED(url);

    // We create the identifier by appending the type to the real identifier
    QString identifier = data.value(QLatin1String(IDENTIFIER_KEY)).toString();
    if (identifier.isEmpty()) {
        return QString();
    }

    QString typeString = data.value(QLatin1String(TYPE_KEY)).toString();
    if (typeString.isEmpty()) {
        return QString();
    }

    identifier.append(typeString);

    return makeOutputFile(SocialSyncInterface::Generic, SocialSyncInterface::Images, identifier);
}

void GenericImageDownloader::dbQueueImage(const QString &url, const QVariantMap &data,
                                                       const QString &file)
{
    Q_D(GenericImageDownloader);
    Q_UNUSED(url);

    QString identifier = data.value(QLatin1String(IDENTIFIER_KEY)).toString();
    if (identifier.isEmpty()) {
        return;
    }
    int type = data.value(QLatin1String(TYPE_KEY)).toInt();

    switch (type) {
    case ThumbnailImage:
        d->database->updateImageThumbnail(identifier, file);
        break;
    case FullImage:
        d->database->updateImageFile(identifier, file);
        break;
    }
}

void GenericImageDownloader::dbWrite()
{
    Q_D(GenericImageDownloader);

    d->database->commit();
}
