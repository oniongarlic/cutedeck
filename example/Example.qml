import QtCore

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import QtMultimedia

import CuteDeck

ApplicationWindow {
    id: root
    visible: true
    width: 1024
    height: 768
    title: qsTr("CuteDeck devices example")

    header: mainToolbar

    MediaPlayer {
        id: mp
        videoOutput: vc1
        audioOutput: ao
    }

    AudioOutput {
        id: ao
    }

    DeckLink {
        id: dl
        onDevicesChanged: {
            console.debug("*** We have decklink devices", devices)
        }
        onHaveDeckLinkChanged: {
            console.debug("*** We have decklink support")

            // Sinks
            dlsink.setOutput(0) // Duo 2 SDI-1 (1+2)
            var d0=dl.getDeviceProperties(0)
            outputCombo.model=d0["outputModes"]
            outputCombo.currentIndex=outputCombo.indexOfValue(dlsink.getMode());
            mp.audioBufferOutput=dlsink.getAudioBuffer()

            // Sources
            dlsrc.setInput(3)
            var d3=dl.getDeviceProperties(3)
            inputCombo.model=d3["inputModes"]
            inputCombo.currentIndex=inputCombo.indexOfValue(dlsrc.getMode());
        }
        Component.onCompleted: {
            console.debug("API Version used", dl.version())
        }
        Component.onCompleted: {
            console.debug("API Version used", dl.version())
        }
    }

    DeckLinkSink {
        id: dlsink
        decklink: dl
        videoSink: vc1.videoSink
    }

    DeckLinkSource {
        id: dlsrc
        decklink: dl
        videoSink: vc2.videoSink

        onInvalidSignal: {
            console.debug("*** Invalid input signal")
        }
        onValidSignal: {
            console.debug("*** Got valid input signal")
        }
    }

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Devices: "+dl.devices
            }
            Label {
                text: "Input"
            }
            Label {
                text: dlsrc.frameCount
            }
            ComboBox {
                id: inputCombo
                textRole: "name"
                valueRole: "mode"
                onActivated: {
                    console.debug("Input mode", currentValue)
                    dlsrc.setMode(currentValue)
                }
            }
            ToolButton {
                text: "Enable"
                enabled: !dlsrc.streaming
                onClicked: dlsrc.enableInput();
            }
            ToolButton {
                text: "Disable"
                enabled: dlsrc.streaming
                onClicked: dlsrc.disableInput()
            }

            ToolButton {
                text: "Grab"
                enabled: dlsrc.streaming
                onClicked: {
                    dlsrc.grabFrame();
                }
            }
            Label {
                text: "Output"
            }
            ComboBox {
                id: outputCombo
                textRole: "name"
                valueRole: "mode"
                onActivated: {
                    console.debug("Output mode", currentValue)
                    dlsink.setMode(currentValue)
                }
            }
        }
    }

    StackView {
        id: rootStack
        initialItem: mainView
        anchors.fill: parent
    }

    Page {
        id: mainView
        anchors.fill: parent

        ColumnLayout {
            id: mainRow
            anchors.fill: parent
            anchors.margins: 4
            spacing: 4
            // uniformCellSizes: true

            GridLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                rows: 2
                columns: 2
                rowSpacing: 4
                columnSpacing: 4

                // 1
                VideoOutput {
                    id: vc1
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumHeight: 256
                    Layout.preferredWidth: mainRow.width/2
                    fillMode: Image.PreserveAspectFit
                }
                // 2
                VideoOutput {
                    id: vc2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumHeight: 256
                    Layout.preferredWidth: mainRow.width/2
                    fillMode: Image.PreserveAspectFit
                }
            }
        }


    }
}
