import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtCharts 2.14

Rectangle {
    id: view_1d
    width: 640
    height: 640

    property bool channel_counts_refresh : false

    Timer {
        id: refresh_timer_wires
        interval: 1 / 1 * 1000 // 10 Hz
        running: channel_counts_refresh
        repeat: true
        onTriggered: {
            chart_wires_x.ymax = FrameProcessor.refreshChannelCountsX(chart_wires_x.series(0));
            chart_wires_y.ymax = FrameProcessor.refreshChannelCountsY(chart_wires_y.series(0));
            // console.log("refresh_1dview")
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 2
        anchors.margins: 2

        Rectangle {
            id: x_view
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 2 - 10

            ChartView {
                id: chart_wires_x
                title: "X direction"
                anchors.fill: parent
                antialiasing: true

                property int ymax: 256

                ValueAxis {
                    id: axisX_wires_x
                    min: 0
                    max: 32
                }
                ValueAxis {
                    id: axisY_wires_x
                    min: 0
                    max: chart_wires_x.ymax
                }

                LineSeries {
                    id: line_wires_x
                    // name: "LineSeries"
                    axisX: axisX_wires_x
                    axisY: axisY_wires_x

                    XYPoint { x: 1; y: 1 }
                    XYPoint { x: 2; y: 2 }
                    XYPoint { x: 3; y: 3 }
                    XYPoint { x: 4; y: 4 }
                    XYPoint { x: 5; y: 5 }
                    XYPoint { x: 6; y: 6 }
                    XYPoint { x: 7; y: 7 }
                    XYPoint { x: 8; y: 8 }
                    XYPoint { x: 9; y: 1 }
                    XYPoint { x: 10; y: 2 }
                    XYPoint { x: 11; y: 3 }
                    XYPoint { x: 12; y: 4 }
                    XYPoint { x: 13; y: 5 }
                    XYPoint { x: 14; y: 6 }
                    XYPoint { x: 15; y: 7 }
                    XYPoint { x: 16; y: 8 }
                    XYPoint { x: 17; y: 1 }
                    XYPoint { x: 18; y: 2 }
                    XYPoint { x: 19; y: 3 }
                    XYPoint { x: 20; y: 4 }
                    XYPoint { x: 21; y: 5 }
                    XYPoint { x: 22; y: 6 }
                    XYPoint { x: 23; y: 7 }
                    XYPoint { x: 24; y: 8 }
                    XYPoint { x: 25; y: 1 }
                    XYPoint { x: 26; y: 2 }
                    XYPoint { x: 27; y: 3 }
                    XYPoint { x: 28; y: 4 }
                    XYPoint { x: 29; y: 5 }
                    XYPoint { x: 30; y: 6 }
                    XYPoint { x: 31; y: 7 }
                    XYPoint { x: 32; y: 8 }
                }
            }
        }

        Rectangle {
            id: y_view
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 2 - 10

            ChartView {
                id: chart_wires_y
                title: "Y direction"
                anchors.fill: parent
                antialiasing: true

                property int ymax: 256

                ValueAxis {
                    id: axisX_wires_y
                    min: 0
                    max: 32
                }
                ValueAxis {
                    id: axisY_wires_y
                    min: 0
                    max: chart_wires_y.ymax
                }

                LineSeries {
                    id: line_wires_y
                    // name: "LineSeries"
                    axisX: axisX_wires_y
                    axisY: axisY_wires_y

                    XYPoint { x: 1; y: 1 }
                    XYPoint { x: 2; y: 2 }
                    XYPoint { x: 3; y: 3 }
                    XYPoint { x: 4; y: 4 }
                    XYPoint { x: 5; y: 5 }
                    XYPoint { x: 6; y: 6 }
                    XYPoint { x: 7; y: 7 }
                    XYPoint { x: 8; y: 8 }
                    XYPoint { x: 9; y: 1 }
                    XYPoint { x: 10; y: 2 }
                    XYPoint { x: 11; y: 3 }
                    XYPoint { x: 12; y: 4 }
                    XYPoint { x: 13; y: 5 }
                    XYPoint { x: 14; y: 6 }
                    XYPoint { x: 15; y: 7 }
                    XYPoint { x: 16; y: 8 }
                    XYPoint { x: 17; y: 1 }
                    XYPoint { x: 18; y: 2 }
                    XYPoint { x: 19; y: 3 }
                    XYPoint { x: 20; y: 4 }
                    XYPoint { x: 21; y: 5 }
                    XYPoint { x: 22; y: 6 }
                    XYPoint { x: 23; y: 7 }
                    XYPoint { x: 24; y: 8 }
                    XYPoint { x: 25; y: 1 }
                    XYPoint { x: 26; y: 2 }
                    XYPoint { x: 27; y: 3 }
                    XYPoint { x: 28; y: 4 }
                    XYPoint { x: 29; y: 5 }
                    XYPoint { x: 30; y: 6 }
                    XYPoint { x: 31; y: 7 }
                    XYPoint { x: 32; y: 8 }
                }
            }
        }
    }
}
