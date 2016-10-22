import QtQuick 2.0

Item {
    id: container

    OsgEarth {
        id: earthView
        anchors.fill: parent

        sceneFile: qmlWidget.earthFile
        fieldOfView: 90

        yaw: attitudeActual.Yaw
        pitch: attitudeActual.Pitch
        roll: attitudeActual.Roll

        latitude: qmlWidget.actualPositionUsed ?
                      gpsPosition.Latitude/10000000.0 : qmlWidget.latitude
        longitude: qmlWidget.actualPositionUsed ?
                       gpsPosition.Longitude/10000000.0 : qmlWidget.longitude
        altitude: qmlWidget.actualPositionUsed ?
                      gpsPosition.Altitude : qmlWidget.altitude
    }

    //Display the center line in the same way as with world view
    Item {
        id: world
        anchors.fill: parent

        transform: [
            Translate {
                id: pitchTranslate
                x: Math.round((world.parent.width - world.width)/2)
                y: (world.parent.height - world.height)/2+
                   world.parent.height/2*Math.sin(attitudeActual.Pitch*Math.PI/180)*1.405
            },
            Rotation {
                angle: -attitudeActual.Roll
                origin.x : world.parent.width/2
                origin.y : world.parent.height/2
            }
        ]

        SvgElementImage {
            id: horizont_line
            elementName: "world-centerline"
            //worldView is loaded with Loader, so background element is visible
            sceneSize: background.sceneSize
            anchors.centerIn: parent
            border: 1
            smooth: true
        }
    }
}
