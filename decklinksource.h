#ifndef DECKLINKSOURCEK_H
#define DECKLINKSOURCEK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQueue>
#include <QMutex>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

#include "DeckLinkAPI.h"

#include "decklink.h"

class DeckLinkInputCallback;

class CUTEDECK_EXPORT Decklinksource: public QObject
{
    friend DeckLinkInputCallback;
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* decklink READ getDecklink WRITE setDecklink NOTIFY decklinkChanged FINAL REQUIRED)
    Q_PROPERTY(QObject* videoSink READ getVideoSink WRITE setVideoSink NOTIFY videoSinkChanged FINAL)
    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged FINAL)
    Q_PROPERTY(quint32 frameCount READ frameCount NOTIFY frameCountChanged FINAL)
    Q_PROPERTY(QSize frameSize READ frameSize NOTIFY frameSizeChanged FINAL)
    Q_PROPERTY(qreal fps READ fps NOTIFY fpsChanged FINAL)
    Q_PROPERTY(bool audio READ audio WRITE setAudio NOTIFY audioChanged FINAL)

    Q_PROPERTY(int profile READ profile NOTIFY profileChanged FINAL)

public:
    explicit Decklinksource(QObject *parent = nullptr);
    bool haveDeckLink() const;

    // xxx { mode, width, height, fps } ?
    enum InputFormat {
        VideoSDPAL=bmdModePAL,
        VideoSDNTSC=bmdModeNTSC,
        VideoSDPALp=bmdModePALp,
        VideoSDNTSCp=bmdModeNTSCp,

        VideoHD720p50=bmdModeHD720p50,
        VideoHD720p59=bmdModeHD720p5994,
        VideoHD720p60=bmdModeHD720p60,

        VideoHD1080p24=bmdModeHD1080p24,
        VideoHD1080p25=bmdModeHD1080p25,
        VideoHD1080p30=bmdModeHD1080p30,
        VideoHD1080p50=bmdModeHD1080p50,
        VideoHD1080p60=bmdModeHD1080p6000,
    };
    Q_ENUM(InputFormat);

    enum InputConnector {
        InputConnectorSDI=bmdVideoConnectionSDI,
        InputConnectorOpticalSDI=bmdVideoConnectionOpticalSDI,
        InputConnectorHDMI=bmdVideoConnectionHDMI,
        InputConnectorComponent=bmdVideoConnectionComponent,
        InputConnectorComposite=bmdVideoConnectionComposite,
        InputConnectorSVideo=bmdVideoConnectionSVideo,
    };
    Q_ENUM(InputConnector);

    Q_INVOKABLE void setFramebufferSize(QSize size);
    Q_INVOKABLE void setVideoSink(QObject *videosink);

    Q_INVOKABLE bool setInput(uint index);

    Q_INVOKABLE bool setInputConnector(InputConnector ic);

    Q_INVOKABLE qint32 getMode();
    Q_INVOKABLE bool setMode(qint32 mode);

    Q_INVOKABLE bool grabFrame();
    Q_INVOKABLE QImage getImage();

    QObject *getVideoSink() const;
    QObject *getDecklink() const;
    void setDecklink(QObject *newDecklink);

    bool streaming() const;
    quint32 frameCount() const;
    QSize frameSize() const;
    uint fps() const;

    bool audio() const;
    void setAudio(bool newAudio);

    int profile() const;

signals:
    void haveDeckLinkChanged();
    void videoSinkChanged();
    void devicesChanged();
    void decklinkChanged();
    void frameQueued(quint32 frames);
    void inputModeChanged(quint32 newMode, quint32 oldMode);
    void streamingChanged();
    void restartStream();
    void frameCountChanged(quint32 frames);
    void validSignal();
    void invalidSignal();
    void frameSizeChanged();
    void fpsChanged(qreal fps);
    void audioChanged();
    void frameGrabbed();
    void profileChanged();
    void restartRequired();

public slots:
    bool enableInput();
    bool disableInput();
protected:
    void imageToBuffer(const QImage &frame);
    void newFrame(IDeckLinkVideoInputFrame *frame, IDeckLinkAudioInputPacket *audio);
    void modeChanged(quint32 mode, BMDPixelFormat format, const QSize size, float fps);
    void setStreaming(bool newStreaming);
    void noInputSource();
protected slots:
    void processFrame();

private:
    DeckLink *m_decklink = nullptr;
    DeckLinkInputCallback *m_icb;

    int m_current=-1;
    QSize m_fbsize;
    QVideoSink *m_videosink=nullptr;

    QMutex m_mutex;
    QQueue<IDeckLinkVideoInputFrame *> m_frames;
    QQueue<IDeckLinkAudioInputPacket *> m_apackets;
    uint64_t m_framecounter=0;

    bool m_streaming=false;
    bool m_audio=false;
    uint m_achannels=2;
    uint m_abitdepth=16;

    // Active output/input/keyer and frame    
    IDeckLinkInput *m_input=nullptr;
    IDeckLinkOutput *m_output=nullptr;
    IDeckLinkMutableVideoFrame* m_frame=nullptr;

    IDeckLinkVideoConversion *m_conv;

    BMDDisplayMode m_mode=bmdModeHD1080p30; // bmdModeHD1080p6000; //
    BMDPixelFormat m_format;
    QSize m_frameSize;
    qreal m_fps=0;
    bool m_signal=false;
    bool m_invalid=false;
    bool m_grabframe=false;
    bool m_autorestart=true;
    QImage m_frameImage;
    int m_profile=0;
};

#endif // DECKLINKSOURCE_H
