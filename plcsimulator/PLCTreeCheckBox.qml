import QtQuick 2.11
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

CheckBox {
    id: box
    style: CheckBoxStyle {
        indicator: Rectangle {
            implicitWidth: box.height
            implicitHeight: box.height
            radius: 3
            border.color: box.checked ? "darkblue" : "gray"
            border.width: 1

            Rectangle {
                id: indicator
                visible: box.checked
                color: "#555"
                border.color: "#333"
                radius: 3
                anchors.margins: 4
                anchors.fill: parent
            }
        }
    }
}
