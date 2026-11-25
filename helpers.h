#ifndef HELPERS_H
#define HELPERS_H

#include <qbuffer.h>
#include <qimage.h>
#include <qobject.h>

static QByteArray imageToBytes(const QImage &image)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

static QImage bytesToImage(const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return {};

    QImage image;
    if (!image.loadFromData(bytes))
        return {};
    return image;
}

#endif // HELPERS_H
