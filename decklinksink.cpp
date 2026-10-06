#include "decklinksink.h"
#include "decklinkaudiodevice.h"

#include <QUrl>
#include <QDebug>
#include <QAudioOutput>

Decklinksink::Decklinksink(QObject *parent)
    : QObject{parent}
{
    m_fbsize.setWidth(1920);
    m_fbsize.setHeight(1080);

    m_audiosinkdevice=new DeckLinkAudioDevice(this);
    m_audiosinkdevice->open(QIODevice::WriteOnly);

    QAudioFormat af;
    af.setSampleRate(48000);
    af.setChannelCount(2);
    af.setSampleFormat(QAudioFormat::Int16);
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    m_audio_buffer=new QAudioBufferOutput(af, this);
    connect(m_audio_buffer, &QAudioBufferOutput::audioBufferReceived, this, &Decklinksink::onAudioBufferReceived);
#endif
    
    QAudioOutput a;

    m_audiosink=new QAudioSink(af, this);
    //m_audiosink->start(m_audiosinkdevice);
}

void Decklinksink::setVideoSink(QObject *videosink)
{
    if (videosink==nullptr)
        return;

    if (m_videosink) {
        disconnect(m_videosink, nullptr, this, nullptr);
        m_videosink=nullptr;
    }
    m_videosink = qobject_cast<QVideoSink*>(videosink);

    if (m_videosink) {
        connect(m_videosink, &QVideoSink::videoFrameChanged, this, &Decklinksink::displayFrame);
    } else {
        qWarning("Not a QVideoSink");
    }

    emit videoSinkChanged();
}

QObject *Decklinksink::getVideoSink() const
{
    return m_videosink;
}

bool Decklinksink::setOutput(uint index)
{
    HRESULT result;
    DeckLinkDevice *d;

    if (!m_decklink->haveDeckLink()) {
        qWarning("No decklink device found");
        return false;
    }

    d=m_decklink->getDevice(index);

    if (!d) {
        qWarning("Invalid decklink device");
        return false;
    }

    if (!d->output) {
        qWarning("Device does not support output");
        return false;
    }

    m_current=index;
    m_output=d->output;
    m_keyer=d->key;

    qDebug() << "Decklink output set to " << m_current << d->name;

    result = m_output->CreateVideoFrame(m_fbsize.width(), m_fbsize.height(), m_fbsize.width()*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &m_frame);
    if (result!=S_OK) {
        qWarning("Failed to create video frame");
        return false;
    }

    return true;
}

qint32 Decklinksink::getMode()
{
    return m_mode;
}

bool Decklinksink::setMode(qint32 mode)
{
    m_mode=(BMDDisplayMode)mode;

    return true;
}

bool Decklinksink::setProfile(uint profile)
{
    HRESULT result;
    IDeckLinkProfileManager *manager = NULL;
    IDeckLinkProfile *lp = NULL;
    BMDProfileID profile_id=bmdProfileOneSubDeviceFullDuplex;

    if (!m_decklink->haveDeckLink())
        return false;

    if (m_current<0) {
        qWarning("No device set!");
        return false;
    }

    DeckLinkDevice *d=m_decklink->getDevice(m_current);

    if (d->dev->QueryInterface (IID_IDeckLinkProfileManager, (void **) &manager) != S_OK) {
        qWarning("Current device does not support profiles");
        return false;
    }

    switch (profile) {
    case 0:
    case 1:
        profile_id=bmdProfileOneSubDeviceFullDuplex;
        break;
    case 2:
        profile_id=bmdProfileOneSubDeviceHalfDuplex;
        break;
    case 3:
        profile_id=bmdProfileTwoSubDevicesFullDuplex;
        break;
    case 4:
        profile_id=bmdProfileTwoSubDevicesHalfDuplex;
        break;
    case 5:
        profile_id=bmdProfileFourSubDevicesHalfDuplex;
        break;
    }

    result=manager->GetProfile(profile_id, &lp);

    if (result==S_OK && profile) {
        result=lp->SetActive();
        lp->Release();
    }

    manager->Release();

    qDebug() << "Profile set " << profile_id << (bool)(result==S_OK);

    return result==S_OK;
}

