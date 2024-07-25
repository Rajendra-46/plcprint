import QtQuick 2.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3

Dialog {
    property var model: undefined
    property var index: undefined
    property bool isBoolean: false

    onAccepted: {
        var success = true
        var val
        if(radioWave.checked)
        {
            if(cmbWave.currentText == "Sine wave")
            {
                val = 10;
            }else if(cmbWave.currentText == "Sawtooth wave")
            {
                val = 11;
            }else{
                val = 12;
            }
            if(avgValueField.text == "")
            {
              avgValueField.text = "0";
            }
        }
        else
        {
            if(cmbWave.currentText == "Sine wave")
            {
                val = 20;
            }else if(cmbWave.currentText == "Sawtooth wave")
            {
                val = 21;
            }else{
                val = 22;
            }
        }
        success = success ? model.writeData(index, minValueField.text,
                                            "fuzzMin") : false
        success = success ? model.writeData(index, maxValueField.text,
                                            "fuzzMax") : false
        success = success ? model.writeData(index, avgValueField.text,
                                            "avg") : false
        success = success ? model.writeData(index, val,
                                            "waves") : false
        success = success ? model.writeData(index, intervalField.text,
                                            "fuzzInterval") : false

        if (!success) {
            setUp()
            error.visible = true
            open()
        }
    }

    onModelChanged: {
        if (model !== undefined && index !== undefined) {
            setUp()
        }
    }

    onIndexChanged: {
        if (model !== undefined && index !== undefined) {
            setUp()
        }
    }

    function setUp() {
        var fuzzMin = model.readData(index, "fuzzMin")
        var fuzzMax = model.readData(index, "fuzzMax")
        var avg = model.readData(index, "avg")
        var waves = model.readData(index, "waves")
        var fuzzInterval = model.readData(index, "fuzzInterval")

        var type = model.readData(index, "type")
        var name = model.readData(index, "name")

        minValueField.text = fuzzMin
        maxValueField.text = fuzzMax
        avgValueField.text = avg
        intervalField.text = fuzzInterval

        valueName.text = "Name : " + name
        valueType.text = "Type : " + type

        // cmbWave.currentText = waves

        error.visible = false
    }

    ColumnLayout {

        anchors.fill: parent
        Label {
            id: header
            text: "Additional Fuzz Settings"
        }
        Item {
            height: 10
            visible: !error.visible
        }
        Label {
            id: error
            text: "Invalid input"
            color: "red"
            visible: false
            height: 10
            Layout.fillWidth: true
        }

        Label {
            id: valueName
        }
        Label {
            id: valueType
        }

        Item {
            height: 10
        }
        RadioButton {
            id: radioMinMax
            checked: true
            visible: !isBoolean
            text: qsTr("Fuzz")
        }

        Label {
            id: minValueLabel
            text: "Minimum value"
            visible: !isBoolean
            Layout.fillWidth: true
        }
        TextField {
            id: minValueField
            visible: !isBoolean
            Layout.fillWidth: true
        }
        Label {
            id: maxValueLabel
            text: "Maximum value"
            visible: !isBoolean
            Layout.fillWidth: true
        }
        TextField {
            id: maxValueField
            visible: !isBoolean
            Layout.fillWidth: true
        }
        Label {
            id: intervalLabel
            text: "Fuzz interval in seconds (0 = \u221E)"
            Layout.fillWidth: true
        }
        TextField {
            id: intervalField
            Layout.fillWidth: true
        }
        RadioButton {
            id: radioWave
            checked: false
            visible: !isBoolean
            text: qsTr("Waves")
        }
        ComboBox {
            id: cmbWave
            visible: !isBoolean
            Layout.fillWidth: true
            model: ["Sine wave", "Sawtooth wave", "Triangle wave" ]
        }
        Label {
            id: avgValueLabel
            text: "Number of Cycles"
            visible: !isBoolean
            Layout.fillWidth: true
        }
        TextField {
            id: avgValueField
            visible: !isBoolean
            Layout.fillWidth: true
        }
    }
}
