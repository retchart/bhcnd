import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import src.UdpServer 1.0

Rectangle {
    id: debug_area
    implicitWidth: 240
    implicitHeight: 360
    visible: false

    Label {
        id: label_debug
        Layout.preferredWidth: 90
        Layout.preferredHeight: 30
        verticalAlignment: Text.AlignVCenter
        text: qsTr("Debug data:")
    }
}
