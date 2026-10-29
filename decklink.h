#ifndef DECKLINK_H
#define DECKLINK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

#include "cutedeck_global.h"

#include "DeckLinkAPI.h"
#include "DeckLinkAPIVersion.h"

struct CUTEDECK_EXPORT DeckLinkDevice
{
    QString name;
    QString model;
    qint64 persistentID;
    qint64 topologicalID;
    bool valid;
    bool active;
    QSize size;
    uint fps;
    quint64 a;
    quint64 audioChannels;
    quint64 duplex;
    quint64 profile;

    QVariantMap properties;
    QVariantList outputModes;
    QVariantList inputModes;

    IDeckLink *dev=nullptr;
    IDeckLinkOutput *output=nullptr;
    IDeckLinkInput *input=nullptr;
    IDeckLinkKeyer *key=nullptr;

    IDeckLinkStatistics *statistics=nullptr;
    IDeckLinkNotification *notification=nullptr;

    BMDDisplayMode mode;
    IDeckLinkMutableVideoFrame *frame=nullptr;

    Q_GADGET    
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString model MEMBER model)
    Q_PROPERTY(qint64 persistentID MEMBER persistentID)
    Q_PROPERTY(qint64 topologicalID MEMBER topologicalID)
    Q_PROPERTY(bool active MEMBER active)
    Q_PROPERTY(uint fps MEMBER fps)
    Q_PROPERTY(uint audioChannels MEMBER audioChannels)
    Q_PROPERTY(QVariantMap properties MEMBER properties)
    Q_PROPERTY(QVariantList outputModes MEMBER outputModes)
    Q_PROPERTY(QVariantList inputModes MEMBER inputModes)
};
Q_DECLARE_METATYPE(DeckLinkDevice)

class CUTEDECK_EXPORT DeckLink : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool haveDeckLink READ haveDeckLink NOTIFY haveDeckLinkChanged FINAL)
    Q_PROPERTY(int devices READ devices NOTIFY devicesChanged FINAL)
public:
    explicit DeckLink(QObject *parent = nullptr);
    ~DeckLink();
    bool haveDeckLink() const;
    int devices() const;

    Q_INVOKABLE QVariantList getDevices(bool includeInactive=false);
    Q_INVOKABLE QVariantMap getDeviceProperties(int index);
    Q_INVOKABLE QString version() const { return m_api_ver; }

    Q_INVOKABLE DeckLinkDevice device(int index);

    DeckLinkDevice* getDevice(int index);

signals:
    void haveDeckLinkChanged();
    void devicesChanged();

private:    
    int m_devices=0;
    bool m_haveDeckLink=false;
    QList<DeckLinkDevice *> m_devs;

    const QString m_api_ver=BLACKMAGIC_DECKLINK_API_VERSION_STRING;
};

#endif // DECKLINKS_H