bool Decklinksink::setKeyer(bool enable, uint8_t level, bool external)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    if (enable) {
        qDebug("*** Enable key");
        result=m_keyer->Enable(external);
        m_keyEnabled=result==S_OK ? true : false;
    } else {
        qDebug("*** Disable key");
        result=m_keyer->Disable();
        m_keyEnabled=result==S_OK ? false : true;
    }

    // Make sure a blending level is set, default seems to be a bit random
    keyerLevel(level);

    qDebug() << "Keyer set to: " << m_keyEnabled;

    return result==S_OK;
}

bool Decklinksink::keyerLevel(uint8_t level)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    result=m_keyer->SetLevel(level);

    return result==S_OK;
}

bool Decklinksink::keyerRampUp(uint32_t frames)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    result=m_keyer->RampUp(frames);

    return result==S_OK;
}

bool Decklinksink::keyerRampDown(uint32_t frames)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    result=m_keyer->RampDown(frames);

    return result==S_OK;
}

void Decklinksink::displayFrame(const QVideoFrame &frame)
{
    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;

    QImage img=frame.toImage();

    if (img.isNull())
        return;

    QImage f;

    switch (img.format()) {
    case QImage::Format_ARGB32:
        f=img;
        break;
    case QImage::Format_RGB32:
        f=img.convertToFormat(QImage::Format_ARGB32);
        break;
    case QImage::Format_RGB888:
        f=img.convertToFormat(QImage::Format_ARGB32);
        break;
    case QImage::Format_Invalid:
        return;
    default: {
        f=img.convertToFormat(QImage::Format_ARGB32);
    }
    }

    imageToBuffer(f);

    m_output->DisplayVideoFrameSync(m_frame);
}

void Decklinksink::displayImage(const QImage &frame)
{
    QImage f;
    QImage::Format tf=QImage::Format_ARGB32;

    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)        
        return;

    if (frame.isNull())
        return;

    if (frame.hasAlphaChannel()) {
        tf=m_premultiplied ? QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32;
    }

    if (frame.size()==m_fbsize && frame.format()==tf) {
        f=frame;
    } else if (frame.size()==m_fbsize) {
        f=frame.convertToFormat(tf);
    } else {
        f=frame.scaled(m_fbsize.width(), m_fbsize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation).convertToFormat(tf);
    }

    imageToBuffer(f);

    m_output->DisplayVideoFrameSync(m_frame);
}

void Decklinksink::imageToBuffer(const QImage &frame)
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

void Decklinksink::clearBuffer(int value)
{
    uint8_t* deckLinkBuffer=nullptr;

    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;

    if (m_frame->GetBytes((void**)&deckLinkBuffer) != S_OK) {
        qWarning("Failed to get buffer pointer");
        return;
    }

    memset(deckLinkBuffer, value, m_fbsize.width()*m_fbsize.height()*4);

    m_output->DisplayVideoFrameSync(m_frame);
}

void Decklinksink::onAudioBufferReceived(const QAudioBuffer &buffer)
{
    uint32_t result;

    if (!m_audio)
        return;

    m_output->WriteAudioSamplesSync((void *)buffer.constData<quint16>(), buffer.frameCount(), &result);
}

