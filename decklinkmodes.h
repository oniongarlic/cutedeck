#ifndef DECKLINKMODES_H
#define DECKLINKMODES_H

#include <QObject>
#include <QString>
#include <QList>

#include <DeckLinkAPI.h>

struct DeckLinkModeInfo
{
    Q_GADGET

    Q_PROPERTY(int32_t id MEMBER id)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(int width MEMBER width)
    Q_PROPERTY(int height MEMBER height)
    Q_PROPERTY(int fpsNumerator MEMBER fpsNumerator)
    Q_PROPERTY(int fpsDenominator MEMBER fpsDenominator)
    Q_PROPERTY(double fps READ fps)
    Q_PROPERTY(bool interlaced MEMBER interlaced)
    Q_PROPERTY(bool dci MEMBER dci)

public:
    int32_t id = 0;
    QString name;

    int width = 0;
    int height = 0;

    int fpsNumerator = 0;
    int fpsDenominator = 1;

    bool interlaced = false;
    bool dci = false;

    Q_INVOKABLE double fps() const
    {
        return double(fpsNumerator) / fpsDenominator;
    }

    Q_INVOKABLE QString resolution() const
    {
        return QString("%1x%2").arg(width).arg(height);
    }

    Q_INVOKABLE QString description() const
    {
        return QString("%1 (%2x%3, %4 fps)")
        .arg(name)
            .arg(width)
            .arg(height)
            .arg(fps(), 0, 'f', 2);
    }
};

Q_DECLARE_METATYPE(DeckLinkModeInfo)

