/*
 * Copyright (C) 2014 Jolla Ltd. <antti.seppala@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include <QtTest/QTest>
#include "genericimagesdatabase.h"
#include "socialsyncinterface.h"
#include "generic/genericimagecachemodel.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class GenericImageTest: public QObject
{
    Q_OBJECT
private:

private slots:
    // Perform some cleanups
    // we basically remove the whole ~/.local/share/system/privileged. While it is
    // damaging on a device, on a desktop system it should not be much
    // damaging.
    void initTestCase()
    {
        QStandardPaths::enableTestMode(true);

        QDir dir (PRIVILEGED_DATA_DIR);
        dir.removeRecursively();
    }

    void images()
    {
        QString url1("http://images.com/image1.jpg");
        QString url2("http://images.com/image2.jpg");
        QString url3("http://images.com/image3.jpg");
        QString url4("http://images.com/image4.jpg");
        QString url5("http://images.com/image5.jpg");
        QString url6("http://images.com/image6.jpg");
        QString url7("http://images.com/image7.jpg");

        QDateTime time1(QDate(2013, 1, 2), QTime(12, 34, 56));
        QDateTime time2(QDate(2012, 3, 4), QTime(10, 11, 12));
        QDateTime time3(QDate(2014, 1, 2), QTime(12, 34, 56));
        QDateTime time4(QDate(2015, 3, 4), QTime(10, 11, 12));

        QString name1("image 1");
        QString name2("image 2");
        QString name3("image 3");
        QString name4("image 4");
        QString name5("image 5");
        QString name6("image 6");
        QString name7("image 7");

        int width1(640);
        int width2(1024);
        int width3(1280);
        int width4(1920);
        int width5(2500);
        int width6(250);
        int width7(400);

        int height1(480);
        int height2(768);
        int height3(720);
        int height4(1080);
        int height5(1600);
        int height6(300);
        int height7(480);

        QString thumb1("file://thumbails/thbm1.jpg");
        QString thumb2("file://thumbails/thbm2.jpg");
        QString thumb3("file://thumbails/thbm3.jpg");
        QString thumb4("file://thumbails/thbm4.jpg");
        QString thumb5("file://thumbails/thbm5.jpg");
        QString thumb6("file://thumbails/thbm6.jpg");
        QString thumb7("file://thumbails/thbm7.jpg");

        int account1(1);
        int account2(2);

        GenericImagesDatabase database(QLatin1String("test.db"));

        database.addImage(url1, time1, time1, name1, width1, height1, thumb1, account1);
        database.addImage(url2, time1, time2, name2, width2, height2, thumb2, account1);
        database.addImage(url3, time3, time3, name3, width3, height3, thumb3, account2);
        database.addImage(url4, time4, time4, name4, width4, height4, thumb4, account2);
        database.addImage(url5, time1, time1, name5, width5, height5, thumb5, account1);
        database.addImage(url6, time2, time2, name6, width6, height6, thumb6, account2);
        database.addImage(url7, time1, time1, name7, width7, height7, thumb7, account1);

        database.commit();
        database.wait();
        QCOMPARE(database.writeStatus(), AbstractSocialCacheDatabase::Finished);

        bool ok = false;
        QStringList imageUrls = database.imageUrls(account1, &ok);
        QCOMPARE(ok, true);
        QCOMPARE(imageUrls.count(), 4);
        QCOMPARE(imageUrls.contains(url1), true);
        QCOMPARE(imageUrls.contains(url2), true);
        QCOMPARE(imageUrls.contains(url5), true);
        QCOMPARE(imageUrls.contains(url7), true);

        ok = false;
        imageUrls = database.imageUrls(account2, &ok);
        QCOMPARE(ok, true);
        QCOMPARE(imageUrls.count(), 3);
        QCOMPARE(imageUrls.contains(url3), true);
        QCOMPARE(imageUrls.contains(url4), true);
        QCOMPARE(imageUrls.contains(url6), true);

        database.purgeAccount(account1);
        database.commit();
        database.wait();

        ok = false;
        imageUrls = database.imageUrls(account1, &ok);
        QCOMPARE(ok, true);
        QCOMPARE(imageUrls.count(), 0);

        ok = false;
        imageUrls = database.imageUrls(account2, &ok);
        QCOMPARE(ok, true);
        QCOMPARE(imageUrls.count(), 3);
        QCOMPARE(imageUrls.contains(url3), true);
        QCOMPARE(imageUrls.contains(url4), true);
        QCOMPARE(imageUrls.contains(url6), true);

        database.removeImage(url3);
        database.commit();
        database.wait();

        ok = false;
        imageUrls = database.imageUrls(account2, &ok);
        QCOMPARE(ok, true);
        QCOMPARE(imageUrls.count(), 2);
        QCOMPARE(imageUrls.contains(url4), true);
        QCOMPARE(imageUrls.contains(url6), true);

        QStringList imgs;
        imgs.append(url4);
        imgs.append(url6);
        database.removeImages(imgs);
        database.commit();
        database.wait();

        ok = false;
        imageUrls = database.imageUrls(account2, &ok);
        QCOMPARE(ok, true);
        QCOMPARE(imageUrls.count(), 0);

        database.addImage(url1, time1, time2, name1, width1, height1, thumb1, account1);
        database.addImage(url2, time1, time2, name2, width2, height2, thumb2, account1);
        database.addImage(url3, time3, time3, name3, width3, height3, thumb3, account2);
        database.addImage(url4, time4, time4, name4, width4, height4, thumb4, account2);
        database.addImage(url5, time1, time1, name5, width5, height5, thumb5, account1);
        database.addImage(url6, time2, time2, name6, width6, height6, thumb6, account2);
        database.addImage(url7, time1, time1, name7, width7, height7, thumb7, account1);
        database.commit();
        database.wait();

        database.queryImages(account1);
        database.commit();
        database.wait();

        QList<GenericImage::ConstPtr> images = database.images();
        QCOMPARE(images.count(), 4);

        GenericImage::ConstPtr img = images.at(0);
        QCOMPARE(img->imageUrl(), url1);
        QCOMPARE(img->createdTime(), time1);
        QCOMPARE(img->updatedTime(), time2);
        QCOMPARE(img->imageName(), name1);
        QCOMPARE(img->width(), width1);
        QCOMPARE(img->height(), height1);
        QCOMPARE(img->thumbnailFile(), thumb1);
        QCOMPARE(img->accountId(), account1);
    }

    void cleanupTestCase()
    {
        // Do the same cleanups
        QDir dir (PRIVILEGED_DATA_DIR);
        dir.removeRecursively();
    }
};

QTEST_MAIN(GenericImageTest)

#include "main.moc"
