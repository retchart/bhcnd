import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12

Rectangle {
    width: 640
    height: 640

    ColumnLayout {
        anchors.fill: parent
        spacing: 2
        anchors.margins: 2

        Rectangle {
            id: phs_x
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 2 - 10


            Text {
                text: "PHS-x"
            }
        }

        Rectangle {
            id: phs_y
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 2 - 10

            Text {
                text: "PHS-y"
            }
        }
    }
}
