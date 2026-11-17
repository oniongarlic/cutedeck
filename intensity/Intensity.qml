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
    title: qsTr("CuteDeck Intensity example")

    header: mainToolbar
    footer: secondaryToolbar

    menuBar: MenuBar {
        Menu {
            title: "File"
            MenuItem {
                text: "Quit"
                onClicked: Qt.quit()
            }
        }
        Menu {
            title: "Capture"
            MenuItem {
                id: captureAudio
                text: "Audio"
                checkable: true
                checked: dlsrc.audio
                enabled: !dlsrc.streaming
            }
        }
    }

    MediaPlayer {
        id: mp
        videoOutput: vc1
        audioOutput: ao
    }

    AudioOutput {
        id: ao
    }

    CaptureSession {
        id: videoCaptureSession
        camera: camera
        videoOutput: vc1
    }

    MediaDevices {
        id: mediaDevices
    }

    Camera {
        id: camera
        cameraDevice: mediaDevices.defaultVideoInput
        focusMode: Camera.FocusModeAuto
        onErrorOccurred: console.debug("CameraError: "+errorString)
        onActiveChanged: console.debug("CameraActive:"+active)
    }

    FileDialog {
        id: videoFilesDialog
        nameFilters: [ "*.mp4 *.mov *.mkv *.avi" ]
        title: qsTr("Select video file")
        currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
        onAccepted: {
            mp.source=selectedFile;
        }
    }

    DeckLink {
        id: dl
        onDevicesChanged: {
            console.debug("*** We have decklink devices", devices)
            deviceCombo.model=getDevices();
        }
        onHaveDeckLinkChanged: {
            console.debug("*** We have decklink support")

            // Sink
            setSinkDevice(0)

            // Source
            setSourceDevice(0)
        }
        Component.onCompleted: {
            console.debug("API Version used", dl.version())
        }        
    }

    function setSinkDevice(i) {
        dlsink.setOutput(i)
        var d0=dl.device(i)
        console.debug(d0.name)
        outputCombo.model=d0.outputModes
        outputConnectorCombo.model=d0.outputConnectors
        outputCombo.currentIndex=outputCombo.indexOfValue(dlsink.getMode());
        mp.audioBufferOutput=dlsink.getAudioBuffer()
    }

    function setSourceDevice(i) {
        dlsrc.setInput(i)
        var d1=dl.device(i)
        console.debug(d1.name)
        inputCombo.model=d1.inputModes
        inputConnectorCombo.model=d1.inputConnectors
        inputCombo.currentIndex=inputCombo.indexOfValue(dlsrc.getMode());
    }

    DeckLinkSink {
        id: dlsink
        decklink: dl
        videoSink: vc1.videoSink        
    }

    DeckLinkSource {
        id: dlsrc
        decklink: dl
        videoSink: vc1.videoSink

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
            ComboBox {
                id: deviceCombo
                textRole: "name"
                valueRole: "index"
                onActivated: {
                    console.debug("Device", currentValue)
                    var d=dl.device(currentValue)
                    console.debug(d)
                    console.debug(d.persistentID)
                    var dev=dl.getDeviceProperties(currentValue)
                    var j=JSON.stringify(dev);
                    console.debug(j)
                    debug.text=j;

                    // Sink
                    setSinkDevice(currentValue)

                    // Source
                    setSourceDevice(currentValue)
                }
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
            ComboBox {
                id: inputConnectorCombo
                textRole: "name"
                valueRole: "id"
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
            ComboBox {
                id: outputConnectorCombo
                textRole: "name"
                valueRole: "id"
            }
            ToolButton {
                text: "Enable"
                enabled: !dlsink.outputEnabled
                onClicked: dlsink.enableOutput();
            }
            ToolButton {
                text: "Disable"
                enabled: dlsink.outputEnabled
                onClicked: dlsink.disableOutput()
            }
        }
    }

    ToolBar {
        id: secondaryToolbar
        RowLayout {
            ToolButton {
                text: "Camera"
                enabled: !camera.active
                onClicked: {
                    camera.start();
                }
            }
            ToolSeparator {

            }
            ToolButton {
                text: "Open"
                onClicked: videoFilesDialog.open()
            }
            ToolButton {
                text: "Play"
                onClicked: mp.play()
            }
            ToolButton {
                text: "Stop"
                onClicked: mp.stop()
            }
            ToolSeparator {

            }
            ToolButton {
                text: "Grab"
                enabled: dlsrc.streaming
                onClicked: {
                    dlsrc.grabFrame();
                }
            }
            ToolSeparator {

            }
            ToolButton {
                text: "Image"
                onClicked: imageDrawer.open()
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

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 80
                TextArea {
                    id: debug
                    wrapMode: TextEdit.Wrap
                }
            }

            GridLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                uniformCellHeights: true
                uniformCellWidths: true
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
                    Rectangle {
                        anchors.fill: parent
                        border.color: "blue"
                        border.width: 1
                        color: "transparent"
                    }
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
                    Rectangle {
                        anchors.fill: parent
                        border.color: "blue"
                        border.width: 1
                        color: "transparent"
                    }
                }
            }
        }
    }
}