void Decklinksink::displayImage(const QVariant image)
{
    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;
    
    QImage img;

    switch (image.metaType().id()) {
    case QMetaType::QUrl: {
        QUrl tmp=image.value<QUrl>();

        if (tmp.isLocalFile()) { // File or absolute path
            if (!img.load(tmp.toLocalFile())) {
                qWarning() << "Failed to load image from url" << tmp << tmp.toLocalFile();
            }
        } else {
            qDebug() << "Can load local files only" << tmp.scheme();
            return;
        }
        displayImage(img);
    }
    break;
    case QMetaType::QImage: {
        img=image.value<QImage>();
        displayImage(img.rgbSwapped());
    }
    break;
    case QMetaType::QString: {
        QString tmp=image.value<QString>();
        QUrl utmp(tmp);
        QString file=utmp.toLocalFile();
        
        //if (file.startsWith("file://"))
        //    file.remove(0,7);
        
        if (img.load(file)) {
            displayImage(img);
        } else {
            qWarning() << "Failed to load image from string" << tmp << utmp.path();
        }
    }
    break;
    default:
        qWarning() << "Unhandled image source" << image.metaType();
        return;
    }
}

bool Decklinksink::enableOutput()
{
    HRESULT result;

    if (m_output==nullptr) {
        qWarning("No output");
        m_outputEnabled=false;
        emit outputEnabledChanged();
        return false;
    }

    result=m_output->EnableVideoOutput(m_mode, bmdVideoOutputFlagDefault);
    switch (result) {
    case S_OK:
        if (m_audio) {
            m_output->EnableAudioOutput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, 2, bmdAudioOutputStreamContinuous);
        }
        m_outputEnabled=true;
        emit outputEnabledChanged();
        return true;
        break;
    case E_UNEXPECTED:
        qWarning("Error: Unexpected");
        break;
    case E_OUTOFMEMORY:
        qWarning("Error: Out of memory");
        break;
    case E_ACCESSDENIED:
        qWarning("Error: Access denied");
        break;
    case E_INVALIDARG:
        qWarning("Error: Invalid argument");
        break;
    case E_FAIL:
        qWarning("Error: Failed");
        break;
    default:
        qWarning("Error: Other error");
        break;
    }

    qWarning() << "Failed to enable output, mode " << m_mode << HRESULT_CODE(result);

    m_outputEnabled=false;
    emit outputEnabledChanged();

    return false;
}

bool Decklinksink::disableOutput()
{
    HRESULT result;

    if (m_output==nullptr) {
        qWarning("No output");
        m_outputEnabled=false;
        emit outputEnabledChanged();
        return false;
    }

    result=m_output->DisableVideoOutput();

    if (m_audio) {
        m_output->DisableAudioOutput();
    }

    if (result!=S_OK) {
        qWarning() << "Failed to disable output" << result;
        m_outputEnabled=false;
        emit outputEnabledChanged();
        return false;
    }

    m_outputEnabled=false;
    emit outputEnabledChanged();

    return true;
}

void Decklinksink::setFramebufferSize(QSize size)
{
    m_fbsize=size;
}

QObject *Decklinksink::getDecklink() const
{
    return m_decklink;
}

void Decklinksink::setDecklink(QObject *newDecklink)
{
    if (m_decklink == newDecklink)
        return;

    qDebug() << "Decklink set for sink " << this->objectName();

    m_decklink = qobject_cast<DeckLink*>(newDecklink);
    emit decklinkChanged();
}

bool Decklinksink::keyEnabled() const
{
    return m_keyEnabled;
}

QObject *Decklinksink::getAudioSink() const
{
    return m_audiosink;
}

QObject *Decklinksink::getAudioBuffer() const
{
    return m_audio_buffer;
}

bool Decklinksink::audioEnabled() const
{
    return m_audio;
}

void Decklinksink::setAudioEnabled(bool newAudio)
{
    if (m_audio == newAudio)
        return;
    m_audio = newAudio;
    emit audioEnabledChanged();
}

bool Decklinksink::premultiplied() const
{
    return m_premultiplied;
}

void Decklinksink::setPremultiplied(bool newPremultiplied)
{
    if (m_premultiplied == newPremultiplied)
        return;
    m_premultiplied = newPremultiplied;
    emit premultipliedChanged();
}

bool Decklinksink::outputEnabled() const
{
    return m_outputEnabled;
}
