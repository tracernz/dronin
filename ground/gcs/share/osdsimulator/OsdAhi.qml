import QtQuick 2.0
import com.dronin.uavo 1.0

Canvas {
    property real roll
    property real pitch
    property int maxPitch
    property int pitchSteps
    property int sizeWidth
    property int sizeHeight

    function redraw() {
        var CentreBody = 3 * childScale;
        var CentreWing = 7 * childScale;
        var CentreRudder = 5 * childScale;
        var PitchStep = maxPitch / pitchSteps;
        var ctx = getContext('2d');

        ctx.strokeStyle = "#ffffff";
        ctx.lineWidth = 1 * childScale;
        ctx.beginPath();

        var dX = sizeWidth / 2;

        // main horizon
        ctx.moveTo(width / 2 - dX, height / 2);
        ctx.lineTo(width / 2 - dX / 3, height / 2);

        ctx.moveTo(width / 2 + dX / 3, height / 2);
        ctx.lineTo(width / 2 + dX, height / 2);

        // 10 deg steps
        var dY_10 = height / 2 * PitchStep / maxPitch;

        for (var i = 1; i <= pitchSteps; i++) {
            //sprintf(tmp_str, "%d", i * PITCH_STEP);

            // top
            // horizontal line
            ctx.moveTo(width / 2 - 3 * dX / 4, height / 2 - dY_10 * i);
            ctx.lineTo(width / 2 + 3 * dX / 4, height / 2 - dY_10 * i);
            // left cap
            ctx.moveTo(width / 2 - 3 * dX / 4, height / 2 - dY_10 * i);
            ctx.lineTo(width / 2 - 3 * dX / 4, height / 2 - dY_10 * i + dY_10 / 6);
            // right cap
            ctx.moveTo(width / 2 + 3 * dX / 4, height / 2 - dY_10 * i);
            ctx.lineTo(width / 2 + 3 * dX / 4, height / 2 - dY_10 * i + dY_10 / 6);

            ctx.text((i * PitchStep).toFixed(0), width / 2 - dX - 16, height / 2 - dY_10 * i);
            ctx.text((i * PitchStep).toFixed(0), width / 2 + dX + 4, height / 2 - dY_10 * i);

            // bottom
            // horizontal line, TODO: dashed
            ctx.moveTo(width / 2 - 3 * dX / 4, height / 2 + dY_10 * i);
            ctx.lineTo(width / 2 + 3 * dX / 4, height / 2 + dY_10 * i);
            // left cap
            ctx.moveTo(width / 2 - 3 * dX / 4, height / 2 + dY_10 * i);
            ctx.lineTo(width / 2 - 3 * dX / 4, height / 2 + dY_10 * i - dY_10 / 6);
            // right cap
            ctx.moveTo(width / 2 + 3 * dX / 4, height / 2 + dY_10 * i);
            ctx.lineTo(width / 2 + 3 * dX / 4, height / 2 + dY_10 * i - dY_10 / 6);

            ctx.text((i * PitchStep).toFixed(0), width / 2 - dX - 16, height / 2 + dY_10 * i);
            ctx.text((i * PitchStep).toFixed(0), width / 2 + dX + 4, height / 2 + dY_10 * i);
        }

        ctx.stroke();
    }

    onPaint: {
        redraw();
    }

    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: redraw()
    }

    Binding {
        target: AttitudeActual
        property: "Roll"
        value: parent.roll
    }
}
