import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import QtCharts 2.14
import "./"
//import src.FrameProcessor 1.0

ApplicationWindow {
    id: window
    width: 960
    height: 720
    visible: true
    title: "BHCND v0.4"

    // some parameters
    // working state: 0 - piexel; 1 - pulse height spectrum; 2 - Frames.
    property int main_timer_count: 0
    property int scan_sate: 0
    property int acquisition: 0
    property string port: "8080"
    property string host_ip: combbox_host_ip.currentText
    property string device_ip: FrameProcessor.deviceIPList[combbox_device.currentIndex]
    property int s_list_image:1
    property int s_gate_on:combbox_gate_on.currentIndex
    property int s_mode:s_list_image + s_gate_on * 16
    property int s_lthd_a: 320
    property int s_lthd_b: 320
    property int s_coin_time_a: 100
    property int s_coin_time_b: 100
    property var frame_rate: 1.
    property var frame_period: 1. / frame_rate
    property var time_preset: 0
    property int time_current: 0
    property var device_settings

    Timer {
        id: main_timer
        interval: 1 / 1 * 1000 // system time's unit is 1s.
        running: false
        repeat: true
        onTriggered: {
            time_current += 1;
            if(time_current == time_preset) {
                FrameProcessor.stopDevice(device_ip);
                FrameProcessor.closeLocalServer();
                acquisition = 0
                main_timer.stop()
            }
        }
    }

    Timer {
        id: timer_delay
        running:false
        repeat: false
    }

    function delay(delayTime, cb) {
        timer_delay.interval = delayTime;
        timer_delay.repeat = false;
        timer_delay.triggered.connect(cb);
        timer_delay.triggered.connect(function release () {
            timer_delay.triggered.disconnect(cb); // This is important
            timer_delay.triggered.disconnect(release); // This is important as well
        });
        timer_delay.start();
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
                    currentIndex: 0
                    ChannelCountsView {
                        id: view_channel_counts
                        channel_counts_refresh: (acquisition) ? true : false

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

                        model: FrameProcessor.localIPList

                        delegate: ItemDelegate {
                            width: combbox_host_ip.width
                            height:combbox_host_ip.height - 5
                            contentItem: Text {
                                text: modelData
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        onActivated: {
                            // FrameProcessor.openLocalServer(host_ip);
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
                            running: false
                        }

                    }

                    ComboBox {
                        id: combbox_device
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 30

                        model: FrameProcessor.deviceList

                        delegate: ItemDelegate {
                            width: combbox_device.width
                            height: combbox_device.height - 5
                            contentItem: Text {
                                text: modelData
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        onActivated: {
                            console.log(combbox_device.currentIndex)
                            device_ip = FrameProcessor.deviceIPList[combbox_device.currentIndex];
                            console.log(FrameProcessor.deviceIPList[combbox_device.currentIndex]);
                            console.log(device_ip);
                            FrameProcessor.readDeviceSettings(device_ip);
                            busyindicator_scan.running = true;

                            // delay 50ms.
                            delay(50, function() {
                                busyindicator_scan.running = false;
                                device_settings = FrameProcessor.readDeviceSettings();
                                s_list_image = device_settings[0] & 0x0F;
                                s_gate_on = device_settings[0] / 16;
                                s_lthd_a = device_settings[1];
                                s_lthd_b = device_settings[2];
                                s_coin_time_a = device_settings[3];
                                s_coin_time_b = device_settings[4];
                                console.log("test2");
                            })
                        }
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
                        // text:"192.168.0.3"
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
                    // Layout.preferredWidth: parent.width
                    spacing: 2

                    Button {
                        id: button_scan
                        // Layout.preferredWidth: parent.width / 3 - 2
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        enabled: scan_sate ? 0 : 1
                        text: qsTr("Scan")

                        onClicked: {
                            console.log("open local server")
                            FrameProcessor.openServer(host_ip);
                            console.log("discover")
                            FrameProcessor.scanDevice();

                            // set the busy indicator.
                            busyindicator_scan.running = true;

                            // delay 1s.
                            delay(1000, function() {
                                busyindicator_scan.running = false;
                                console.log("close server")
                                FrameProcessor.closeServer();

                                // update settings
                                device_settings = FrameProcessor.readDeviceSettings();
                                s_gate_on = device_settings[0] / 16;
                                s_list_image = device_settings[0] & 0x0F;
                                s_lthd_a = device_settings[1];
                                s_lthd_b = device_settings[2];
                                s_coin_time_a = device_settings[3];
                                s_coin_time_b = device_settings[4];
                                console.log(s_gate_on, s_list_image, s_lthd_a, s_lthd_a, s_lthd_b, s_coin_time_a, s_coin_time_b)
                            })
                        }
                    }

                    Button {
                        id: button_start
                        // Layout.preferredWidth: parent.width / 3 -2
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        enabled:  (!acquisition && combbox_device.currentIndex >= 0) ? true : false
                        text: qsTr("Start")

                        onClicked: {
                            console.log(host_ip)
                            FrameProcessor.openServer(host_ip);
                            FrameProcessor.startDevice(device_ip);
                            acquisition = 1;
                            main_timer.start();
                            time_current = 0;
                        }
                    }

                    Button {
                        id: button_stop
                        // Layout.preferredWidth: parent.width / 3 - 2
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        enabled: (acquisition && combbox_device.currentIndex >= 0) ? true : false
                        text: qsTr("Stop")

                        onClicked: {
                            FrameProcessor.stopDevice(device_ip);
                            FrameProcessor.closeServer(host_ip);
                            main_timer.stop()
                            acquisition = 0
                        }
                    }
                }

                // Other control options area
                ColumnLayout {
                    width: parent.width

                    Label {
                        id: information
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Information:")
                    }

                    RowLayout {
                        Label {
                            id: label_current_mode
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Mode:")
                        }

                        Label {
                            id: text_current_mode
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: s_list_image ? "List" : "Image"
                            font.italic: true
                        }
                    }
                    RowLayout {
                        Label {
                            id: label_current_time
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Meas. time:")
                        }
                        Label {
                            id: text_current_time
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: time_current.toString()
                            font.italic: true
                        }
                    }
                }

                Rectangle {
                    id: rec_spacing
                    // Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                ColumnLayout {
                    Label {
                        id: daq_settings
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("DAQ settings:")
                    }

                    RowLayout {
                        Label {
                            id: label_time_preset
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Meas. time:")
                        }

                        TextField {
                            id: text_time_preset
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30
                            readOnly: acquisition
                            text: time_preset.toString()

                            onAccepted: {
                                time_preset = Number(text_time_preset.text);
                            }
                        }
                    }

                    RowLayout {
                        anchors.margins: 2

                        Label {
                            id: label_gate_on
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("Gate:")
                        }

                        ComboBox {
                            id: combbox_gate_on
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30

                            enabled: !acquisition

                            model: ["Off", "On"]

                            delegate: ItemDelegate {
                                width: combbox_device.width
                                height: combbox_device.height - 5
                                contentItem: Text {
                                    text: modelData
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            onActivated: {
                                s_gate_on = combbox_gate_on.currentIndex;
                                FrameProcessor.openServer(host_ip);
                                FrameProcessor.setDevice(s_mode, s_lthd_a, s_lthd_b, s_coin_time_a, s_coin_time_b, device_ip);
                                FrameProcessor.closeServer(host_ip);
                                console.log(combbox_gate_on.currentIndex);
                                console.log(s_gate_on);
                                console.log(s_mode);
                            }
                        }
                    }

                    RowLayout {
                        spacing:2
                        Label {
                            id: label_lthd_a
                            Layout.preferredWidth: 45
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("LthdA:")
                        }

                        TextField {
                            id: text_lthd_a
                            Layout.preferredWidth: 75
                            Layout.preferredHeight: 30
                            readOnly: acquisition
                            text: s_lthd_a

                            onAccepted: {
                                s_lthd_a = text_lthd_a.text
                                FrameProcessor.openServer(host_ip);
                                FrameProcessor.setDevice(s_mode, s_lthd_a, s_lthd_b, s_coin_time_a, s_coin_time_b, device_ip);
                                FrameProcessor.closeServer(host_ip);
                            }
                        }
                        Label {
                            id: label_lthd_b
                            Layout.preferredWidth: 45
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("LthdB:")
                        }

                        TextField {
                            id: text_lthd_b
                            Layout.preferredWidth: 75
                            Layout.preferredHeight: 30
                            readOnly: acquisition
                            text:s_lthd_b

                            onAccepted: {
                                s_lthd_b = text_lthd_b.text
                                FrameProcessor.openServer(host_ip);
                                FrameProcessor.setDevice(s_mode, s_lthd_a, s_lthd_b, s_coin_time_a, s_coin_time_b, device_ip);
                                FrameProcessor.closeServer(host_ip);
                            }
                        }
                    }
                    RowLayout {
                        spacing:2
                        Label {
                            id: label_coin_time_a
                            Layout.preferredWidth: 45
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("CoinA:")
                        }

                        TextField {
                            id: text_coin_time_a
                            Layout.preferredWidth: 75
                            Layout.preferredHeight: 30
                            readOnly: acquisition
                            text: s_coin_time_a

                            onAccepted: {
                                s_coin_time_a = text_coin_time_a.text
                                FrameProcessor.openServer(host_ip);
                                FrameProcessor.setDevice(s_mode, s_lthd_a, s_lthd_b, s_coin_time_a, s_coin_time_b, device_ip);
                                FrameProcessor.closeServer(host_ip);
                            }
                        }
                        Label {
                            id: label_coin_time_b
                            Layout.preferredWidth: 45
                            Layout.preferredHeight: 30
                            verticalAlignment: Text.AlignVCenter
                            text: qsTr("CoinB:")
                        }

                        TextField {
                            id: text_coin_time_b
                            Layout.preferredWidth: 75
                            Layout.preferredHeight: 30
                            readOnly: acquisition
                            text:s_coin_time_b

                            onAccepted: {
                                s_coin_time_b = text_coin_time_b.text
                                FrameProcessor.openServer(host_ip);
                                FrameProcessor.setDevice(s_mode, s_lthd_a, s_lthd_b, s_coin_time_a, s_coin_time_b, device_ip);
                                FrameProcessor.closeServer(host_ip);
                            }
                        }
                    }
                }

            }

        }


    }

}
