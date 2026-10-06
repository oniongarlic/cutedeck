#ifndef DECKLINKAUDIODEVICE_H
#define DECKLINKAUDIODEVICE_H

#include <QIODevice>
#include <QObject>

class DeckLinkAudioDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit DeckLinkAudioDevice(QObject *parent = nullptr);

    // QIODevice interface
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
};

#endif // DECKLINKAUDIODEVICE_H
