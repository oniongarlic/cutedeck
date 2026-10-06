#ifndef DECKLINK_H
#define DECKLINK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

#include "DeckLinkAPI.h"

struct DeckLinkDevice
{
    QString name;
    QString model;
    bool valid;
    QVariantMap properties;
    QSize size;
    uint fps;

    IDeckLink *dev;
    IDeckLinkOutput *output;
    IDeckLinkInput *input;
    IDeckLinkKeyer *key;
    BMDDisplayMode mode;
    IDeckLinkMutableVideoFrame *frame;

    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString model MEMBER model)
    Q_PROPERTY(uint fps MEMBER fps)
    Q_PROPERTY(QVariantMap properties MEMBER properties)
};
Q_DECLARE_METATYPE(DeckLinkDevice)

class DeckLink : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool haveDeckLink READ haveDeckLink NOTIFY haveDeckLinkChanged FINAL)
    Q_PROPERTY(int devices READ devices NOTIFY devicesChanged FINAL)
public:
    explicit DeckLink(QObject *parent = nullptr);
    bool haveDeckLink() const;
    int devices() const;

    Q_INVOKABLE QVariantMap getDeviceProperties(int index);

    DeckLinkDevice *getDevice(int index);

signals:
    void haveDeckLinkChanged();
    void devicesChanged();

private:
    int m_devices=0;
    bool m_haveDeckLink=false;
    QList<DeckLinkDevice *> m_devs;
};

#endif // DECKLINKS_H
