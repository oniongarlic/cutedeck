#include "decklinksource.h"

#include <QUrl>
#include <QDebug>
#include <QVideoFrameFormat>

#ifndef Q_OS_WIN
#define BOOL bool
#endif

class DeckLinkInputCallback: public IDeckLinkInputCallback
{
public:
    DeckLinkInputCallback(Decklinksource *src) :
        m_src(src)
    {
    }

    HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) override {
        BMDTimeValue time;
        BMDTimeScale scale;
        BMDPixelFormat format;
        const char *name;
        float fps;

        newDisplayMode->GetFrameRate(&time, &scale);
        //newDisplayMode->GetName(&name);
        QSize size(newDisplayMode->GetWidth(), newDisplayMode->GetHeight());
        fps=(float)scale/(float)time;

        qDebug() << "VideoInputFormatChanged" << notificationEvents << detectedSignalFlags;
        qDebug() << "Mode: " << newDisplayMode->GetDisplayMode() << size << fps  << newDisplayMode->GetFlags();
        qDebug() << time << scale;

        // Check input formats
        if (detectedSignalFlags & bmdDetectedVideoInputRGB444) {
            format=bmdFormat8BitARGB;
        } else if (detectedSignalFlags & bmdDetectedVideoInputYCbCr422) {
            format=bmdFormat8BitYUV;
        } else {
            // Fallback
            qWarning("Unknown frame format?");
            format=bmdFormat8BitYUV;
        }

        m_src->modeChanged(newDisplayMode->GetDisplayMode(), format, size, fps);

        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override {                
        if (!videoFrame) {
            qDebug("Invalid input frame");
            return S_OK;
        }

        if (videoFrame->GetFlags() & bmdFrameHasNoInputSource) {
            m_src->noInputSource();
            return S_OK;
        }

        if (videoFrame && !audioPacket) {
            videoFrame->AddRef();
            m_src->newFrame(videoFrame, nullptr);
        } else {
            videoFrame->AddRef();
            audioPacket->AddRef();
            m_src->newFrame(videoFrame, audioPacket);
        }

        return S_OK;
    }
    HRESULT	STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) override {
        Q_UNUSED(iid)
        Q_UNUSED(ppv)
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }

private:
    Decklinksource*  m_src;        
};

Decklinksource::Decklinksource(QObject *parent)
    : QObject{parent}
{
    m_fbsize.setWidth(1920);
    m_fbsize.setHeight(1080);
    m_audio=false;
    m_icb=new DeckLinkInputCallback(this);

#ifdef Q_OS_WIN
    HRESULT result;
    result = CoCreateInstance(CLSID_CDeckLinkVideoConversion, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&m_conv);
#else
    m_conv = CreateVideoConversionInstance();
#endif

    QObject::connect(this, &Decklinksource::frameQueued,
                     this, &Decklinksource::processFrame,
                     Qt::QueuedConnection);
}

void Decklinksource::setVideoSink(QObject *videosink)
{
    if (videosink==nullptr) {
        m_videosink=nullptr;
        return;
    }    

    m_videosink = qobject_cast<QVideoSink*>(videosink);
    if (!m_videosink) {
        qWarning("Invalid video sink set");
        return;
    }

    emit videoSinkChanged();
}

QObject *Decklinksource::getVideoSink() const
{
    return m_videosink;
}

bool Decklinksource::setInput(uint index)
{
    HRESULT result;
    DeckLinkDevice *d;

    if (!m_decklink->haveDeckLink()) {
        qWarning("No decklink device found");
        return false;
    }

    if (m_streaming) {
        qWarning("Input device in use, can not change");
        return false;
    }

    d=m_decklink->getDevice(index);

    if (!d) {
        qWarning("Invalid decklink device");
        return false;
    }

    if (!d->input) {
        qWarning("Decklink device has no input");
        return false;
    }

    m_current=index;
    m_input=d->input;
    m_output=d->output;    

    qDebug() << "Decklink input set to " << m_current << d->name;

    result=m_input->SetCallback(m_icb);
    if (result!=S_OK)
        qWarning("Failed to set input callback");

    return true;
}

bool Decklinksource::setInputConnector(InputConnector ic)
{
    return false;
}

qint32 Decklinksource::getMode()
{
    return m_mode;
}

bool Decklinksource::setMode(qint32 mode)
{
    m_mode=(BMDDisplayMode)mode;

    return true;
}

bool Decklinksource::grabFrame()
{
    m_grabframe=true;

    return true;
}

