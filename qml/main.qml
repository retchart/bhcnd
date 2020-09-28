import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import "./"
import src.UdpServer 1.0

ApplicationWindow {
    id: window
    width: 960
    height: 720
    visible: true
    title: "BHCND v0.1"

    // some parameters
    // working state: 0 - piexel; 1 - pulse height spectrum; 2 - Frames.
    property int working_state: bar.currentIndex
    property int timer100ms_count: 0
    property int scan_sate: 0
    property int acquisition: 0
    property string port: "2001"
    property string host_ip: combbox_host_ip.currentText
    property string device_ip: ""
    property var frame_rate: 1.
    property var frame_period: 1. / frame_rate
    property var preset_time: 0
    property int event_num: 1000
    property int event_current: 1000

    UdpServer {
        id: udp_server
        onDeviceListChanged: {
            device_ip = udp_server.deviceIPList[0]
        }
    }

    Timer {
        id: timer100ms
        interval: 100
        repeat: true

        onTriggered: {
            timer100ms_count++;
            if(timer100ms_count > 29 && scan_sate == 1)
            {
                scan_sate = 0;
                udp_server.closeServer();
            }
        }
    }

    FileDialog {
        id: openDialog
        title: "Please choose a file"
        folder: shortcuts.home
        // folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: ["Text files (*.txt)"]
        onAccepted: document.load(file)
    }

    FileDialog {
        id: saveDialog
        title: "Save as"
        folder: shortcuts.home
        defaultSuffix: document.fileType
        nameFilters: openDialog.nameFilters
        onAccepted: document.saveAs(file)
    }

    About {
        id: about
        visible: false
    }

    menuBar: MenuBar {
        id: menubar
        Layout.leftMargin: 2
        height: 30
        spacing: 1

        background: Rectangle {
            opacity: enabled ? 1 : 0.3
            // color: control.down ? "#d0d0d0" : "#e0e0e0"
        }

        Menu {
            title: qsTr("&File")

            Action {
                text: qsTr("&Open")

                onTriggered: openDialog.open()
            }
            Action {
                text: qsTr("&Save As...")
                onTriggered: saveDialog.open()
            }
            Action {
                text: qsTr("&Quit")
                onTriggered: close()
            }

            delegate: MenuItem {
                implicitWidth: 100
                implicitHeight: 30
            }
        }

        Menu {
            title: qsTr("&Acquisition")

            Action {
                text: qsTr("&Connect")
            }
            Action {
                text: qsTr("&Start")
            }
            Action {
                text: qsTr("S&top")
            }

            delegate: MenuItem {
                implicitWidth: 100
                implicitHeight: 30
            }
        }

        Menu {
            title: qsTr("&Help")

            Action {
                text: qsTr("&About")
                onTriggered: about.open()
            }

            delegate: MenuItem {
                implicitWidth: 100
                implicitHeight: 30
            }
        }

        delegate: MenuBarItem {
            id: menuBarItem
            // implicitWidth: 100
            implicitHeight: 30
        }
    }

    GridLayout{
        anchors.fill: parent
        columns: 2
        columnSpacing: 2
        rowSpacing: 2
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        anchors.bottomMargin: 1

        Rectangle{
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: "gray"
            border.width: 1
            anchors.margins: 1

            ColumnLayout {
                anchors.fill: parent
                Layout.fillHeight: true
                anchors.margins: 1

                // data view area
                StackLayout {
                    width: parent.width
                    currentIndex: bar.currentIndex
                    BHCND2DView {
                        id: view_2d
                    }
                    BHCNDPHSView {
                        id: view_phs
                    }

                    BHCND1DView {
                        id: view_1d
                    }
                }

                // Toggle between two data views
                TabBar {
                    id: bar
                    Layout.preferredHeight: 30
                    TabButton {
                        implicitWidth: 75
                        implicitHeight: 25
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Pixel")
                        font.pointSize: 9
                    }

                    TabButton {
                        implicitWidth: 75
                        implicitHeight: 25
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("PHS")
                        font.pointSize: 9
                    }

                    TabButton {
                        implicitWidth: 75
                        implicitHeight: 25
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Frame")
                        font.pointSize: 9
                    }
                }
            }
        }

        // Right part of the main window
        Rectangle{
            width: 250
            Layout.fillHeight: true
            border.color: "gray"
            border.width: 1

            GridLayout{
                anchors.fill: parent
                columns: 1
                columnSpacing: 2
                rowSpacing: 2
                anchors.margins: 2

                RowLayout {
                    Label {
                        id: label_host_ip
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Host IP:")
                    }

                    ComboBox {
                        id: combbox_host_ip
                        font.family: "Arial"
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30

                        model: udp_server.localIPList

                        delegate: ItemDelegate {
                            width: combbox_host_ip.width
                            height:combbox_host_ip.height - 5
                            contentItem: Text {
                                text: modelData
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        onActivated: {
                            //mainWindow.serialPortName = serialComboBox.currentText;
                        }
                    }
                }

                RowLayout {
                    Label {
                        id: label_host_port
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Host port:")
                    }

                    TextField {
                        id: text_host_port
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        readOnly: true

                        text: port
                    }
                }

                RowLayout {
                    Label {
                        id: label_frame_rate
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Frame rate:")
                    }

                    TextField {
                        id: text_frame_rate
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: frame_rate.toString()

                        onEditingFinished: {
                            frame_rate = parseFloat(text_frame_rate.text)
                            console.log(frame_rate)
                            frame_period = 1. / frame_rate
                        }
                    }
                }

                RowLayout {
                    Label {
                        id: label_frame_period
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Frame period:")
                    }

                    TextField {
                        id: text_frame_period
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: frame_period.toString()
                        readOnly: true
                    }
                }

                RowLayout {
                    Label {
                        id: label_daq_ip
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("DAQ IP:")
                    }

                    TextField {
                        id: text_daq_ip
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        readOnly: true
                        // text:"192.168.0.253"
                        text: device_ip
                        validator: RegExpValidator
                            {
                                regExp: /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
                            }
                    }
                }

                RowLayout {
                    Label {
                        id: label_daq_type
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("DAQ type:")
                    }

                    TextField {
                        id: text_daq_type
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30
                        readOnly: true
                        text:"IDE1162"
                    }
                }

                RowLayout {
                    anchors.margins: 2

                    RowLayout {
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30

                        Label {
                            id: label_device
                            Layout.preferredWidth: 55
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Device:")
                        }

                        BusyIndicator {
                            id: busyindicator_scan
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 20
                            running: scan_sate == 1
                        }

                    }

                    ComboBox {
                        id: combbox_device
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30

                        model: udp_server.deviceList

                        delegate: ItemDelegate {
                            width: combbox_device.width
                            height: combbox_device.height - 5
                            contentItem: Text {
                                text: modelData
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        onActivated: {
                            device_ip = udp_server.deviceIPList[combbox_device.currentIndex]
                        }
                    }
                }

                RowLayout {
                    Layout.preferredWidth: parent.width
                    spacing: 2

                    Button {
                        id: button_scan
                        Layout.preferredWidth: parent.width / 3 - 2
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        enabled: scan_sate ? 0 : 1
                        text: qsTr("Scan")

                        onClicked: {
                            udp_server.openLocalServer(host_ip, port);
                            udp_server.discover()
                            timer100ms.start()
                            timer100ms_count = 0
                            scan_sate = 1
                        }
                    }

                    Button {
                        id: button_start
                        Layout.preferredWidth: parent.width / 3 -2
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        enabled:  (!acquisition && combbox_device.currentIndex >= 0) ? true : false
                        text: qsTr("Start")

                        onClicked: {
                            console.log(host_ip, port)
                            udp_server.openLocalServer(host_ip, port);
                            acquisition = 1;
                        }
                    }

                    Button {
                        id: button_stop
                        Layout.preferredWidth: parent.width / 3 - 2
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        enabled: (acquisition && combbox_device.currentIndex >= 0) ? true : false
                        text: qsTr("Stop")

                        onClicked: {
                            acquisition = 0
                        }
                    }
                }

                // Other control options area
                StackLayout {
                    width: parent.width
                    currentIndex: working_state

                    RowLayout {
                        Label {
                            id: label_auto_save
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Auto save:")
                        }

                        Switch {
                            id: switch_auto_save
                            width: 30
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 30
                            scale: 0.8
                            checked: false

                            onCheckedChanged: {

                            }
                        }
                    }
                    RowLayout {
                        Label {
                            id: label_phs
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Preset time:")
                        }

                        TextField {
                            id: text_phs
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30
                            readOnly: true
                            text: preset_time.toString()
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: parent.width
                        spacing: 1

                        Label {
                            id: label_event_index
                            Layout.preferredWidth: 75
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Event:")
                        }

                        Slider {
                            id: slider_event_index
                            // Layout.preferredWidth: 75
                            Layout.fillWidth: true
                            Layout.preferredHeight: 30
                            from: 0
                            to: event_num
                            value: event_current

                            onMoved: {
                                event_current = slider_event_index.value
                            }
                        }
                        SpinBox {
                            id: spinbox_event_index
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 30
                            from: 0
                            to: event_num
                            value: event_current

                            onValueModified: {
                                event_current = spinbox_event_index.value
                            }
                        }
                    }
                }

                Rectangle {
                    id: rec_spacing
                    // Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // Debug area
                DebugArea {
                    id: debug_area
                    Layout.preferredWidth: parent.width
                    Layout.fillHeight: true
                    Layout.topMargin: 15
                }

                // Debug enagle
                RowLayout {
                    Label {
                        id: label_debug_enable
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        text: qsTr("Debug:")
                        verticalAlignment: Text.AlignVCenter
                    }

                    Switch {
                        id: switch_debug
                        width: 30
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 30
                        scale: 0.8
                        checked: false

                        onCheckedChanged: {
                            udp_server.debugEnable(switch_debug.checked);
                            debug_area.visible = switch_debug.checked;
                        }
                    }
                }
            }

        }


    }

}