struct DeckLinkModes
{
    Q_GADGET
    const QList<DeckLinkModeInfo> modes = {
    // SD
    {bmdModeNTSC, "NTSC", 720, 486, 30000, 1001, true},
    {bmdModePAL, "PAL", 720, 576, 25, 1, true},
    {bmdModeNTSCp, "NTSCp", 720, 486, 60000, 1001, false},
    {bmdModePALp, "PALp", 720, 576, 50, 1, false},

    // 720p
    {bmdModeHD720p50, "720p50", 1280, 720, 50, 1, false},
    {bmdModeHD720p5994, "720p59.94", 1280, 720, 60000, 1001, false},
    {bmdModeHD720p60, "720p60", 1280, 720, 60, 1, false},

    // 1080p
    {bmdModeHD1080p2398, "1080p23.98", 1920, 1080, 24000, 1001, false},
    {bmdModeHD1080p24, "1080p24", 1920, 1080, 24, 1, false},
    {bmdModeHD1080p25, "1080p25", 1920, 1080, 25, 1, false},
    {bmdModeHD1080p2997, "1080p29.97", 1920, 1080, 30000, 1001, false},
    {bmdModeHD1080p30, "1080p30", 1920, 1080, 30, 1, false},
    {bmdModeHD1080p50, "1080p50", 1920, 1080, 50, 1, false},
    {bmdModeHD1080p5994, "1080p59.94", 1920, 1080, 60000, 1001, false},
    {bmdModeHD1080p6000, "1080p60", 1920, 1080, 60, 1, false},

    // 1080i
    {bmdModeHD1080i50, "1080i50", 1920, 1080, 25, 1, true},
    {bmdModeHD1080i5994, "1080i59.94", 1920, 1080, 30000, 1001, true},
    {bmdModeHD1080i6000, "1080i60", 1920, 1080, 30, 1, true},

    // 2K
    {bmdMode2k2398, "2K23.98", 2048, 1556, 24000, 1001, false},
    {bmdMode2k24, "2K24", 2048, 1556, 24, 1, false},
    {bmdMode2k25, "2K25", 2048, 1556, 25, 1, false},

    // 2K DCI
    {bmdMode2kDCI2398, "2K DCI23.98", 2048, 1080, 24000, 1001, false, true},
    {bmdMode2kDCI24, "2K DCI24", 2048, 1080, 24, 1, false, true},
    {bmdMode2kDCI25, "2K DCI25", 2048, 1080, 25, 1, false, true},

    // UHD
    {bmdMode4K2160p2398, "2160p23.98", 3840, 2160, 24000, 1001, false},
    {bmdMode4K2160p24, "2160p24", 3840, 2160, 24, 1, false},
    {bmdMode4K2160p25, "2160p25", 3840, 2160, 25, 1, false},
    {bmdMode4K2160p2997, "2160p29.97", 3840, 2160, 30000, 1001, false},
    {bmdMode4K2160p30, "2160p30", 3840, 2160, 30, 1, false},
    {bmdMode4K2160p50, "2160p50", 3840, 2160, 50, 1, false},
    {bmdMode4K2160p5994, "2160p59.94", 3840, 2160, 60000, 1001, false},
    {bmdMode4K2160p60, "2160p60", 3840, 2160, 60, 1, false},

    // 4K DCI
    {bmdMode4kDCI2398, "4K DCI23.98", 4096, 2160, 24000, 1001, false, true},
    {bmdMode4kDCI24, "4K DCI24", 4096, 2160, 24, 1, false, true},
    {bmdMode4kDCI25, "4K DCI25", 4096, 2160, 25, 1, false, true},

    // 4K DCI (4096x2160)
    {bmdMode4kDCI2997, "4K DCI29.97", 4096, 2160, 30000, 1001, false, true},
    {bmdMode4kDCI30, "4K DCI30", 4096, 2160, 30, 1, false, true},
    {bmdMode4kDCI4795, "4K DCI47.95", 4096, 2160, 48000, 1001, false, true},
    {bmdMode4kDCI48, "4K DCI48", 4096, 2160, 48, 1, false, true},
    {bmdMode4kDCI50, "4K DCI50", 4096, 2160, 50, 1, false, true},
    {bmdMode4kDCI5994, "4K DCI59.94", 4096, 2160, 60000, 1001, false, true},
    {bmdMode4kDCI60, "4K DCI60", 4096, 2160, 60, 1, false, true},
    {bmdMode4kDCI9590, "4K DCI95.90", 4096, 2160, 96000, 1001, false, true},
    {bmdMode4kDCI96, "4K DCI96", 4096, 2160, 96, 1, false, true},
    {bmdMode4kDCI100, "4K DCI100", 4096, 2160, 100, 1, false, true},
    {bmdMode4kDCI11988, "4K DCI119.88", 4096, 2160, 120000, 1001, false, true},
    {bmdMode4kDCI120, "4K DCI120", 4096, 2160, 120, 1, false, true},

    // 8K UHD (7680x4320)
    {bmdMode8K4320p2398, "8K23.98", 7680, 4320, 24000, 1001, false},
    {bmdMode8K4320p24, "8K24", 7680, 4320, 24, 1, false},
    {bmdMode8K4320p25, "8K25", 7680, 4320, 25, 1, false},
    {bmdMode8K4320p2997, "8K29.97", 7680, 4320, 30000, 1001, false},
    {bmdMode8K4320p30, "8K30", 7680, 4320, 30, 1, false},
    {bmdMode8K4320p4795, "8K47.95", 7680, 4320, 48000, 1001, false},
    {bmdMode8K4320p48, "8K48", 7680, 4320, 48, 1, false},
    {bmdMode8K4320p50, "8K50", 7680, 4320, 50, 1, false},
    {bmdMode8K4320p5994, "8K59.94", 7680, 4320, 60000, 1001, false},
    {bmdMode8K4320p60, "8K60", 7680, 4320, 60, 1, false},

    // 8K DCI (8192x4320)
    {bmdMode8kDCI2398, "8K DCI23.98", 8192, 4320, 24000, 1001, false, true},
    {bmdMode8kDCI24, "8K DCI24", 8192, 4320, 24, 1, false, true},
    {bmdMode8kDCI25, "8K DCI25", 8192, 4320, 25, 1, false, true},
    {bmdMode8kDCI2997, "8K DCI29.97", 8192, 4320, 30000, 1001, false, true},
    {bmdMode8kDCI30, "8K DCI30", 8192, 4320, 30, 1, false, true},
    {bmdMode8kDCI4795, "8K DCI47.95", 8192, 4320, 48000, 1001, false, true},
    {bmdMode8kDCI48, "8K DCI48", 8192, 4320, 48, 1, false, true},
    {bmdMode8kDCI50, "8K DCI50", 8192, 4320, 50, 1, false, true},
    {bmdMode8kDCI5994, "8K DCI59.94", 8192, 4320, 60000, 1001, false, true},
    {bmdMode8kDCI60, "8K DCI60", 8192, 4320, 60, 1, false, true},

    // PC modes
    {bmdMode640x480p60, "640x480p60", 640, 480, 60, 1, false},
    {bmdMode800x600p60, "800x600p60", 800, 600, 60, 1, false},
    {bmdMode1440x900p50, "1440x900p50", 1440, 900, 50, 1, false},
    {bmdMode1440x900p60, "1440x900p60", 1440, 900, 60, 1, false},
    {bmdMode1440x1080p50, "1440x1080p50", 1440, 1080, 50, 1, false},
    {bmdMode1440x1080p60, "1440x1080p60", 1440, 1080, 60, 1, false},
    {bmdMode1600x1200p50, "1600x1200p50", 1600, 1200, 50, 1, false},
    {bmdMode1600x1200p60, "1600x1200p60", 1600, 1200, 60, 1, false},
    {bmdMode1920x1200p50, "1920x1200p50", 1920, 1200, 50, 1, false},
    {bmdMode1920x1200p60, "1920x1200p60", 1920, 1200, 60, 1, false},
    {bmdMode1920x1440p50, "1920x1440p50", 1920, 1440, 50, 1, false},
    {bmdMode1920x1440p60, "1920x1440p60", 1920, 1440, 60, 1, false},
    {bmdMode2560x1440p50, "2560x1440p50", 2560, 1440, 50, 1, false},
    {bmdMode2560x1440p60, "2560x1440p60", 2560, 1440, 60, 1, false},
    {bmdMode2560x1600p50, "2560x1600p50", 2560, 1600, 50, 1, false},
    {bmdMode2560x1600p60, "2560x1600p60", 2560, 1600, 60, 1, false},
    };

public:
    Q_INVOKABLE const DeckLinkModeInfo *findDeckLinkMode(BMDDisplayMode id)
    {
        auto it = std::find_if(modes.begin(), modes.end(), [id](const auto &mode) {
                return mode.id == id;
            });

        return it != modes.end() ? &*it : nullptr;
    }

    Q_INVOKABLE const qsizetype count() const {
        return modes.count();
    }

};
Q_DECLARE_METATYPE(DeckLinkModes)

#endif // DECKLINKMODES_H
