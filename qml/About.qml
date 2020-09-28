import QtQuick 2.14
import QtQuick.Dialogs 1.2

Dialog {
    id: dialog
    title: "About BHCND"
    width: 480
    height: 240
    visible: true
    standardButtons: StandardButton.Ok
    onAccepted: console.log("Ok clicked")

    contentItem: Rectangle {
        width: parent.width - 2
        height: parent.height
        anchors.left: parent.left
        anchors.leftMargin: 0

        Image {
            id: iconPic
            width: 120
            height: 120
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 30
            source: "qrc:/images/bhcnd.png"
            sourceSize.height: 120
            sourceSize.width: 120
        }
        Text {
            id: infoTitleText
            width: parent.width - iconPic.width - 20
            height: 35
            anchors.right: iconPic.left
            anchors.top: parent.top
            anchors.topMargin: 30
            text: qsTr("BHCND")
            font.pixelSize: 25
            font.bold: true
        }
        Text {
            id: infoText
            width: infoTitleText.width
            height: 35
            anchors.right: iconPic.left
            anchors.top: infoTitleText.bottom
            text: qsTr("Boron-lined honeycomb convertor neutron detector")
            wrapMode: Text.Wrap
            font.bold: true
        }
        Text {
            id: versionText
            width: infoTitleText.width
            height: 15
            anchors.right: iconPic.left
            anchors.top: infoText.bottom
            text: qsTr("Version v0.1")
        }
        Text {
            id: copyrightText
            width: infoTitleText.width
            height: 15
            anchors.right: iconPic.left
            anchors.top: versionText.bottom
            anchors.topMargin: 30
            text: qsTr("Copyright© 2008-2020 Tsinghua Univercity. All Rights Reserved.")
            wrapMode: Text.Wrap
            // color: "gray"
        }
    }
}
