import QtQuick 2.0

Item {
    id: sceneItem
    property variant sceneSize

    //AttitudeActual.Yaw is converted to -180..180 range
    property real yaw: sceneItem.parent.circular_modulus_deg(attitudeActual.Yaw)
    property real pitch : (attitudeActual.Pitch)

    // Telemetry status arrow
    SvgElementImage {
        id: telemetry_status
        elementName: "gcstelemetry-"+statusName
        sceneSize: sceneItem.sceneSize

        property string statusName : ["Disconnected","HandshakeReq","HandshakeAck","Connected"][gcsTelemetryStats.Status]

        // Force refresh of the arrow image when elementName changes
        onElementNameChanged: { generateSource() }

        scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "gcstelemetry-Disconnected")
        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    // Telemetry rate text
    Text {
        id: telemetry_rate
        text: gcsTelemetryStats.TxDataRate.toFixed()+"/"+gcsTelemetryStats.RxDataRate.toFixed()
        color: "white"
        font.family: "Arial"
        font.pixelSize: telemetry_status.height * 0.75

        anchors.top: telemetry_status.bottom
        anchors.horizontalCenter: telemetry_status.horizontalCenter
    }

    // GPS status text
    Text {
        id: gps_text
        text: "GPS: " + gpsPosition.Satellites + "\nPDP: " + gpsPosition.PDOP.toFixed(2) + "\nACC: " + gpsPosition.Accuracy.toFixed(2)
        color: "white"
        font.family: "Arial"
        font.pixelSize: telemetry_status.height * 0.55

        visible: gpsPosition.Satellites

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "gps-txt")
        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    // Battery text
    Text {
        id: battery_text

        text: flightBatteryState.Voltage.toFixed(2)+" V\n" +
              flightBatteryState.Current.toFixed(2)+" A\n" +
              flightBatteryState.ConsumedEnergy.toFixed()+" mAh"

        color: "white"
        font.family: "Arial"

        font.pixelSize: telemetry_status.height * .55

        visible: flightBatteryState.Voltage > 0 || flightBatteryState.Current > 0

        property variant scaledBounds: svgRenderer.scaledElementBounds("pfd.svg", "battery-txt")
        x: Math.floor(scaledBounds.x * sceneItem.width)
        y: Math.floor(scaledBounds.y * sceneItem.height)
    }

    // Draw home location marker
    Item {
        id: homelocation
        anchors.fill: parent

        transform: [
            Translate {
                id: homeLocationTranslate
                x: homelocation.parent.width/2*Math.sin(homewaypoint.bearing_R)*(Math.sin(Math.PI/2)/Math.sin(parent.fovX_D*Math.PI/180/2))
                y: -homelocation.parent.height/2*Math.sin(homewaypoint.elevation_R)*(Math.sin(Math.PI/2)/Math.sin(parent.fovY_D*Math.PI/180/2))
            },
            Rotation {
                angle: -attitudeActual.Roll
                origin.x : homelocation.parent.width/2
                origin.y : homelocation.parent.height/2
            }
        ]

        SvgElementImage {
            id: homewaypoint

            elementName: "homewaypoint"
            sceneSize: sceneItem.sceneSize

            // Home location is only visible if it is set and when it is in front of the viewport
            visible: (homeLocation.Set != 0 && Math.abs(bearing_R) < Math.PI/2)

            property real bearing_R : sceneItem.parent.circular_modulus_rad(Math.atan2(-positionActual.East, -positionActual.North) - yaw*Math.PI/180)
            property real elevation_R : Math.atan(-positionActual.Down / -Math.sqrt(Math.pow(positionActual.North,2)+Math.pow(positionActual.East,2))) - pitch*Math.PI/180

            // Center the home location marker in the middle of the PFD
            anchors.centerIn: parent
        }
    }

    // Draw waypoint marker
    Item {
        id: waypoint
        anchors.fill: parent

        transform: [
            Translate {
                id: waypointTranslate
                x: waypoint.parent.width/2*Math.sin(nextwaypoint.bearing_R)*(Math.sin(Math.PI/2)/Math.sin(parent.fovX_D*Math.PI/180/2))
                y: -waypoint.parent.height/2*Math.sin(nextwaypoint.elevation_R)*(Math.sin(Math.PI/2)/Math.sin(parent.fovY_D*Math.PI/180/2))
            },
            Rotation {
                angle: -attitudeActual.Roll
                origin.x : waypoint.parent.width/2
                origin.y : waypoint.parent.height/2
            }
        ]

        SvgElementImage {
            id: nextwaypoint

            elementName: "nextwaypoint"
            sceneSize: sceneItem.sceneSize

            property int activeWaypoint: waypointActive.Index

            // When the active waypoint changes, load active the waypoint coordinates into the
            // local instance of Waypoint
            onActiveWaypointChanged: qmlWidget.exportUAVOInstance("Waypoint", activeWaypoint)

            // Waypoint is only visible when it is in front of the viewport
            visible: (Math.abs(bearing_R) < Math.PI/2 && (waypoint.Position_North != 0 || waypoint.Position_East != 0 || waypoint.Position_Down != 0))

            property real bearing_R : sceneItem.parent.circular_modulus_rad(Math.atan2(waypoint.Position_East - positionActual.East, waypoint.Position_North - positionActual.North) - yaw*Math.PI/180)
            property real elevation_R : Math.atan((waypoint.Position_Down - positionActual.Down) / -Math.sqrt(Math.pow(waypoint.Position_North - positionActual.North,2)+Math.pow(waypoint.Position_East - positionActual.East,2))) - pitch*Math.PI/180

            // Center the home location marker in the middle of the PFD
            anchors.centerIn: parent
        }
    }

}
