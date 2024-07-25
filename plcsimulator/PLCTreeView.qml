import QtQuick 2.11
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

TreeView {
    id: plcView
    property var type: model.modelType()
    model: treeModel
    selectionMode: type === "sending" ? SelectionMode.SingleSelection : SelectionMode.NoSelection

    function writeData(index, value, role) {
        model.writeData(index, value, role)
    }

    function readData(index, role) {
        model.readData(index, role)
    }

    TableViewColumn {
        width: parent.width * 0.6
        id: name
        role: "name"
        title: qsTr("Name")
        movable: false
    }

    TableViewColumn {
        width: parent.width * 0.1
        id: type
        role: "type"
        title: qsTr("Type")
        movable: false
    }

    TableViewColumn {
        width: parent.width * 0.1
        id: value
        role: "value"
        title: qsTr("Value")
        movable: false
    }

    TableViewColumn {
        width: parent.width * 0.2
        id: fuzz
        role: "fuzz"
        title: qsTr("Fuzz")
        movable: false
        visible: plcView.type === "sending"
    }

//    styleData.selected - if the item is currently selected
//    styleData.value - the value or text for this item
//    styleData.textColor - the default text color for an item
//    styleData.row - the index of the row
//    styleData.column - the index of the column
//    styleData.elideMode - the elide mode of the column
//    styleData.textAlignment - the horizontal text alignment of the column
//    console.log(fieldType);

    style: TreeViewStyle {
        itemDelegate: Item {
            property string fieldType: getFieldType()
            property string valueType: getValueType()

            Text {
                text: styleData.value
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: fieldType !== "name" ? parent.horizontalCenter : undefined
                font.pixelSize: 12
                width: parent.width
                clip: true
                elide: Text.ElideLeft
                visible: fieldType === "name" || fieldType === "type"
            }

            TextField {
                id: valueTextField
                visible: fieldType === "value" && valueType !== "boolean"
                         && styleData.value !== ""

                Binding on text {
                    value: styleData.value
                }

                width: parent.width
                enabled: plcView.type === "sending"

                onAccepted: {
                    plcView.model.writeData(styleData.index,
                                            valueTextField.text, "value")
                }
                onEditingFinished: {
                    plcView.model.writeData(styleData.index,
                                            valueTextField.text, "value")
                }
                textColor: "black"
            }

            PLCTreeCheckBox {
                id: valueCheckBox
                visible: fieldType === "value" && valueType === "boolean"
                         && styleData.value !== ""
                enabled: plcView.type === "sending"
                anchors.centerIn: parent
                height: parent.height

                Binding on checked {
                    value: styleData.value
                }

                onClicked: {
                    plcView.model.writeData(styleData.index,
                                            valueCheckBox.checked, "value")
                }
            }

            Item {
                anchors.fill: parent
                anchors.leftMargin: (parent.width - (fuzzCheckBox.width + settingsButton.width)) / 2
                PLCTreeCheckBox {
                    id: fuzzCheckBox
                    visible: fieldType === "fuzz" && valueType === "boolean"
                             && styleData.value !== ""
                    enabled: plcView.type === "sending"
                    anchors.left: parent.left
                    height: parent.height

                    Binding on checked {
                        value: styleData.value
                    }

                    onClicked: {
                        plcView.model.writeData(styleData.index,
                                                fuzzCheckBox.checked, "fuzz")
                    }
                }

                PLCTreeSettingsButton {
                    id: settingsButton
                    property bool rowIsBoolean: plcView.model.readData(
                                                    styleData.index,
                                                    "type") === "Bool"
                    visible: fuzzCheckBox.visible
                    anchors.left: fuzzCheckBox.right
                    height: parent.height

                    onClicked: {
                        prompt.model = plcView.model
                        prompt.index = styleData.index
                        prompt.isBoolean = rowIsBoolean
                        prompt.open()
                    }
                }
                PLCFuzzSettingsPrompt {
                    id: prompt
                    height: 400
                    width: 220
                }
            }

            MouseArea {
                anchors.fill: parent
                visible: fieldType === "name"
                onClicked: {
                    if (plcView.isExpanded(styleData.index)) {
                        plcView.collapse(styleData.index)
                    } else if (!plcView.isExpanded(styleData.index)) {
                        plcView.expand(styleData.index)
                    }
                }
            }

            function getFieldType() {
                switch (styleData.column) {
                case (0):
                    return "name"
                case (1):
                    return "type"
                case (2):
                    return "value"
                case (3):
                    return "fuzz"
                }
            }

            function getValueType() {
                return typeof (styleData.value)
            }
        }

        rowDelegate: Rectangle {
            height: 30
            color: styleData.row % 2 == 0 ? "lightGray" : "white"
        }

        headerDelegate: Rectangle {
            height: 20
            width: parent.width
            color: "#F6F3EF"
            border.color: "#999999"
            border.width: 1
            Text {
                text: styleData.value
                anchors.fill: parent
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
            }
        }
    }
}
