#include "decklink.h"

#include <QUrl>
#include <QTimer>
#include <QDebug>

#ifdef Q_OS_WIN
#include <objbase.h>

QString btoqs(BSTR name)
{
    QString s = QString::fromWCharArray(name);
    SysFreeString(name);
    return s;
}
#else

#define BOOL bool

#endif

static QVariantMap getConnections(BMDVideoConnection value)
{
    QVariantMap vo;

    vo["hdmi"]=(bool)(bmdVideoConnectionHDMI & value);
    vo["sdi"]=(bool)(bmdVideoConnectionSDI & value);
    vo["opticalsdi"]=(bool)(bmdVideoConnectionOpticalSDI & value);
    vo["component"]=(bool)(bmdVideoConnectionComponent & value);
    vo["composite"]=(bool)(bmdVideoConnectionComposite & value);
    vo["svideo"]=(bool)(bmdVideoConnectionSVideo & value);

    return vo;
}

static bool getFlag(IDeckLinkProfileAttributes* dla, BMDDeckLinkAttributeID id) {
    bool bval=false;

    if (dla->GetFlag(id, &bval) != S_OK) {
        qDebug() << "Failed to get flag for (default to false)." << id;
        bval=false;
    }

    return bval;
}

template<typename IT, typename ID>
static int64_t getInt(IT* dla, ID id)
{
    int64_t ival;

    if (dla->GetInt(id, &ival) != S_OK) {
        qDebug() << "Failed to get int value for (default to 0)." << id;
        ival=0;
    }

    return ival;
}


class DeckLinkNotificationCallback: public IDeckLinkNotificationCallback
{
public:

    DeckLinkNotificationCallback(DeckLink *parent, DeckLinkDevice *device, IDeckLinkStatus *status) :
        m_parent(parent),
        m_device(device),
        m_status(status)
    {
        m_status->AddRef();
    }

    ~DeckLinkNotificationCallback()
    {
        m_status->Release();
    }

    HRESULT Notify(BMDNotifications topic, uint64_t param1, uint64_t param2) override
    {
        switch (topic) {
        case bmdPreferencesChanged:
            qDebug() << "bmdPreferencesChanged"  << param1 << param2;
            break;
        case bmdStatusChanged:
            qDebug() << "bmdStatusChanged"  << param1 << param2;
            BMDDeckLinkStatusID id = (BMDDeckLinkStatusID)param1;
            handleStatusChanged(id, param2);
            break;
        }

        return S_OK;
    }

    void handlePreferencesChanged(uint64_t id, uint64_t param2)
    {
    }

    void handleStatusChanged(BMDDeckLinkStatusID id, uint64_t param2)
    {
        HRESULT result;
        int64_t ival;
        bool bval;

        if (!m_status)
            return;

        switch (id) {
        case bmdDeckLinkStatusBusy:
            result = m_status->GetInt(id, &ival);
            qDebug() << "Busy" << ival;
        break;
        case bmdDeckLinkStatusCurrentVideoInputMode:
            result = m_status->GetInt(id, &ival);
            qDebug() << "Input mode" << ival;
            break;
        case bmdDeckLinkStatusCurrentVideoInputPixelFormat:
            result = m_status->GetInt(id, &ival);
            qDebug() << "Format" << ival;
            break;
        case bmdDeckLinkStatusDetectedVideoInputFormatFlags:
            result = m_status->GetInt(id, &ival);
            qDebug() << "Format flags" << ival;
        break;
        case bmdDeckLinkStatusVideoInputSignalLocked:
            result = m_status->GetFlag(id, &bval);
            qDebug() << "Locked" << ival;
        break;
        }
    }

    HRESULT QueryInterface(REFIID iid, LPVOID *ppv) override
    {
        return E_NOINTERFACE;
    }

    ULONG AddRef() override
    {
        return 1;
    }

    ULONG Release() override
    {
        return 1;
    }

private:
    DeckLink *m_parent=nullptr;
    DeckLinkDevice *m_device=nullptr;
    IDeckLinkStatus* m_status=nullptr;
};


