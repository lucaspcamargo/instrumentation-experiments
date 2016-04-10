import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import qmldash 1.0

ApplicationWindow {
    id: window
    visible: true

    width: 450
    height: 332

    title: "Speed Control"

    property real maxSpeed: 1200

    color: "transparent"
    flags: Qt.FramelessWindowHint

    Rectangle
    {
        color: "#333"
        anchors.fill: parent
        radius: 5
    }

    SerialPort
    {
        id: serial
        portName: getPortNameByIndex(0)
        baudRate: 115200

        property bool ready: false

        Component.onCompleted: openSerial();

        property real hz: 0
        property real rpm: hz * 60

        property real tickCounter: 0
        property real tickCounterL: 0
        property real angle: 0

        property real encoderTicks: ticksSpin.value
        property real tickAngle: 360 / encoderTicks

        property string lastChar: ""

        onReadyRead:
        {
            while(bytesAvailable())
            {
                var c = readChar();

                tickCounter ++;
                if(c == 'L')
                {
                    angle -= tickAngle;
                    tickCounterL++;
                }
                else if(c == 'R')
                {
                    angle += tickAngle;
                }
            }

            activity.on = true;
            lastChar = c;
        }

    }

    Timer
    {
        id: samplingTimer
        interval: samplingSpin.value
        running: true
        repeat: true
        onTriggered:
        {
            var revsWalked = serial.tickCounter / serial.encoderTicks;
            serial.hz = revsWalked / (interval / 1000.0);

            if(serial.tickCounterL > 0 && serial.tickCounterL != serial.tickCounter)
                warning.text = "Direction changes";
            else
                warning.text = "";

            serial.tickCounter = 0;
            serial.tickCounterL = 0;
        }
    }


    Timer
    {
        id: controlTimer
        interval: 250
        running: true
        repeat: true

        property real pwm: 0
        property real factor: 255 / maxSpeed // max pwm raw over S on max pwm

        onTriggered:
        {
            if(directBtn.checked)
            {
                serial.sendChar(255.0 * speedSlider.value / speedSlider.maximumValue);
                return;
            }

            var diff = serial.rpm - speedSlider.value;

            pwm -= 0.4 * diff * factor;

            pwm = Math.min( 255, Math.max( 0, pwm ));

            serial.sendChar(pwm);
        }
    }


    Rectangle
    {
        id: mainPane
        x: 5
        y: 5
        radius: 5
        color: "#111"
        width: parent.width - 10
        height: parent.height - 30

        Text
        {
            id: appTitle

            font.pixelSize: 20
            font.family: "Oxygen-Sans"
            text: "Speed Control"
            color: "#eee"

            x: 10
            y: 10
        }

        Text
        {
            id: freqLabel

            font.pixelSize: 70
            font.family: "Oxygen Mono"
            text: Math.round(serial.rpm)
            opacity: serial.level > 0.10? 1 : 0.5
            horizontalAlignment: Text.AlignRight
            width: 220
            anchors.right: parent.right
            anchors.rightMargin: 95
            height: 80
            verticalAlignment: Text.AlignBottom
            color: "#eee"

            Behavior on opacity{NumberAnimation{}}
        }

        Text
        {
            id: targetLabel

            font.pixelSize: 30
            text: speedSlider.value + " rpm"
            x: 10
            height: 80
            verticalAlignment: Text.AlignBottom
            color: "#eee"
            opacity: directBtn.checked? 0.25 : 1
        }

        Text
        {
            id: warning
            x: 10
            y: 50
            text: ""
            font.pixelSize: 14
            opacity: text == ""? 0 : 1
            Behavior on opacity{NumberAnimation{}}
            color: "#f55"
        }

        Text
        {
            id: freqUnit
            anchors.right: parent.right
            anchors.rightMargin: 20
            height: 76
            verticalAlignment: Text.AlignBottom
            text: "rpm"
            font.pixelSize: 40
            color: "#eee"
        }

        Text
        {
            text: "⚙"
            color: "#eee"

            anchors.right: parent.right
            anchors.baseline: parent.bottom
            anchors.margins: 10
            anchors.baselineOffset: -anchors.margins

            opacity: confMA.containsMouse? 1.0 : 0.5

            font.pixelSize: 22

            MouseArea {
                id: confMA
                anchors.fill: parent
                hoverEnabled: true
                onClicked: settingsDialog.visible = !settingsDialog.visible
            }
        }


        Slider
        {
            id: speedSlider

            width: parent.width-20
            x: 10
            y: 80

            minimumValue: 0
            maximumValue: maxSpeed

            stepSize: 25

            tickmarksEnabled: true
        }


        Button
        {
            id: directBtn
            checkable: true
            checked:  false

            text: "Bypass control"
            x: 10
            y: parent.height - height - 10
        }

        Item
        {
            id: positionView
            width: parent.width
            height: (parent.height - 100    )
            x: 0
            y: 100



            Rectangle
            {
                id: positionDial
                width: 0.8 * Math.min(parent.width, parent.height)
                height: width
                radius: width/2

                anchors.centerIn: parent
                color: "#333"

                rotation: serial.angle

                Rectangle
                {
                    id: positionDialCenter
                    width: parent.width*0.75
                    height: width
                    radius: width/2

                    anchors.centerIn: parent
                    color: "#111"
                }

                Rectangle
                {
                    id: positionDialTick
                    width: parent.width/15
                    height: width
                    radius: width/2

                    anchors.horizontalCenter: parent.horizontalCenter
                    y: width / 2
                    color: "#111"
                }
            }

            Text
            {
                text: serial.lastChar == "R"? "⤼" : (serial.lastChar == "L"? "⤽" : "")
                color: serial.lastChar == "R"? "#ff6" : (serial.lastChar == "L"? "#66f" : "#eee")

                anchors.centerIn: parent
                anchors.verticalCenterOffset: -10

                opacity: confMA.containsMouse? 1.0 : 0.5

                font.pixelSize: 40

            }
        }


        /*
        Canvas
        {
            id: fft
            width: parent.width
            anchors.top: waveform.bottom
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            x: 0
            y: 200
            contextType: "2d"
            property color waveColor: "#ff6"
            property color lineColor: "#181818"
            property color markerColor: waveColor
            property int freqInterval: 200

            onPaint:
            {
                if(!available) return;
                if(!serial.ready) return;

                context.clearRect(0,0,width, height);

                var hh = height / 2;
                context.lineWidth = 1;


                var points = serial.bufferSize/2;
                var dx = width / points;
                var fftLevel = serial.fftLevel;

                var freqSpacing = 1.0 * serial.bufferSize / serial.sampleRate;
                var freqX = freqInterval * freqSpacing * dx;
                var peakX = dx * serial.fftPeakFrequency * freqSpacing;

                context.strokeStyle = lineColor;
                context.lineWidth = 1.5;
                context.beginPath();
                while(freqX < width)
                {
                    context.moveTo(freqX, 2);
                    context.lineTo(freqX, height);
                    freqX += freqInterval * freqSpacing * dx;
                }
                context.stroke();

                context.fillStyle = markerColor;
                context.beginPath();

                context.moveTo(peakX-5, 0);
                context.lineTo(peakX+5, 0);
                context.lineTo(peakX, 5);
                context.closePath();

                context.fill();


                context.strokeStyle = waveColor;
                context.lineWidth = 1;

                context.beginPath();
                context.moveTo(0, height);
                for(var i = 0; i < points; i++)
                {
                    context.lineTo(i*dx, (height-1) * (1.0 - serial.fftSample(i)/fftLevel) );
                }
                context.stroke();

            }

            Text
            {
                text: fft.freqInterval + " Hz / div"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
                color: "#666"
            }

            Text
            {
                text: "Hann Window " + (serial.hannWindowEnabled? "On" : "Off")
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                color: serial.hannWindowEnabled? "#ff6" : "#666"

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: serial.hannWindowEnabled = !serial.hannWindowEnabled
                }
            }
        }
    */

    }


    Rectangle {

        id: statusbar
        color: "transparent"
        width: parent.width
        height: 20
        anchors.bottom: parent.bottom

        Text {
            text: serial.portName + " @ " + serial.baudRate + " baud "
            color: "#666"
            x: 6
        }


        Item {
            height: parent.height
            width: 20

            Layout.alignment: Qt.AlignRight
            anchors.right: parent.right

            Rectangle
            {
                id: led
                property bool on: serial.serialPort !== null && serial.state == SerialPort.STATE_OPEN
                property bool error: serial.state == SerialPort.STATE_ERROR
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 4

                color: error? "red" : (on? "lime" : "#80000000")
            }
        }

        Item {
            height: parent.height
            width: 20

            anchors.right: parent.right
            anchors.rightMargin: 14

            Rectangle
            {
                id: activity
                property bool on: false
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 4

                color: on? "skyblue" : "#80000000"

                onOnChanged: if(on){ activityOffTimer.start(); }

                Timer
                {
                    id: activityOffTimer
                    interval: 200
                    onTriggered: activity.on = false
                }
            }
        }

    }

    function openSerial()
    {
        console.log("Opening serial...");
        serial.open();
        serial.sendChar(0);
    }

    Component.onDestruction:
    {
        serial.sendChar(0);
    }

    Dialog
    {
        id: settingsDialog

        visible: false
        title: "Settings"

        width: 200
        height: 120

        GridLayout
        {
            anchors.fill: parent
            columns: 2

            Label{ text: "Ticks per revolution" }

            SpinBox{ id: ticksSpin; minimumValue: 3; maximumValue: 128; value: 60 }

            Label{ text: "Sampling window" }

            SpinBox{ id: samplingSpin; minimumValue: 200; maximumValue: 1000; value: 100; suffix: " ms" }

        }
    }

}

