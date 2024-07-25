import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: window
    title: "PlcSimul"
    visible: true
    width: 600
    height: 900
    minimumWidth: 400
    minimumHeight: 200

    RowLayout {
        anchors.top: parent.top
        anchors.bottom: version.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 5
        spacing: 5

        ROSElement {
            id: incoming
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: 5
            title: "Outgoing Messages"
            firstModel: model_rosToServer
            secondModel: model_craneToServer
            thirdModel: model_yardToServer
        }

//        ROSElement {
//            id: outgoing
//            Layout.fillHeight: true
//            Layout.fillWidth: true
//            Layout.rightMargin: 5
//            title: "Incoming Messages"
//            Layout.row: 2
//            firstModel: model_serverToRos
//            secondModel: model_serverToCrane
//            thirdModel: model_serverToYard
//        }
    }

    Text {
        id: version
        text: "Version: " + softwareVersion
        height: 15
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }
}