DeckLink::DeckLink(QObject *parent)
    : QObject{parent}
{
    IDeckLinkIterator* deckLinkIterator	= nullptr;
    IDeckLink* deckLink=nullptr;
    HRESULT result = S_OK;
    
#ifdef Q_OS_WIN
    result = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&deckLinkIterator);
#else
    deckLinkIterator=CreateDeckLinkIteratorInstance();
#endif
    if (deckLinkIterator!=nullptr || !FAILED(result)) {
        qDebug() << "Found DeckLink support";
        m_haveDeckLink=true;
    } else {
        qDebug() << "No DeckLink support installed";
        return;
    }    

    /* Look for devices */
    while ((deckLinkIterator->Next(&deckLink))==S_OK) {
        const char *model, *name;
        IDeckLinkProfileAttributes* deckLinkAttributes = NULL;
        int64_t value;

        DeckLinkDevice *dld=new DeckLinkDevice;
        IDeckLinkInput *input=nullptr;
        IDeckLinkOutput *output=nullptr;
#ifdef Q_OS_WIN
        BSTR bm, bn;
        deckLink->GetModelName(&bm);
        deckLink->GetDisplayName(&bn);
        
        dld->model=btoqs(bm);
        dld->name=btoqs(bn);
#else
        deckLink->GetModelName(&model);
        deckLink->GetDisplayName(&name);
        dld->name=name;
        dld->model=model;
#endif

        dld->dev=deckLink;
        dld->valid=true;

        dld->input=nullptr;
        dld->output=nullptr;
        dld->key=nullptr;

        QVariantMap dev;
        dev["modelName"]=QVariant(model);
        dev["displayName"]=QVariant(name);

        qDebug() << "*** Device: " << dld->name << dld->model;

        result = deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&deckLinkAttributes);
        if (result != S_OK) {
            qWarning("Failed to get decklink device attributes");
            continue;
        }

        result = deckLinkAttributes->GetInt(BMDDeckLinkDeviceInterface, &value);
        if (result != S_OK) {
            qWarning("Failed to get decklink device interface attribute");
            goto out;
        }

        switch(value) {
        case bmdDeviceInterfacePCI:
            dev["interface"]="PCI";
            break;
        case bmdDeviceInterfaceUSB:
            dev["interface"]="USB";
            break;
        case bmdDeviceInterfaceThunderbolt:
            dev["interface"]="Thunderbolt";
            break;
        default:
            qDebug() << "Unknown interface?";
        }

        dld->persistentID=getInt(deckLinkAttributes, BMDDeckLinkPersistentID);
        dld->topologicalID=getInt(deckLinkAttributes, BMDDeckLinkTopologicalID);

        if (deckLinkAttributes->GetInt(BMDDeckLinkVideoIOSupport, &value) != S_OK) {
            qDebug("Failed to get video IO support, skipping");
            goto out;
        }

        // If device profile is part of a pair (A,B) then the secondary will be inactive, mark it such
        dld->duplex=getInt(deckLinkAttributes, BMDDeckLinkDuplex);
        if (dld->duplex==bmdDuplexInactive) {
            dld->active=false;
        } else {
            dld->active=true;
        }

        dld->audioChannels=getInt(deckLinkAttributes, BMDDeckLinkMaximumAudioChannels);

        dev["serialport"]=getFlag(deckLinkAttributes, BMDDeckLinkHasSerialPort);
        dev["monitorOut"]=getFlag(deckLinkAttributes, BMDDeckLinkHasMonitorOut);

        dev["LTC"]=getFlag(deckLinkAttributes, BMDDeckLinkSupportsHDMITimecode);

        dev["HANCfilter"]=getFlag(deckLinkAttributes, BMDDeckLinkHANCRequiresInputFilterConfiguration);
        dev["HANCoutput"]=getFlag(deckLinkAttributes, BMDDeckLinkSupportsHANCOutput);
        dev["HANCinput"]=getFlag(deckLinkAttributes, BMDDeckLinkSupportsHANCInput);

        dev["subdevices"]=(qint64)getInt(deckLinkAttributes, BMDDeckLinkNumberOfSubDevices);
        dev["subindex"]=(qint64)getInt(deckLinkAttributes, BMDDeckLinkSubDeviceIndex);

        dev["outputs"]=getConnections((BMDVideoConnection)getInt(deckLinkAttributes, BMDDeckLinkVideoOutputConnections));
        dev["inputs"]=getConnections((BMDVideoConnection)getInt(deckLinkAttributes, BMDDeckLinkVideoInputConnections));

        // Collect output details
        if ((value & bmdDeviceSupportsPlayback) != 0) {
            IDeckLinkDisplayModeIterator *dmi;
            IDeckLinkDisplayMode *dm;
            IDeckLinkKeyer *k;
            QVariantList modes;

            result = deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&output);
            if (result != S_OK) {
                qWarning("Failed to get output interface");
            } else {
                output->GetDisplayModeIterator(&dmi);
                while ((dmi->Next(&dm))==S_OK) {
                    const char *mname;
                    qint64 w,h;
                    uint m;

                    QVariantMap mode;

#ifdef Q_OS_WIN
                    BSTR bm;
                    dm->GetName(&bm);
                    mode["name"]=btoqs(bm);
#else
                    dm->GetName(&mname);
                    mode["name"]=QVariant(mname);
#endif
                    h=dm->GetHeight();
                    w=dm->GetWidth();
                    m=dm->GetDisplayMode();

                    mode["width"]=w;
                    mode["height"]=h;
                    mode["mode"]=m;
                    modes.append(mode);
                }
                dmi->Release();

                result = deckLink->QueryInterface (IID_IDeckLinkKeyer, (void **) &k);
                if (result != S_OK) {
                    qDebug("No keyer for output");
                    dev["keyer"]=false;
                    dld->key=nullptr;
                } else {
                    BOOL ki, ke;
                    qDebug("Keyer supported");
                    dev["keyer"]=true;
                    dld->key=k;

                    dev["keyerInternal"]=getFlag(deckLinkAttributes, BMDDeckLinkSupportsInternalKeying);
                    dev["keyerExternal"]=getFlag(deckLinkAttributes, BMDDeckLinkSupportsExternalKeying);

                    qDebug() << "Int, Ext" << ki << ke;
                }

                IDeckLinkProfileManager *manager = NULL;
                if (deckLink->QueryInterface (IID_IDeckLinkProfileManager, (void **) &manager) == S_OK) {
                    qDebug("Has duplex/profiles mode");
                    dev["profiles"]=true;

                    int64_t current;
                    result=deckLinkAttributes->GetInt(BMDDeckLinkProfileID, &current);

                    qDebug() << "Current profile ID" << current;

                    dev["profile"]=QVariant((qint64)current);

                    IDeckLinkProfileIterator *pi;
                    IDeckLinkProfile *p;
                    manager->GetProfiles(&pi);

                    while ((pi->Next(&p))==S_OK) {
                        BOOL pactive;
                        IDeckLinkProfileAttributes*	pa;
                        int64_t pid=0;

                        result=p->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&pa);
                        if (result==S_OK) {
                            result=pa->GetInt(BMDDeckLinkProfileID, &pid);
                        }
                        p->IsActive(&pactive);
                        qDebug() << "*** Profile: " << pid << pactive;
                    }

#if 0
                IDeckLinkProfile *profile = NULL;
                BMDProfileID bmd_profile_id = 0;
                result = manager->GetProfile(bmd_profile_id, &profile);

                if (result==S_OK && profile) {
                    result=profile->SetActive();
                    profile->Release();
                }
#endif

                    manager->Release();
                } else {
                    dev["profiles"]=false;
                }

                dev["playback"]=true;
                dld->outputModes=modes;

                dld->output=output;
            }
        }

        // Collect input details
        if ((value & bmdDeviceSupportsCapture) != 0) {
            QVariantList modes;

            result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&input);
            if (result != S_OK) {
                qWarning("Failed to get input interface");
            } else {
                IDeckLinkDisplayModeIterator *dmi;
                IDeckLinkDisplayMode *dm;
                dld->input=input;

                output->GetDisplayModeIterator(&dmi);
                while ((dmi->Next(&dm))==S_OK) {
                    const char *mname;
                    qint64 w,h;
                    uint m;
                    BMDTimeValue tv;
                    BMDTimeScale ts;

                    QVariantMap mode;
                    
#ifdef Q_OS_WIN
                    BSTR bm;
                    dm->GetName(&bm);
                    mode["name"]=btoqs(bm);
#else
                    dm->GetName(&mname);
                    mode["name"]=QVariant(mname);
#endif
                    h=dm->GetHeight();
                    w=dm->GetWidth();
                    m=dm->GetDisplayMode();
                    dm->GetFrameRate(&tv, &ts);

                    mode["width"]=w;
                    mode["height"]=h;
                    mode["mode"]=m;
                    modes.append(mode);
                }
                dmi->Release();

                dev["record"]=true;
                dld->inputModes=modes;
            }
        }

        // Status / Notifications
        IDeckLinkStatus *status;
        result = deckLink->QueryInterface(IID_IDeckLinkStatus, (void**)&status);
        if (result == S_OK) {
            IDeckLinkNotification *notification;
            result = deckLink->QueryInterface(IID_IDeckLinkNotification, (void**)&notification);
            if (result == S_OK) {
                DeckLinkNotificationCallback *ncb=new DeckLinkNotificationCallback(this, dld, status);
                notification->Subscribe(bmdStatusChanged, ncb);
                dld->notification=notification;
            }
        }

        IDeckLinkStatistics *statistics;
        result = deckLink->QueryInterface(IID_IDeckLinkStatistics, (void**)&statistics);
        if (result == S_OK) {
            auto temp=getInt<IDeckLinkStatistics, BMDDeckLinkAttributeID >(statistics, bmdDeckLinkStatisticDeviceTemperature);
            qDebug() << temp;
            dld->statistics=statistics;
        }

        m_devices++;
        dld->properties=dev;
        m_devs.append(dld);

    out:;

        deckLinkAttributes->Release();        
    }

    qDebug() << "Found devices" << m_devices;

    QTimer::singleShot(0, this, [&]{
        emit haveDeckLinkChanged();
        emit devicesChanged();
    });
}

