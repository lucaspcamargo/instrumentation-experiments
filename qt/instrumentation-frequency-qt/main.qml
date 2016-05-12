import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import qmldash 1.0

ApplicationWindow {
    id: window
    visible: true

    width: 450
    height: 332

    title: "Frequency Meter"

    color: "transparent"
    flags: Qt.FramelessWindowHint
    Rectangle
    {
        color: "#333"
        anchors.fill: parent
        radius: 5
    }

    AudioSerialPort
    {
        id: serial
        portName: getPortNameByIndex(0)
        baudRate: 921600

        property bool ready: false
        property real smoothLevel: level
        Behavior on smoothLevel { NumberAnimation{ duration: 250; easing.type: Easing.InOutQuart} }

        onProcessed:
        {
            ready = true;
            activity.on = true;
            waveform.requestPaint();
            fft.requestPaint();
        }

        Component.onCompleted: openSerial();
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
            text: "Frequency Meter"
            color: "#eee"

            x: 10
            y: 10
        }

        Text
        {
            id: freqLabel

            font.pixelSize: 70
            font.family: "Oxygen Mono"
            text: serial.fftPeakFrequency
            opacity: serial.level > 0.10? 1 : 0.5
            horizontalAlignment: Text.AlignRight
            width: 220
            anchors.right: parent.right
            anchors.rightMargin: 70
            height: 80
            verticalAlignment: Text.AlignBottom
            color: "#eee"

            Behavior on opacity{NumberAnimation{}}
        }

        Text
        {
            id: levelWarning
            x: 10
            y: 50
            text: serial.cutoff? "Level is too high!" : "Level is too low!"
            font.pixelSize: 14
            opacity: serial.level > 0.10 || serial.cutoff? 0 : 1
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
            text: "Hz"
            font.pixelSize: 40
            color: "#eee"
        }

        Rectangle
        {
            id: levelBar

            y: 85
            width: parent.width
            height: 10
            color: "#080808"

            Rectangle
            {
                x: 0
                y: 2
                height: parent.height - 4
                width: parent.width * Math.min(serial.smoothLevel, 1)
                color: serial.cutoff? "#f55" : "#6ff"
                Behavior on color{ColorAnimation{}}
            }
        }

        Canvas
        {
            id: waveform
            width: parent.width
            height: (parent.height - 100) / 2
            x: 0
            y: 100
            contextType: "2d"
            property color waveColor: "#6ff"
            property color lineColor: "#181818"
            property color waveColorCutoff: "#f66"

            onPaint:
            {
                if(!available) return;
                if(!serial.ready) return;

                context.clearRect(0,0,width, height);

                context.strokeWidth = 1;


                context.strokeStyle = lineColor;
                context.beginPath();
                context.moveTo(0, height/2);
                context.lineTo(width, height/2);
                context.stroke();

                context.strokeStyle = serial.cutoff? waveColorCutoff : waveColor;
                context.beginPath();
                context.moveTo(0, height);

                var points = serial.bufferSize;
                var dx = width / points;
                var increment = Math.max(1, Math.round(points/width/4) );

                for(var i = 0; i < points; i+= increment)
                {
                    context.lineTo(i*dx, height * (0.5 - serial.sample(i)) );
                }

                context.stroke();
            }

            Text
            {
                text: "Normalization " + (serial.normalizationEnabled? "On" : "Off")
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                anchors.topMargin: 0
                color: serial.normalizationEnabled? "#6ff" : "#666"

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: serial.normalizationEnabled = !serial.normalizationEnabled
                }
            }
        }

        Rectangle
        {
            color: "#080808"
            width: parent.width
            height: 2
            x: 0
            anchors.top: waveform.bottom
            anchors.topMargin: -1

        }

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
                property bool on: serial.serialPort !== null && serial.state == AudioSerialPort.STATE_OPEN
                property bool error: serial.state == AudioSerialPort.STATE_ERROR
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
    }


}

