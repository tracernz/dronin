import QtQuick 2.0
import com.dronin.uavo 1.0
import "convertint8.js" as CInt8

Rectangle {
    color: "#666666"

    property real scaleX: this.width / 352
    property real scaleY: this.height / 240
    property real childScale: Math.min(this.scaleX, this.scaleY)

    property OnScreenDisplayPageSettingsClass pageSettings: OnScreenDisplayPageSettings

    Rectangle {
        color: "#0000ff"
        width: 352 * childScale
        height: 240 * childScale
        clip: true
        anchors.centerIn: parent

        Item {
            id: screenOverlay
            anchors.fill: parent
            visible: CInt8.ConvertInt8(OnScreenDisplaySettings.OSDEnabled) == OnScreenDisplaySettingsClass.OSDENABLED_ENABLED

            OsdTextItem {
                id: batteryVolt
                isVisible: CInt8.ConvertInt8(pageSettings.BatteryVolt)
                align: CInt8.ConvertInt8(pageSettings.BatteryVoltAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.BatteryVoltFont)
                posX: pageSettings.BatteryVoltPosX
                posY: pageSettings.BatteryVoltPosY
                text: FlightBatteryState.Voltage.toFixed(1) + "V"
            }

            OsdTextItem {
                id: batteryCurrent
                isVisible: CInt8.ConvertInt8(pageSettings.BatteryCurrent)
                align: CInt8.ConvertInt8(pageSettings.BatteryCurrentAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.BatteryCurrentFont)
                posX: pageSettings.BatteryCurrentPosX
                posY: pageSettings.BatteryCurrentPosY
                text: FlightBatteryState.Current.toFixed(1) + "A"
            }

            OsdTextItem {
                id: batteryConsumed
                isVisible: CInt8.ConvertInt8(pageSettings.BatteryConsumed)
                align: CInt8.ConvertInt8(pageSettings.BatteryConsumedAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.BatteryConsumedFont)
                posX: pageSettings.BatteryConsumedPosX
                posY: pageSettings.BatteryConsumedPosY
                text: FlightBatteryState.ConsumedEnergy.toFixed(0) + "mAh"
            }

            OsdTextItem {
                id: rssi
                isVisible: CInt8.ConvertInt8(pageSettings.Rssi)
                fontSize: CInt8.ConvertInt8(pageSettings.RssiFont)
                posX: pageSettings.RssiPosX + (rssiIcon.width - 4) * childScale
                posY: pageSettings.RssiPosY
                text: ManualControlCommand.Rssi.toFixed(0).slice(-3)

                Image {
                    id: rssiIcon
                    source: "images/rssi.png"
                    scale: childScale
                    x: - rssiIcon.width * childScale
                    anchors.verticalCenter: rssi.verticalCenter
                    visible: CInt8.ConvertInt8(pageSettings.RssiShowIcon) != 0
                }
            }

            OsdTextItem {
                id: cpu
                isVisible: CInt8.ConvertInt8(pageSettings.Cpu)
                align: CInt8.ConvertInt8(pageSettings.CpuAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.CpuFont)
                posX: pageSettings.CpuPosX
                posY: pageSettings.CpuPosY
                text: "CPU:" + CInt8.ConvertInt8(SystemStats.CPULoad).toFixed(0).slice(-2)
            }

            OsdTextItem {
                id: customText
                isVisible: CInt8.ConvertInt8(pageSettings.CustomText)
                align: CInt8.ConvertInt8(pageSettings.CustomTextAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.CustomTextFont)
                posX: pageSettings.CustomTextPosX
                posY: pageSettings.CustomTextPosY
                text: String.fromCharCode(
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_0),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_1),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_2),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_3),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_4),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_5),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_6),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_7),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_8),
                    CInt8.ConvertInt8(OnScreenDisplaySettings.CustomText_9))
            }

            OsdTextItem {
                id: gForce
                isVisible: CInt8.ConvertInt8(pageSettings.GForce)
                align: CInt8.ConvertInt8(pageSettings.GForceAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.GForceFont)
                posX: pageSettings.GForcePosX
                posY: pageSettings.GForcePosY
                text: (Math.sqrt(Math.pow(Accels.x, 2) + Math.pow(Accels.y, 2) + Math.pow(Accels.z, 2))/9.81).toFixed(1) + "G"
            }

            OsdTextItem {
                id: time
                isVisible: CInt8.ConvertInt8(pageSettings.Time)
                align: CInt8.ConvertInt8(pageSettings.TimeAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.TimeFont)
                posX: pageSettings.TimePosX
                posY: pageSettings.TimePosY
                text: (("0" + (SystemStats.FlightTime / 3600000).toFixed(0)).slice(-2) + ":" + 
                    ("0" + (SystemStats.FlightTime / 60000 % 60).toFixed(0)).slice(-2) + ":" +
                    ("0" + (SystemStats.FlightTime / 1000 % 60).toFixed(0)).slice(-2))
            }

            OsdTextItem {
                id: armStatus
                isVisible: CInt8.ConvertInt8(pageSettings.ArmStatus)
                align: CInt8.ConvertInt8(pageSettings.ArmStatusAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.ArmStatusFont)
                posX: pageSettings.ArmStatusPosX
                posY: pageSettings.ArmStatusPosY
                text: CInt8.ConvertInt8(FlightStatus.Armed) != FlightStatusClass.ARMED_DISARMED ? "ARMED" : ""
            }

            OsdTextItem {
                id: flightMode
                isVisible: CInt8.ConvertInt8(pageSettings.FlightMode)
                align: CInt8.ConvertInt8(pageSettings.FlightModeAlign)
                fontSize: CInt8.ConvertInt8(pageSettings.FlightModeFont)
                posX: pageSettings.FlightModePosX
                posY: pageSettings.FlightModePosY
                text: {
                    switch (CInt8.ConvertInt8(FlightStatus.FlightMode)) {
                    case FlightStatusClass.FLIGHTMODE_MANUAL:
                        return "MAN";
                    case FlightStatusClass.FLIGHTMODE_ACRO:
                        return "ACRO";
                    case FlightStatusClass.FLIGHTMODE_ACROPLUS:
                        return "ACRO PLUS";
                    case FlightStatusClass.FLIGHTMODE_LEVELING:
                        return "LEVEL";
                    case FlightStatusClass.FLIGHTMODE_MWRATE:
                        return "MWRTE";
                    case FlightStatusClass.FLIGHTMODE_HORIZON:
                        return "HOR";
                    case FlightStatusClass.FLIGHTMODE_AXISLOCK:
                        return "ALCK";
                    case FlightStatusClass.FLIGHTMODE_VIRTUALBAR:
                        return "VBAR";
                    case FlightStatusClass.FLIGHTMODE_STABILIZED1:
                        return "ST1";
                    case FlightStatusClass.FLIGHTMODE_STABILIZED2:
                        return "ST2";
                    case FlightStatusClass.FLIGHTMODE_STABILIZED3:
                        return "ST3";
                    case FlightStatusClass.FLIGHTMODE_AUTOTUNE:
                        return "TUNE";
                    case FlightStatusClass.FLIGHTMODE_ALTITUDEHOLD:
                        return "AHLD";
                    case FlightStatusClass.FLIGHTMODE_POSITIONHOLD:
                        return "PHLD";
                    case FlightStatusClass.FLIGHTMODE_RETURNTOHOME:
                        return "RTH";
                    case FlightStatusClass.FLIGHTMODE_PATHPLANNER:
                        return "PLAN";
                    case FlightStatusClass.FLIGHTMODE_TABLETCONTROL:
                        switch (CInt8.ConvertInt8(TabletInfo.TabletModeDesired)) {
                            case TabletInfo.TABLETMODEDESIRED_POSITIONHOLD:
                                return "TAB PH";
                            case TabletInfo.TABLETMODEDESIRED_RETURNTOHOME:
                                return "TAB RTH";
                            case TabletInfo.TABLETMODEDESIRED_RETURNTOTABLET:
                                return "TAB RTT";
                            case TabletInfo.TABLETMODEDESIRED_PATHPLANNER:
                                return "TAB Path";
                            case TabletInfo.TABLETMODEDESIRED_FOLLOWME:
                                return "TAB FollowMe";
                            case TabletInfo.TABLETMODEDESIRED_LAND:
                                return "TAB Land";
                            case TabletInfo.TABLETMODEDESIRED_CAMERAPOI:
                                return "TAB POI";
                            default:
                                return "ERROR";
                        }
                    default:
                        "ERROR";
                    }
                }
            }

            Canvas {
                id: centreMark
                anchors.fill: parent
                onPaint: {
                    var CentreBody = 3 * childScale;
                    var CentreWing = 7 * childScale;
                    var CentreRudder = 5 * childScale;
                    var ctx = this.getContext('2d');
                    ctx.strokeStyle = "#ffffff";
                    ctx.lineWidth = 1 * childScale;
                    ctx.beginPath();
                    ctx.moveTo(this.width / 2 - CentreBody - CentreWing , this.height / 2);
                    ctx.lineTo(this.width / 2 - CentreBody, this.height / 2);
                    ctx.moveTo(this.width / 2 + 1 + CentreBody , this.height / 2);
                    ctx.lineTo(this.width / 2 + 1 + CentreBody + CentreWing, this.height / 2);
                    ctx.moveTo(this.width / 2 , this.height / 2 - CentreRudder - CentreBody);
                    ctx.lineTo(this.width / 2, this.height / 2 - CentreBody);
                    ctx.stroke();
                }
                visible: CInt8.ConvertInt8(pageSettings.CenterMark)
            }

            OsdAhi {
                id: ahi
                anchors.fill: parent
                sizeWidth: 150 * childScale
                sizeHeight: 150 * childScale
                roll: AttitudeActual.Roll
                pitch: AttitudeActual.Pitch
                maxPitch: CInt8.ConvertInt8(pageSettings.ArtificialHorizonMaxPitch)
                pitchSteps: CInt8.ConvertInt8(pageSettings.ArtificialHorizonPitchSteps)
                visible: CInt8.ConvertInt8(pageSettings.ArtificialHorizon)
                transform: [
                    Rotation {
                        origin.x: width / 2
                        origin.y: height/ 4
                        angle: AttitudeActual.Roll
                    },
                    Translate {
                        y: height / 4 * AttitudeActual.Pitch / CInt8.ConvertInt8(pageSettings.ArtificialHorizonMaxPitch)
                    }
                ]
            }
        }
    }
}
