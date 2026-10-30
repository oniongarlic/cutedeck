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
    footer: secondaryToolbar

    menuBar: MenuBar {
        Menu {
            title: "Profile"
            MenuItem {
                text: "Clear"
                onClicked: dlsink.setProfile(DeckLinkSink.ProfileDefault)
            }
            MenuItem {
                text: "1 1-Full"
                onClicked: dlsink.setProfile(DeckLinkSink.ProfileOneSubDeviceFullDuplex)
            }
            MenuItem {
                text: "2 1-Half"
                onClicked: dlsink.setProfile(DeckLinkSink.ProfileOneSubDeviceHalfDuplex)
            }
            MenuItem {
                text: "3 2-Full"
                onClicked: dlsink.setProfile(3)
            }
            MenuItem {
                text: "4 2-Half"
                onClicked: dlsink.setProfile(4)
            }
            MenuItem {
                text: "5 4-Half"
                onClicked: dlsink.setProfile(5)
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
        videoOutput: vc4
        audioOutput: ao
    }

    AudioOutput {
        id: ao
    }

    CaptureSession {
        id: videoCaptureSession
        camera: camera
        videoOutput: vc3
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

            // Sinks
            dlsink.setOutput(0) // Duo 2 SDI-1 (1+2), SDI 1 & 3
            var d0=dl.device(0)
            console.debug(d0.name)
            outputCombo.model=d0.outputModes
            outputCombo.currentIndex=outputCombo.indexOfValue(dlsink.getMode());
            mp.audioBufferOutput=dlsink.getAudioBuffer()

            dlsink2.setOutput(1) // Duo 2 SDI-2
            var d1=dl.device(0)
            outputCombo2.model=d1.outputModes
            outputCombo2.currentIndex=outputCombo.indexOfValue(dlsink2.getMode());

            // Sources
            dlsrc.setInput(3) // Duo 2 SDI-4
            var d3=dl.device(3)
            console.debug(d3.name)
            inputCombo.model=d3.inputModes
            inputCombo.currentIndex=inputCombo.indexOfValue(dlsrc.getMode());
        }
        Component.onCompleted: {
            console.debug("API Version used", dl.version())
        }        
    }    

    DeckLinkSink {
        id: dlsink
        decklink: dl
        videoSink: vc4.videoSink
    }

    DeckLinkSink {
        id: dlsink2
        decklink: dl
        videoSink: vc3.videoSink
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

    ImageDrawer {
        id: imageDrawer

        onImageSet: {
            switch (dest) {
            case 1:
                dlsink.displayImage(image)
                break;
            case 2:
                dlsink2.displayImage(image)
                break;
            }
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
            ToolButton {
                text: "Camera"
                enabled: !camera.active
                onClicked: {
                    camera.start();
                }
            }
            ComboBox {
                id: outputCombo2
                textRole: "name"
                valueRole: "mode"
                onActivated: {
                    console.debug("Output mode", currentValue)
                    dlsink2.setMode(currentValue)
                }
            }
            ToolButton {
                text: "Enable"
                enabled: !dlsink2.outputEnabled
                onClicked: dlsink2.enableOutput();
            }
            ToolButton {
                text: "Disable"
                enabled: dlsink2.outputEnabled
                onClicked: dlsink2.disableOutput()
            }
        }
    }

    ToolBar {
        id: secondaryToolbar
        RowLayout {
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

            ToolButton {
                text: "K-ON"
                onClicked: {
                    dlsink.setKeyer(true)
                }
            }
            ToolButton {
                text: "K-Off"
                onClicked: {
                    dlsink.setKeyer(false)
                }
            }
            ToolButton {
                text: "KR-Up"
                onClicked: {
                    dlsink.keyerRampUp(60)
                }
            }
            ToolButton {
                text: "KR-Down"
                onClicked: {
                    dlsink.keyerRampDown(60)
                }
            }
            ToolButton {
                text: "PM"
                checkable: true
                checked: dlsink.premultiplied
                onClicked: dlsink.premultiplied=checked
            }
            Slider {
                from: 0
                to: 255
                stepSize: 1
                wheelEnabled: true
                onMoved: {
                    dlsink.keyerLevel(value)
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
                // 3
                VideoOutput {
                    id: vc3
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
                    id: vc4
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
