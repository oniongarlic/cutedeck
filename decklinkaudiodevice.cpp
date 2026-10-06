#include "decklinkaudiodevice.h"

DeckLinkAudioDevice::DeckLinkAudioDevice(QObject *parent)
    : QIODevice{parent}
{}

qint64 DeckLinkAudioDevice::readData(char *data, qint64 maxlen)
{
    return 0;
}

qint64 DeckLinkAudioDevice::writeData(const char *data, qint64 len)
{
    return len;
}
