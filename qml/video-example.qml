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
    title: qsTr("CuteDeck example")
    header: mainToolbar

    property bool inProgress: false

    menuBar: MenuBar {
        Menu {
            title: "Profile"
            MenuItem {
                text: "Clear"
                onClicked: dls.setProfile(0)
            }
            MenuItem {
                text: "1 1-Full"
                onClicked: dls.setProfile(1)
            }
            MenuItem {
                text: "2 1-Half"
                onClicked: dls.setProfile(2)
            }
            MenuItem {
                text: "3 2-Full"
                onClicked: dls.setProfile(3)
            }
            MenuItem {
                text: "4 2-Half"
                onClicked: dls.setProfile(4)
            }
            MenuItem {
                text: "5 4-Half"
                onClicked: dls.setProfile(5)
            }
        }
        Menu {
            title: "Capture"
            MenuItem {
                id: captureAudio
                text: "Audio"
                checkable: true
                checked: dlsrc.audio
            }
        }
    }

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Input"
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

            Label {
                text: "Output"
            }
            ComboBox {
                id: outputCombo
                textRole: "name"
                valueRole: "mode"
                onActivated: {
                    console.debug("Output mode", currentValue)
                    dls.setMode(currentValue)
                }
            }

            CheckBox {
                id: sdi1
                text: "1"
            }
            CheckBox {
                id: sdi2
                text: "2"
            }
            CheckBox {
                id: sdi3
                text: "3"
            }
            CheckBox {
                id: sdi4
                text: "4"
            }

            Slider {
                from: 0
                to: 255
                stepSize: 1
                wheelEnabled: true
                onMoved: {
                    dls.keyerLevel(value)
                }
            }

            Text {
                id: cnameText
                text: "n/a"
                Layout.preferredWidth: 100
            }
            Text {
                id: cgroupText
                text: "n/a"
                Layout.preferredWidth: 100
            }
            Text {
                text: dlsrc.frameCount
            }
            Rectangle {
                id: crect
                Layout.fillHeight: true
                width: 40;
            }
            ToolSeparator {

            }
            ToolButton {
                text: "F"
                onClicked: videoFilesDialog.open()
            }
            ToolButton {
                text: "P"
                onClicked: mediaPlayer.play()
            }
            ToolButton {
                text: "S"
                onClicked: mediaPlayer.stop()
            }
        }
    }

    function previewImageFile(file) {
        imagePreview.source=file
        imagePreview.visible=true;
    }

    function processImageFile(file) {
        previewImage.source=file
        previewImage.visible=true;
        // cd.processImageFile(file);
        od.processImageFile(file);
        //imp.setImage(file);

        odface.processImageFile(file)

        var it=od.getImage();
        console.debug(it)

        imp.setImage(it);

        //dls.displayImage(it)

        if (sdi1.checked)
            dls.displayImage(file)

        if (sdi2.checked)
            dls2.displayImage(file)
    }

    function imageToDecklink(sink, file) {        
        sink.displayImage(file)
    }

    MediaPlayer {
        id: mediaPlayer
        videoOutput: vc2
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
            mediaPlayer.source=selectedFile;
        }
    }

    ImageGallerySelector {
        id: filesDialog
        onFileSelected: {
            camera.stop();
            processImageFile(src)
        }
    }

    ListModel {
        id: detectedItems
    }

    MediaDevices {
        id: mediaDevices
    }

    CaptureSession {
        id: videoCaptureSession
        camera: camera
        videoOutput: vc
    }

    Camera {
        id: camera
        cameraDevice: mediaDevices.defaultVideoInput
        focusMode: Camera.FocusModeAuto
        onErrorOccurred: console.debug("CameraError: "+errorString)
        onActiveChanged: console.debug("CameraActive:"+active)
        Component.onCompleted: {

        }
    }

    DeckLink {
        id: dl
        onDevicesChanged: {
            console.debug("*** We have decklink devices", devices)
        }
        onHaveDeckLinkChanged: {
            console.debug("*** We have decklink support")

            // Sinks
            dls.setOutput(0) // Duo 2 SDI-1 (1+2)
            var d0=dl.getDeviceProperties(0)
            outputCombo.model=d0["outputModes"]
            outputCombo.currentIndex=outputCombo.indexOfValue(dls.getMode());

            dls2.setOutput(1) // Duo 2 SDI-3
            dls2.setMode(DeckLinkSource.VideoHD1080p30)
            mediaPlayer.audioBufferOutput=dls2.getAudioBuffer()

            // Sources
            dlsrc.setInput(3)
            var d3=dl.getDeviceProperties(3)
            inputCombo.model=d3["inputModes"]
            inputCombo.currentIndex=inputCombo.indexOfValue(dlsrc.getMode());
        }
    }

    DeckLinkSink {
        id: dls
        decklink: dl
        objectName: "Sink 1"
        //videoSink: vc.videoSink
    }
    DeckLinkSink {
        id: dls2
        decklink: dl
        objectName: "Sink 2"
        videoSink: vc2.videoSink
    }

    DeckLinkSource {
        id: dlsrc
        decklink: dl
        objectName: "Source 4"
        audio: captureAudio.checked
        videoSink: vc4.videoSink
        onInvalidSignal: {
            console.debug("*** Invalid input signal")
        }
        onValidSignal: {
            console.debug("*** Got valid input signal")
        }
        onFrameGrabbed: {
            imp.setImage(getImage());
            // just some random string to get it set
            previewImage.source="image://cute/frame_"+frameCount
        }
    }

    FolderListModel {
        id: fileModel
        folder: "file://"+imagePath
        showDotAndDotDot: true
        showDirsFirst: true
        nameFilters: ["*.jpg", "*.png", "*.jpeg"]
    }

    StackView {
        id: rootStack
        initialItem: mainView
        anchors.fill: parent
    }

    Page {
        id: mainView
        anchors.fill: parent

        footer: ToolBar {
            RowLayout {
                anchors.fill: parent
                ToolButton {
                    text: "Load Image"
                    enabled: !inProgress
                    onClicked: {
                        filesDialog.startSelector();
                    }
                }
                ToolButton {
                    text: "Grab"
                    enabled: dlsrc.streaming
                    onClicked: {
                        dlsrc.grabFrame();
                    }
                }
                ToolButton {
                    text: "Capture"
                    enabled: !inProgress && camera.active
                    onClicked: {
                        videoInput.imageCapture.capture();
                    }
                }
                ToolButton {
                    text: "Camera"
                    enabled: !camera.active
                    onClicked: {
                        camera.start();
                    }
                }
                ToolButton {
                    text: "Edit"
                    onClicked: {
                        imp.setImage(previewImage.source);
                        if (!imp.isEmpty()) {
                            rootStack.push(imageEditor);
                        } else {
                            messageDialog.show("Operation failed", "Failed to set image for editing")
                        }
                    }
                }

                ToolSeparator {

                }

                ToolButton {
                    text: "E2"
                    onClicked: {
                        dls2.enableOutput();
                    }
                }
                ToolButton {
                    text: "D2"
                    onClicked: {
                        dls2.disableOutput();
                    }
                }
                ToolButton {
                    text: "DI"
                    onClicked: {
                        dls2.displayImage()
                    }
                }                

                ToolSeparator {

                }

                ToolButton {
                    text: "E1"
                    onClicked: {
                        dls.enableOutput();
                    }
                }
                ToolButton {
                    text: "D1"
                    onClicked: {
                        dls.disableOutput();
                    }
                }
                ToolButton {
                    text: "Clr"
                    onClicked: {
                        dls.clearBuffer();
                    }
                }

                ToolButton {
                    text: "PM"
                    checkable: true
                    checked: dls.premultiplied
                    onClicked: dls.premultiplied=checked
                }

                ToolButton {
                    text: "K-ON"
                    onClicked: {
                        dls.setKeyer(true)
                    }
                }
                ToolButton {
                    text: "K-Off"
                    onClicked: {
                        dls.setKeyer(false)
                    }
                }
                ToolButton {
                    text: "KR-Up"
                    onClicked: {
                        dls.keyerRampUp(60)
                    }
                }
                ToolButton {
                    text: "KR-Down"
                    onClicked: {
                        dls.keyerRampDown(60)
                    }
                }
                ToolSeparator {

                }

                ToolButton {
                    text: "I-E"
                    enabled: !dlsrc.streaming
                    onClicked: {
                        dlsrc.enableInput()
                    }
                }
                ToolButton {
                    text: "I-D"
                    enabled: dlsrc.streaming
                    onClicked: {
                        dlsrc.disableInput()
                    }
                }
            }
        }

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
                    id: vc
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumHeight: 256
                    Layout.preferredWidth: mainRow.width/2
                    fillMode: Image.PreserveAspectFit

                    MouseArea {
                        anchors.fill: parent
                        onPressAndHold: {
                            console.debug(mouse.x)
                            console.debug(mouse.y)

                            console.debug(mouse.x/width)
                            console.debug(mouse.y/height)

                            // cd.setRoi();
                        }
                        onClicked: {
                            camera.searchAndLock();
                        }
                        onDoubleClicked: {
                            camera.imageCapture.capture();
                        }
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 33
                        height: 33
                        border.width: 4
                        border.color: "green"
                        color: "transparent"
                    }

                    Image {
                        id: previewImage
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit                        
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        width: 128
                        height: 128
                        visible: inProgress
                        running: inProgress
                    }
                    Rectangle {
                        anchors.fill: parent
                        border.color: "red"
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
                        border.color: "green"
                        border.width: 1
                        color: "transparent"
                    }
                }

                // 4
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
                        border.color: "orange"
                        border.width: 1
                        color: "transparent"
                    }
                }
            }

            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: root.height/5
                Layout.maximumHeight: root.height/3

                Rectangle {
                    id: pcid
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumWidth: 128

                    Image {
                        id: imagePreview
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                    }


                    RowLayout {
                        anchors.bottom: pcid.bottom
                        anchors.margins: 4
                        Button {
                            text: "1"
                            onClicked: imageToDecklink(dls, imagePreview.source);
                        }
                        Button {
                            text: "2"
                            onClicked: imageToDecklink(dls2, imagePreview.source);
                        }
                    }

                }

                ListView {
                    id: fileList
                    model: fileModel
                    delegate: fileDelegate

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumWidth: 200
                    Layout.maximumWidth: 460
                    clip: true

                    highlight: Rectangle { color: "lightsteelblue"; radius: 2 }

                    function processItem(index) {
                        fileList.currentIndex=index
                        var f=fileModel.get(currentIndex, "fileURL")
                        if (fileModel.isFolder(currentIndex))
                            fileModel.folder=f;
                        else
                            previewImageFile(f);
                    }

                    function processFile(index) {
                        var f=fileModel.get(currentIndex, "fileURL")
                        processImageFile(f);
                    }

                    Component {
                        id: fileDelegate
                        ItemDelegate {
                            width: ListView.view.width
                            height: r.height

                            onClicked: {
                                onClicked: fileList.processItem(index);
                            }

                            RowLayout {
                                id: r
                                width: parent.width
                                height: 20
                                spacing: 4
                                RowLayout{
                                    id: rl
                                    spacing: 8
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: parent.width/4
                                    Layout.maximumWidth: parent.width/3
                                    Text { text: fileName }
                                }
                                Button {
                                    text: "1"
                                    Layout.fillWidth: false
                                    visible: !fileModel.isFolder(index)
                                    onClicked: fileList.processFile(index);
                                }
                                Button {
                                    text: "2"
                                    Layout.fillWidth: false
                                    visible: !fileModel.isFolder(index)
                                    onClicked: fileList.processFile(index);
                                }
                                Button {
                                    text: "AI"
                                    Layout.fillWidth: false
                                    visible: !fileModel.isFolder(index)
                                    onClicked: fileList.processFile(index);
                                }
                            }
                        }
                    }

                }
            }
        }

    }

    MessageDialog {
        id: messageDialog
        title: ""
        text: ""

        onAccepted: {
            messageDialog.close();
        }

        function show(title, message) {
            messageDialog.title=title;
            messageDialog.text=message;
            messageDialog.open();
        }
    }

    FileSaveDialog {
        id: fsd
        onAccepted: {
            imp.commit();
            console.debug("SaveAs: "+file)
            if (!imp.save(file))
                messageDialog.show("Save As", "Image save failed")
        }
    }
}
