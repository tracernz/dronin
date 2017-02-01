/**
 ******************************************************************************
 * @file       modelview.qml
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelView Model View Gadget
 * @{
 * @brief Provides a 3D model view
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

import QtQuick 2.0
import QtQuick.Scene3D 2.0
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0
import com.dronin.uavo 1.0

Rectangle {
    id: mainView
    visible: true
    anchors.fill: parent
    color: "gray"

    Scene3D {
        id: scene3d
        objectName: "scene3D"
        anchors.fill: parent
        focus: true
        aspects: "input"

        Entity {
            id: sceneRoot

            Camera {
                id: camera
                projectionType: CameraLens.PerspectiveProjection
                fieldOfView: 90
                aspectRatio: 16/9
                nearPlane: 0.1
                farPlane: 10000
                position: Qt.vector3d(0.0, 3000.0, 3000.0)
                upVector: Qt.vector3d(0.0, 1.0, 0.0)
                viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
            }

            components: [
                RenderSettings {
                    activeFrameGraph: ForwardRenderer {
                        clearColor: Qt.rgba(0, 0.5, 1, 1)
                        camera: camera
                    }
                },
                InputSettings {}
            ]

            MouseHandler {

            }

            Entity {
                components: [
                    SceneLoader {
                        id: loader
                        source: {
                            switch (systemSettings.AirframeType) {
                                case SystemSettings.AIRFRAMETYPE_QUADX:
                                    return "gaui_330x.3ds";
                                case SystemSettings.AIRFRAMETYPE_HEXAX:
                                    return "gemini.3ds";
                                case SystemSettings.AIRFRAMETYPE_HELICP:
                                    return "trex450xl.obj";
                                default:
                                    return "aeroquad_+.3ds";
                            }
                        }
                    },
                    Transform {
                        rotation: fromEulerAngles(-attitudeActual.Pitch, /*-attitudeActual.Yaw*/180.0, attitudeActual.Roll)
                    }
                ]
            }
        }
    }
    Text{
        text: String(loader.source).substring(String(loader.source).length - 20)
        color: 'white'
        font.pointSize: 20
    }
}

/**
 * @}
 * @}
 */
