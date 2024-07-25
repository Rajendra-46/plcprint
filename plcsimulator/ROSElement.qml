import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

Item {
    id: rosElement
    property string title: ""
    property var firstModel: undefined
    property var secondModel: undefined
    property var thirdModel: undefined

    Rectangle {
        id: header
        width: parent.width
        height: 50
        anchors.top: parent.top
        Text {
            id: sendingLabel
            anchors.fill: parent
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            text: rosElement.title
            font.pixelSize: 16
            font.bold: true
        }
    }
    TabBar {
        id: topBar
        width: parent.width
        anchors.top: header.bottom
        Connections {
            target: worker_ros
            onStatusChanged: {
                rosIndicator.color = topBar.getColor(rosElement.firstModel,
                                                     worker_ros)
            }
        }

        Connections {
            target: worker_crane
            onStatusChanged: {
                craneIndicator.color = topBar.getColor(rosElement.secondModel,
                                                       worker_crane)
            }
        }

        Connections {
            target: worker_yard
            onStatusChanged: {
                yardIndicator.color = topBar.getColor(rosElement.thirdModel,
                                                      worker_yard)
            }
        }

        TabButton {
            id: rosButton
            text: rosElement.firstModel.modelId()
            font.pixelSize: 14
            Rectangle {
                id: rosIndicator
                height: parent.height / 2
                width: parent.height / 2
                radius: height
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                color: topBar.getColor(rosElement.firstModel, worker_ros)
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var color = topBar.getColor(rosElement.firstModel,
                                                    worker_ros)
                        if (color === "red") {
                            buttons.start(true, 0)
                        } else if (color === "green") {
                            buttons.start(false, 0)
                        }
                    }
                }
            }
        }
        TabButton {
            id: craneButton
            text: rosElement.secondModel.modelId()
            font.pixelSize: 14
            Rectangle {
                id: craneIndicator
                height: parent.height / 2
                width: parent.height / 2
                radius: height
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                color: topBar.getColor(rosElement.secondModel, worker_crane)
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var color = topBar.getColor(rosElement.secondModel,
                                                    worker_crane)
                        if (color === "red") {
                            buttons.start(true, 1)
                        } else if (color === "green") {
                            buttons.start(false, 1)
                        }
                    }
                }
            }
        }
        TabButton {
            id: yardButtor
            text: rosElement.thirdModel.modelId()
            font.pixelSize: 14

            Rectangle {
                id: yardIndicator
                height: parent.height / 2
                width: parent.height / 2
                radius: height
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                color: topBar.getColor(rosElement.thirdModel, worker_yard)

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var color = topBar.getColor(rosElement.thirdModel,
                                                    worker_yard)
                        if (color === "red") {
                            buttons.start(true, 2)
                        } else if (color === "green") {
                            buttons.start(false, 2)
                        }
                    }
                }
            }
        }

        function getColor(model, worker) {
            if (model.modelType() === "sending") {
                return worker.sending() ? "green" : "red"
            } else if (model.modelType() === "receiving") {
                return worker.receiving() ? "green" : "red"
            } else {
                return "gray"
            }
        }
    }

    StackLayout {
        id: treeViewStack
        width: parent.width
        anchors.top: topBar.bottom
        currentIndex: topBar.currentIndex
        anchors.bottom: buttons.top

        PLCTreeView {
            id: firstElement
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: rosElement.firstModel
        }

        PLCTreeView {
            id: secondElement
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: rosElement.secondModel
        }

        PLCTreeView {
            id: thirdElement
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: rosElement.thirdModel
        }
    }

    RowLayout {
        id: buttons
        height: 40
        width: parent.width
        anchors.bottom: parent.bottom

        Button {
            text: "Start " + buttons.getOperation()
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: {
                buttons.start(true, treeViewStack.currentIndex)
            }
        }

        Button {
            text: "Stop " + buttons.getOperation()
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: {
                buttons.start(false, treeViewStack.currentIndex)
            }
        }

        function getOperation() {
            if (treeViewStack.currentIndex === 0) {
                if (rosElement.firstModel.modelType() === "sending") {
                    return "Sending"
                } else if (rosElement.firstModel.modelType() === "receiving") {
                    return "Receiving"
                }
            } else if (treeViewStack.currentIndex === 1) {
                if (rosElement.secondModel.modelType() === "sending") {
                    return "Sending"
                } else if (rosElement.secondModel.modelType() === "receiving") {
                    return "Receiving"
                }
            } else if (treeViewStack.currentIndex === 2) {
                if (rosElement.thirdModel.modelType() === "sending") {
                    return "Sending"
                } else if (rosElement.thirdModel.modelType() === "receiving") {
                    return "Receiving"
                }
            }
        }

        function start(start, index) {
            if (index === 0) {
                if (rosElement.firstModel.modelType() === "sending") {
                    if (start) {
                        worker_ros.startSending()
                    } else {
                        worker_ros.stopSending()
                    }
                } else if (rosElement.firstModel.modelType() === "receiving") {
                    if (start) {
                        worker_ros.startReceiving()
                    } else {
                        worker_ros.stopReceiving()
                    }
                }
            } else if (index === 1) {
                if (rosElement.secondModel.modelType() === "sending") {
                    if (start) {
                        worker_crane.startSending()
                    } else {
                        worker_crane.stopSending()
                    }
                } else if (rosElement.secondModel.modelType() === "receiving") {
                    if (start) {
                        worker_crane.startReceiving()
                    } else {
                        worker_crane.stopReceiving()
                    }
                }
            } else if (index === 2) {
                if (rosElement.thirdModel.modelType() === "sending") {
                    if (start) {
                        worker_yard.startSending()
                    } else {
                        worker_yard.stopSending()
                    }
                } else if (rosElement.thirdModel.modelType() === "receiving") {
                    if (start) {
                        worker_yard.startReceiving()
                    } else {
                        worker_yard.stopReceiving()
                    }
                }
            }
        }
    }
}
