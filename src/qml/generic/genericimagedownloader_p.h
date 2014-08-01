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

#ifndef GENERICIMAGEDOWNLOADER_P_H
#define GENERICIMAGEDOWNLOADER_P_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QSet>

#include "abstractimagedownloader_p.h"
#include "genericimagedownloader.h"
#include "genericimagesdatabase.h"

class GenericImageCacheModel;
class GenericImageDownloaderPrivate: public AbstractImageDownloaderPrivate
{
public:
    explicit GenericImageDownloaderPrivate(GenericImageDownloader *q);
    virtual ~GenericImageDownloaderPrivate();

    GenericImagesDatabase *database;
    QSet<GenericImageCacheModel*> m_connectedModels;

private:
    Q_DECLARE_PUBLIC(GenericImageDownloader)
};

#endif // GENERICIMAGEDOWNLOADER_P_H
