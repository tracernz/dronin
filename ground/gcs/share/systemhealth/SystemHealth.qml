import QtQuick 2.0
import QtWebKit 3.0
import com.dronin.uavo 1.0

Rectangle {
    color: "#666666"

    property SystemAlarmsClass sysAlarms: SystemAlarms

    property var alarms: [
        {
            name: "OutOfMemory",
            text: "Out Of Memory",
            value: sysAlarms.Alarm_OutOfMemory
        },
        {
            name: "CPUOverload",
            text: "CPU Overload",
            value: sysAlarms.Alarm_CPUOverload
        },
        {
            name: "StackOverflow",
            text: "Stack Overflow",
            value: sysAlarms.Alarm_StackOverflow
        },
        {
            name: "SystemConfiguration",
            text: "System Configuration",
            value: sysAlarms.Alarm_SystemConfiguration
        },
        {
            name: "EventSystem",
            text: "Event System",
            value: sysAlarms.Alarm_EventSystem
        },
        {
            name: "Telemetry",
            text: "Telemetry",
            value: sysAlarms.Alarm_Telemetry
        },
        {
            name: "ManualControl",
            text: "RC Input",
            value: sysAlarms.Alarm_ManualControl
        },
        {
            name: "Actuator",
            text: "Servo Output",
            value: sysAlarms.Alarm_Actuator
        },
        {
            name: "Attitude",
            text: "Attitude",
            value: sysAlarms.Alarm_Attitude
        },
        {
            name: "Sensors",
            text: "Sensors",
            value: sysAlarms.Alarm_Sensors
        },
        {
            name: "Geofence",
            text: "Geofence",
            value: sysAlarms.Alarm_Geofence
        },
        {
            name: "PathFollower",
            text: "Path Follower",
            value: sysAlarms.Alarm_PathFollower
        },
        {
            name: "PathPlanner",
            text: "Path Planner",
            value: sysAlarms.Alarm_PathPlanner
        },
        {
            name: "Battery",
            text: "Battery",
            value: sysAlarms.Alarm_Battery
        },
        {
            name: "FlightTime",
            text: "Flight Time",
            value: sysAlarms.Alarm_FlightTime
        },
        {
            name: "I2C",
            text: "I2C",
            value: sysAlarms.Alarm_I2C
        },
        {
            name: "GPS",
            text: "GPS",
            value: sysAlarms.Alarm_GPS
        },
        {
            name: "AltitudeHold",
            text: "Altitude Hold",
            value: sysAlarms.Alarm_AltitudeHold
        },
        {
            name: "BootFault",
            text: "Boot Fault",
            value: sysAlarms.Alarm_BootFault
        },
        {
            name: "TempBaro",
            text: "Baro Temp.",
            value: sysAlarms.Alarm_TempBaro
        },
        {
            name: "GyroBias",
            text: "Gyro Bias",
            value: sysAlarms.Alarm_GyroBias
        },
        {
            name: "ADC",
            text: "ADC",
            value: sysAlarms.Alarm_ADC
        }
    ]

    property var alarmStates: [ ]

    Component.onCompleted: {
        alarmStates[SystemAlarmsClass.ALARM_UNINITIALISED] = "Uninitialised";
        alarmStates[SystemAlarmsClass.ALARM_OK] = "OK";
        alarmStates[SystemAlarmsClass.ALARM_WARNING] = "Warning";
        alarmStates[SystemAlarmsClass.ALARM_ERROR] = "Error";
        alarmStates[SystemAlarmsClass.ALARM_CRITICAL] = "Critical";
    }

    Item {
        anchors.fill: parent

        Grid {
            id: alarmGrid
            columns: 4
            spacing: 10
            anchors.centerIn: parent
            Repeater {
                model: alarms
                Rectangle {
                    height: 60
                    width: 150
                    border.width: 3
                    border.color: "#ffffff"
                    radius: 3
                    color: ["#808080", "#04b629", "#f1b907", "#332d2d", "#cf0e0e"][modelData.value]
                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 10
                        height: parent.height - 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                        color: "#ffffff"
                        font.pixelSize: 20
                        font.bold: true
                        text: modelData.text
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                             if (!description.visible) {
                                var request = new XMLHttpRequest()
                                request.open("GET", "html/" + modelData.name + "-" + alarmStates[modelData.value] +".html")
                                request.onreadystatechange = function(event) {
                                    if (request.readyState == XMLHttpRequest.DONE) {
                                        description.text = request.responseText
                                        description.visible = true;
                                    } else {
                                        description.text = "oh shit";
                                    }
                                }
                                request.send()
                            } else {
                                description.visible = false;
                            }
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
           
        }
    }

    Rectangle {
        id: description
        width: 2 * alarmGrid.width / 3
        height: 2 * alarmGrid.height / 3
        anchors.centerIn: parent
        visible: false
        property string text
        
        Text {
            anchors.centerIn: parent
            width: parent.width - 10
            height: parent.height - 10
            text: parent.text
        }
    }
}