DeckLink::~DeckLink()
{
    for (const DeckLinkDevice *item : std::as_const(m_devs)) {
        if (item->input)
            item->input->Release();
        if (item->output)
            item->output->Release();
        item->dev->Release();
    }
}

bool DeckLink::haveDeckLink() const
{
    return m_haveDeckLink;
}

int DeckLink::devices() const
{
    return m_devices;
}

QVariantList DeckLink::getDevices(bool includeInactive)
{
    QVariantList devs;
    int i=0;

    for (const DeckLinkDevice *item : std::as_const(m_devs)) {
        QVariantMap dev;

        if (!item->active && !includeInactive)
            continue;

        dev["index"]=i;
        dev["name"]=item->name;
        dev["model"]=item->name;
        devs.append(dev);
        i++;
    }

    return devs;
}

QVariantMap DeckLink::getDeviceProperties(int index)
{
    if (index>m_devs.count()-1 || index<0) {
        qWarning("Requested device out of range");
        return QVariantMap();
    }
    auto d=m_devs.at(index);

    return d->properties;
}

DeckLinkDevice DeckLink::device(int index)
{
    DeckLinkDevice n;

    if (index>m_devs.count()-1 || index<0) {
        qWarning("Requested device out of range");
        n.active=false;
        return n;
    }

    return *m_devs.at(index);
}

DeckLinkDevice *DeckLink::getDevice(int index)
{    
    if (index>m_devs.count()-1 || index<0) {
        qWarning("Requested device out of range");
        return nullptr;
    }

    return m_devs.at(index);
}
