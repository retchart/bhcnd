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
            id: x_view
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 2 - 10


            Text {
                text: "1D-x"
            }
        }

        Rectangle {
            id: y_view
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 2 - 10

            Text {
                text: "1D-y"
            }
        }
    }
}
