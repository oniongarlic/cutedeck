import QtCore

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import QtMultimedia

import Qt.labs.folderlistmodel

import CuteDeck

ApplicationWindow {
    id: root
    visible: true
    width: 1024
    height: 768
    title: qsTr("CuteDeck Image Viewer")

    header: mainToolbar
    footer: secondaryToolbar

    readonly property int imgSize: imgThumbSizeSlider.value+5

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
    }

    MediaPlayer {
        id: mp
        // videoOutput: vc4
        audioOutput: ao
    }

    AudioOutput {
        id: ao
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

    function setSinkDevice(i) {
        dlsink.setOutput(i)
        var d0=dl.device(i)
        console.debug(d0.name)
        outputCombo.model=d0.outputModes
        outputCombo.currentIndex=outputCombo.indexOfValue(dlsink.getMode());
    }

    DeckLink {
        id: dl
        onDevicesChanged: {
            console.debug("*** We have decklink devices", devices)
            deviceCombo.model=getDevices();

            // Sinks
            setSinkDevice(0)
            mp.audioBufferOutput=dlsink.getAudioBuffer()
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
        //videoSink: vc1.videoSink
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
                    setSinkDevice(currentValue)
                    var d=dl.device(currentValue)
                    console.debug(d)
                    console.debug(d.persistentID)
                    var dev=dl.getDeviceProperties(currentValue)
                    var j=JSON.stringify(dev);
                    console.debug(j)
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
                text: "Image"
                onClicked: imageDrawer.open()
            }
            ToolButton {
                text: "PM"
                checkable: true
                checked: dlsink.premultiplied
                onClicked: dlsink.premultiplied=checked
            }
            ToolSeparator {

            }
            ToolButton {
                text: "Key ON"
                onClicked: {
                    dlsink.setKeyer(true)
                }
            }
            ToolButton {
                text: "Key Off"
                onClicked: {
                    dlsink.setKeyer(false)
                }
            }
            ToolButton {
                text: "KeyUp"
                onClicked: {
                    dlsink.keyerRampUp(60)
                }
            }
            ToolButton {
                text: "KeyDown"
                onClicked: {
                    dlsink.keyerRampDown(60)
                }
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
            ToolSeparator {

            }
            Slider {
                id: imgThumbSizeSlider
                from: 1
                to: 10
                value: 4
                stepSize: 1
                wheelEnabled: true
            }
        }
    }

    FolderListModel {
        id: fileModel
        folder: folderModel.folder
        showDotAndDotDot: true
        showDirsFirst: true
        showDirs: false
        nameFilters: ["*.jpg", "*.png", "*.jpeg"]
    }

    FolderListModel {
        id: folderModel
        folder: "file://"+imagePath
        showDotAndDotDot: true
        showDirsFirst: true
        showFiles: false
        showDirs: true
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

            VideoOutput {
                id: vc1
                visible: false
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

            RowLayout {
                id: imageRow
                Layout.fillHeight: false
                Layout.fillWidth: true
                Layout.minimumHeight: 200
                Layout.maximumHeight: 300
                Rectangle {
                    width: (root.width/5)*2
                    height: Math.min(root.height/3, 300)
                    border.color: "green"
                    border.width: 2
                    color: "transparent"
                    Image {
                        id: imagePreview
                        anchors.fill: parent
                        anchors.margins: 2
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: 1920/2
                        sourceSize.height: 1080/2
                        asynchronous: true
                    }
                }

                ColumnLayout {
                    Layout.minimumWidth: 60
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 16
                    Button {
                        text: "Set live"
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: {
                            imageOutput.source=imagePreview.source
                            dlsink.displayImage(imageOutput.source)
                        }
                    }
                    Button {
                        text: "Clear black"
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: {
                            dlsink.clearBuffer(0)
                        }
                    }
                    Button {
                        text: "Clear white"
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: {
                            dlsink.clearBuffer(255)
                        }
                    }
                    Label {
                        text: "Fade"
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: 0
                        to: 255
                        onValueChanged: dlsink.clearBuffer(value)
                    }
                }
                Rectangle {
                    width: (root.width/5)*2
                    height: Math.min(root.height/3, 300)
                    border.color: "red"
                    border.width: 2
                    color: "transparent"
                    Image {
                        id: imageOutput
                        anchors.fill: parent
                        anchors.margins: 2
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: 1920/2
                        sourceSize.height: 1080/2
                        asynchronous: true
                    }
                }
            }

            RowLayout {
                id: fileRow
                Layout.minimumHeight: 300
                Layout.fillWidth: true
                ListView {
                    id: folderList
                    model: folderModel
                    delegate: folderDelegate

                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.minimumWidth: 200
                    Layout.maximumWidth: 460
                    clip: true

                    highlight: Rectangle { color: "lightsteelblue"; radius: 2 }

                    function processItem(index) {
                        folderList.currentIndex=index
                        var f=folderModel.get(currentIndex, "fileUrl")
                        if (folderModel.isFolder(currentIndex))
                            folderModel.folder=f;
                    }

                    Component {
                        id: folderDelegate
                        ItemDelegate {
                            width: ListView.view.width
                            text: fileName

                            onClicked: {
                                onClicked: folderList.processItem(index);
                            }
                        }
                    }
                }
                GridView {
                    id: fileList
                    model: fileModel
                    delegate: fileDelegate
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: 200
                    cellWidth: 1920/imgSize
                    cellHeight: 1080/imgSize
                    clip: true
                    focus: true

                    highlight: Rectangle { color: "lightsteelblue"; radius: 2 }

                    function processItem(index) {
                        fileList.currentIndex=index
                        var f=fileModel.get(currentIndex, "fileUrl")
                        imagePreview.source=f;
                    }

                    ScrollBar.vertical: ScrollBar {

                    }

                    Component {
                        id: fileDelegate
                        ItemDelegate {
                            id: fid
                            width: fileList.cellWidth
                            height: fileList.cellHeight

                            onClicked: {
                                onClicked: fileList.processItem(index);
                            }
                            Image {
                                anchors.fill: parent
                                anchors.margins: 4
                                fillMode: Image.PreserveAspectFit
                                source: fileUrl
                                sourceSize.width: 192*2
                                sourceSize.height: 108*2
                                asynchronous: true
                            }
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 2
                                border.color: "blue"
                                border.width: fid.highlighted ? 3 : 1
                                color: "transparent"
                                radius: 2
                            }
                        }
                    }
                }
            }
        }
    }
}
