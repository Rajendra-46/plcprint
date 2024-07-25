import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Button {
    width: height
    style: ButtonStyle {}
    Image {
        id: icon
        property double iconScale: 0.8
        sourceSize.height: parent.height * iconScale
        sourceSize.width: parent.width * iconScale
        anchors.centerIn: parent
        source: "qrc:/settingsIcon.svg"
    }
}