QImage Decklinksource::getImage()
{
    return m_frameImage;
}

void Decklinksource::imageToBuffer(const QImage &frame)
{
    uint8_t* deckLinkBuffer=nullptr;

    if (m_frame->GetBytes((void**)&deckLinkBuffer) != S_OK) {
        qWarning("Failed to get buffer pointer");
        return;
    }

    for (int i=0;i<frame.height(); i++) {
        memcpy(deckLinkBuffer, frame.constScanLine(i), frame.bytesPerLine());
        deckLinkBuffer += m_frame->GetRowBytes();
    }
}

void Decklinksource::newFrame(IDeckLinkVideoInputFrame *frame, IDeckLinkAudioInputPacket* audio)
{
    QMutexLocker locker(&m_mutex);
    m_frames.enqueue(frame);
    if (audio) {
        m_apackets.enqueue(audio);
    }
    emit frameQueued(m_frames.size());
}

void Decklinksource::modeChanged(quint32 mode, BMDPixelFormat format, const QSize size, float fps)
{
    qDebug() << "Input mode detected " << (BMDDisplayMode)mode << size << fps;

    m_frameSize=size;
    emit frameSizeChanged();

    m_fps=fps;
    emit fpsChanged(m_fps);

    if (m_mode==mode && m_format==format) {
        qDebug() << "Mode and format as requested, valid.";
        emit validSignal();
        return;
    }

    qDebug() << "Capture restart required, got new mode or format" << mode << format << ", requested was" << m_mode << m_format;

    quint32 old=m_mode;
    m_mode=(BMDDisplayMode)mode;
    m_format=format;

    emit inputModeChanged(m_mode, old);

    if (m_autorestart) {
        qDebug("Restarting stream with detected mode");
        m_input->StopStreams();
        m_input->FlushStreams();
        m_input->DisableVideoInput();
        if (m_input->EnableVideoInput(m_mode, m_format, bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection)!=S_OK) {
            qWarning("Failed to enable input");
        }
        m_input->StartStreams();
    } else {
        qWarning("Manual restart required");
        emit restartRequired();
    }
}

void Decklinksource::processFrame()
{
    QMutexLocker locker(&m_mutex);
    uint8_t *deckLinkBuffer=nullptr;
    uint32_t h,w;
    BMDPixelFormat pf;    
    HRESULT result;
    IDeckLinkVideoInputFrame *frame=nullptr;
    IDeckLinkAudioInputPacket *ap=nullptr;
    BMDTimeValue stime, ftime;
    BMDTimeScale vscale=1001;
    void *abuffer;

    if (m_frames.isEmpty()) {
        qWarning("No frames queueed ?");
        return;
    }    

    frame=m_frames.dequeue();

    if (m_audio) {
        long fc;
        ap=m_apackets.dequeue();
        fc=ap->GetSampleFrameCount();
        qDebug() << "Audio" << fc << fc*m_achannels*(m_abitdepth/8);
        if (ap->GetBytes(&abuffer)==S_OK) {

        }
    }

    locker.unlock();

    h=frame->GetHeight();
    w=frame->GetWidth();
    pf=frame->GetPixelFormat();
    frame->GetStreamTime(&stime, &ftime, vscale);

    qDebug() << stime << ftime << m_framecounter;

    QVideoFrameFormat vff(QSize(w,h), QVideoFrameFormat::Format_ARGB8888);
    vff.setStreamFrameRate(m_fps);

    QVideoFrame vf(vff);

    if (frame->GetBytes((void**)&deckLinkBuffer)!= S_OK)
        goto out;

    if (!deckLinkBuffer)
        goto out;

    switch (pf) {
    case bmdFormat8BitYUV: {        
        IDeckLinkMutableVideoFrame* rgbaFrame = nullptr;
        result = m_output->CreateVideoFrame(
            (int32_t)w,
            (int32_t)h,
            (int32_t)w * 4,
            bmdFormat8BitARGB,
            bmdFrameFlagDefault,
            &rgbaFrame);

        if (result!=S_OK) {
            qWarning("Failed to create RGB conversion frame!");
            goto out;
        }

        result=m_conv->ConvertFrame(frame, rgbaFrame);
        if (result!=S_OK) {
            qWarning("Failed to convert frame!");
            goto out;
        }

        if (rgbaFrame->GetBytes((void**)&deckLinkBuffer)!= S_OK) {
            qWarning("Failed to get converted buffer!");
            goto out;
        }

        {
            uint8_t *dst;

            vf.map(QVideoFrame::ReadWrite);
            dst=vf.bits(0);
            memcpy(dst, deckLinkBuffer, vf.mappedBytes(0));
            vf.unmap();
        }

        rgbaFrame->Release();
    }
    break;
    case bmdFormat8BitBGRA:
    case bmdFormat8BitARGB: {
        uint8_t *dst;        

        vf.map(QVideoFrame::ReadWrite);
        dst=vf.bits(0);
        memcpy(dst, deckLinkBuffer, vf.mappedBytes(0));
        vf.unmap();
    }
    break;
    default:
        qWarning("Unexpected frame format, only ARGB, BGRA and 8-bit YUV supported");
        break;
    }

    m_framecounter++;

    if (m_videosink) {
        m_videosink->setVideoFrame(vf);
    }

    if (m_grabframe) {
        m_grabframe=false;
        m_frameImage=vf.toImage().rgbSwapped();
        emit frameGrabbed();
    }

out: ;

    frame->Release();    
    if (ap) {
        ap->Release();
    }

    emit frameCountChanged(m_framecounter);
}

