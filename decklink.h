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

#include "decklinkmodes.h"

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

    QVariantList outputConnectors;
    QVariantList inputConnectors;

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
    Q_PROPERTY(QVariantList outputConnectors MEMBER outputConnectors)
    Q_PROPERTY(QVariantList inputConnectors MEMBER inputConnectors)
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

    // Video modes aliases so we can use them in QtQuick nicely
    enum DeviceModes : int32_t {
        // SD
        Video_SD_NTSC          = bmdModeNTSC,
        Video_SD_NTSC_2398     = bmdModeNTSC2398,
        Video_SD_PAL           = bmdModePAL,
        Video_SD_NTSCp         = bmdModeNTSCp,
        Video_SD_PALp          = bmdModePALp,

        // HD 720
        Video_HD_720p50        = bmdModeHD720p50,
        Video_HD_720p5994      = bmdModeHD720p5994,
        Video_HD_720p60        = bmdModeHD720p60,

        // Full HD 1080 progressive
        Video_FHD_1080p2398    = bmdModeHD1080p2398,
        Video_FHD_1080p24      = bmdModeHD1080p24,
        Video_FHD_1080p25      = bmdModeHD1080p25,
        Video_FHD_1080p2997    = bmdModeHD1080p2997,
        Video_FHD_1080p30      = bmdModeHD1080p30,
        Video_FHD_1080p4795    = bmdModeHD1080p4795,
        Video_FHD_1080p48      = bmdModeHD1080p48,
        Video_FHD_1080p50      = bmdModeHD1080p50,
        Video_FHD_1080p5994    = bmdModeHD1080p5994,
        Video_FHD_1080p60      = bmdModeHD1080p6000,
        Video_FHD_1080p9590    = bmdModeHD1080p9590,
        Video_FHD_1080p96      = bmdModeHD1080p96,
        Video_FHD_1080p100     = bmdModeHD1080p100,
        Video_FHD_1080p11988   = bmdModeHD1080p11988,
        Video_FHD_1080p120     = bmdModeHD1080p120,

        // Full HD 1080 interlaced
        Video_FHD_1080i50      = bmdModeHD1080i50,
        Video_FHD_1080i5994    = bmdModeHD1080i5994,
        Video_FHD_1080i60      = bmdModeHD1080i6000,

        // 2K
        Video_2K_2398          = bmdMode2k2398,
        Video_2K_24            = bmdMode2k24,
        Video_2K_25            = bmdMode2k25,

        // 2K DCI
        Video_DCI_2K_2398      = bmdMode2kDCI2398,
        Video_DCI_2K_24        = bmdMode2kDCI24,
        Video_DCI_2K_25        = bmdMode2kDCI25,
        Video_DCI_2K_2997      = bmdMode2kDCI2997,
        Video_DCI_2K_30         = bmdMode2kDCI30,
        Video_DCI_2K_4795      = bmdMode2kDCI4795,
        Video_DCI_2K_48        = bmdMode2kDCI48,
        Video_DCI_2K_50        = bmdMode2kDCI50,
        Video_DCI_2K_5994      = bmdMode2kDCI5994,
        Video_DCI_2K_60        = bmdMode2kDCI60,
        Video_DCI_2K_9590      = bmdMode2kDCI9590,
        Video_DCI_2K_96        = bmdMode2kDCI96,
        Video_DCI_2K_100       = bmdMode2kDCI100,
        Video_DCI_2K_11988     = bmdMode2kDCI11988,
        Video_DCI_2K_120       = bmdMode2kDCI120,

        // UHD 4K (3840x2160)
        Video_UHD_4Kp2398      = bmdMode4K2160p2398,
        Video_UHD_4Kp24        = bmdMode4K2160p24,
        Video_UHD_4Kp25        = bmdMode4K2160p25,
        Video_UHD_4Kp2997      = bmdMode4K2160p2997,
        Video_UHD_4Kp30        = bmdMode4K2160p30,
        Video_UHD_4Kp4795      = bmdMode4K2160p4795,
        Video_UHD_4Kp48        = bmdMode4K2160p48,
        Video_UHD_4Kp50        = bmdMode4K2160p50,
        Video_UHD_4Kp5994      = bmdMode4K2160p5994,
        Video_UHD_4Kp60        = bmdMode4K2160p60,
        Video_UHD_4Kp9590      = bmdMode4K2160p9590,
        Video_UHD_4Kp96        = bmdMode4K2160p96,
        Video_UHD_4Kp100       = bmdMode4K2160p100,
        Video_UHD_4Kp11988     = bmdMode4K2160p11988,
        Video_UHD_4Kp120       = bmdMode4K2160p120,

        // 4K DCI (4096x2160)
        Video_DCI_4K_2398      = bmdMode4kDCI2398,
        Video_DCI_4K_24        = bmdMode4kDCI24,
        Video_DCI_4K_25        = bmdMode4kDCI25,
        Video_DCI_4K_2997      = bmdMode4kDCI2997,
        Video_DCI_4K_30        = bmdMode4kDCI30,
        Video_DCI_4K_4795      = bmdMode4kDCI4795,
        Video_DCI_4K_48        = bmdMode4kDCI48,
        Video_DCI_4K_50        = bmdMode4kDCI50,
        Video_DCI_4K_5994      = bmdMode4kDCI5994,
        Video_DCI_4K_60        = bmdMode4kDCI60,
        Video_DCI_4K_9590      = bmdMode4kDCI9590,
        Video_DCI_4K_96        = bmdMode4kDCI96,
        Video_DCI_4K_100       = bmdMode4kDCI100,
        Video_DCI_4K_11988     = bmdMode4kDCI11988,
        Video_DCI_4K_120       = bmdMode4kDCI120,

        // UHD 8K (7680x4320)
        Video_UHD_8Kp2398      = bmdMode8K4320p2398,
        Video_UHD_8Kp24        = bmdMode8K4320p24,
        Video_UHD_8Kp25        = bmdMode8K4320p25,
        Video_UHD_8Kp2997      = bmdMode8K4320p2997,
        Video_UHD_8Kp30        = bmdMode8K4320p30,
        Video_UHD_8Kp4795      = bmdMode8K4320p4795,
        Video_UHD_8Kp48        = bmdMode8K4320p48,
        Video_UHD_8Kp50        = bmdMode8K4320p50,
        Video_UHD_8Kp5994      = bmdMode8K4320p5994,
        Video_UHD_8Kp60        = bmdMode8K4320p60,

        // 8K DCI (8192x4320)
        Video_DCI_8K_2398      = bmdMode8kDCI2398,
        Video_DCI_8K_24        = bmdMode8kDCI24,
        Video_DCI_8K_25        = bmdMode8kDCI25,
        Video_DCI_8K_2997      = bmdMode8kDCI2997,
        Video_DCI_8K_30        = bmdMode8kDCI30,
        Video_DCI_8K_4795      = bmdMode8kDCI4795,
        Video_DCI_8K_48        = bmdMode8kDCI48,
        Video_DCI_8K_50        = bmdMode8kDCI50,
        Video_DCI_8K_5994      = bmdMode8kDCI5994,
        Video_DCI_8K_60        = bmdMode8kDCI60,

        // PC / VESA
        Video_PC_640x480p60    = bmdMode640x480p60,
        Video_PC_800x600p60    = bmdMode800x600p60,
        Video_PC_1440x900p50   = bmdMode1440x900p50,
        Video_PC_1440x900p60   = bmdMode1440x900p60,
        Video_PC_1440x1080p50  = bmdMode1440x1080p50,
        Video_PC_1440x1080p60  = bmdMode1440x1080p60,
        Video_PC_1600x1200p50  = bmdMode1600x1200p50,
        Video_PC_1600x1200p60  = bmdMode1600x1200p60,
        Video_PC_1920x1200p50  = bmdMode1920x1200p50,
        Video_PC_1920x1200p60  = bmdMode1920x1200p60,
        Video_PC_1920x1440p50  = bmdMode1920x1440p50,
        Video_PC_1920x1440p60  = bmdMode1920x1440p60,
        Video_PC_2560x1440p50  = bmdMode2560x1440p50,
        Video_PC_2560x1440p60  = bmdMode2560x1440p60,
        Video_PC_2560x1600p50  = bmdMode2560x1600p50,
        Video_PC_2560x1600p60  = bmdMode2560x1600p60

    };
    Q_ENUM(DeviceModes);

    Q_INVOKABLE QVariantList getDevices(bool includeInactive=false);
    Q_INVOKABLE QVariantMap getDeviceProperties(int index);
    Q_INVOKABLE QString version() const { return m_api_ver; }

    Q_INVOKABLE DeckLinkDevice device(int index);

    Q_INVOKABLE const DeckLinkModeInfo *getModeInfo(DeviceModes mode) const;

    DeckLinkDevice* getDevice(int index);

signals:
    void haveDeckLinkChanged();
    void devicesChanged();

private:    
    int m_devices=0;
    bool m_haveDeckLink=false;
    QList<DeckLinkDevice *> m_devs;

    DeckLinkModes m_modes;

    const QString m_api_ver=BLACKMAGIC_DECKLINK_API_VERSION_STRING;
};

#endif // DECKLINKS_H
