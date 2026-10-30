import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.folderlistmodel


Drawer {
    id: d

    height: parent.height
    width: parent.width/1.5

    signal imageSet(string image, int dest)

    function previewImageFile(file) {
        imagePreview.source=file
        imagePreview.visible=true;
    }

    FolderListModel {
        id: fileModel
        // folder: "file://"+imagePath
        showDotAndDotDot: true
        showDirsFirst: true
        nameFilters: ["*.jpg", "*.png", "*.jpeg"]
    }

    RowLayout {
        anchors.fill: parent

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
                    onClicked: d.imageSet(imagePreview.source, 1)
                }
                Button {
                    text: "2"
                    onClicked: d.imageSet(imagePreview.source, 2);
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
                var f=fileModel.get(currentIndex, "fileUrl")
                if (fileModel.isFolder(currentIndex))
                    fileModel.folder=f;
                else
                    previewImageFile(f);
            }

            function processFile(index) {
                var f=fileModel.get(currentIndex, "fileUrl")
                // processImageFile(f);
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
                    }
                }
            }
        }
    }
}