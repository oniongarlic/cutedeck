import QtCore

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import CuteDeck

ApplicationWindow {
    id: root
    visible: true
    width: 1024
    height: 768
    title: qsTr("CuteDeck devices example")

    header: mainToolbar

    DeckLink {
        id: dl
        onDevicesChanged: {
            console.debug("*** We have decklink devices", devices)
        }
        onHaveDeckLinkChanged: {
            console.debug("*** We have decklink support")
        }
        Component.onCompleted: {
            console.debug("API Version used", dl.version())
        }
    }

    DeckLinkSink {
        id: dlsink
        decklink: dl
    }

    DeckLinkSource {
        id: dlsrc
        decklink: dl
    }

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Devices: "+dl.devices
            }
        }
    }

    TextArea {
        id: ta
        anchors.fill: parent
    }
}