bool Decklinksource::enableInput()
{
    BMDDisplayMode amode;
    BOOL supported;

    if (m_input==nullptr) {
        qWarning("No input");
        return false;
    }

    m_framecounter=0;

    if (m_audio) {
        m_input->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, m_achannels);
    }

    if (m_input->DoesSupportVideoMode(bmdVideoConnectionUnspecified, m_mode, bmdFormat8BitBGRA, bmdNoVideoInputConversion, bmdSupportedVideoModeDefault, &amode, &supported)!=S_OK) {
        qWarning("Failed to query supported mode");
        return false;
    }    

    // RGBA is supported for some resolution and fps only
    if (supported) {
        if (m_input->EnableVideoInput(m_mode, bmdFormat8BitBGRA, bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection)!=S_OK) {
            qWarning("Failed to enable input");
            return false;
        }
        qDebug("Using BGRA");
        m_format=bmdFormat8BitBGRA;
    } else {
        if (m_input->EnableVideoInput(m_mode, bmdFormat8BitYUV, bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection)!=S_OK) {
            qWarning("Failed to enable input");
            return false;
        }
        qDebug("Using YUV");
        m_format=bmdFormat8BitYUV;
    }
    if (m_input->StartStreams()!=S_OK) {
        qWarning("Failed to start stream input");
        return false;
    }

    setStreaming(true);    

    return true;
}

bool Decklinksource::disableInput()
{
    if (m_input==nullptr) {
        qWarning("No input");
        return false;
    }

    if (m_input->StopStreams()!=S_OK) {
        qWarning("Failed to stop stream input");
        //return false;
    }

    if (m_audio) {
        m_input->DisableAudioInput();
    }


    if (m_input->DisableVideoInput()!=S_OK) {
        qWarning("Failed to disable input");
        return false;
    }

    setStreaming(false);

    return true;
}

void Decklinksource::setFramebufferSize(QSize size)
{
    m_fbsize=size;
}

QObject *Decklinksource::getDecklink() const
{
    return m_decklink;
}

void Decklinksource::setDecklink(QObject *newDecklink)
{
    if (m_decklink == newDecklink)
        return;

    qDebug() << "Decklink set for source " << this->objectName();

    m_decklink = qobject_cast<DeckLink*>(newDecklink);
    emit decklinkChanged();
}

bool Decklinksource::streaming() const
{
    return m_streaming;
}

void Decklinksource::setStreaming(bool newStreaming)
{
    if (m_streaming == newStreaming)
        return;
    m_streaming = newStreaming;
    emit streamingChanged();
}

void Decklinksource::noInputSource()
{
    if (!m_invalid) {
        m_invalid=true;
        emit invalidSignal();
    }
}

quint32 Decklinksource::frameCount() const
{
    return m_framecounter;
}

QSize Decklinksource::frameSize() const
{
    return m_frameSize;
}

uint Decklinksource::fps() const
{
    return m_fps;
}

bool Decklinksource::audio() const
{
    return m_audio;
}

void Decklinksource::setAudio(bool newAudio)
{
    if (m_audio == newAudio)
        return;

    if (m_streaming) {
        qWarning("Can not change audio setting, streaming in progress");
        return;
    }

    m_audio = newAudio;
    emit audioChanged();
}

int Decklinksource::profile() const
{
    return m_profile;
}
