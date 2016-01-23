import QtQuick 2.0
import com.dronin.uavo 1.0

Text {
	property int isVisible
	property int align
	property int fontSize
	property int posX
	property int posY

	color: "white"
	font.family: "Arial"

	font.pixelSize: childScale * ((fontSize == OnScreenDisplayPageSettingsClass.BATTERYCURRENTFONT_SMALL) ? 8 : (fontSize == OnScreenDisplayPageSettingsClass.BATTERYCURRENTFONT_LARGE) ? 12 : 10)
	visible: isVisible == OnScreenDisplayPageSettingsClass.BATTERYVOLT_ENABLED

	x: posX * childScale - ((align == OnScreenDisplayPageSettingsClass.BATTERYCURRENTALIGN_CENTER) ? this.width / 2 : (align == OnScreenDisplayPageSettingsClass.BATTERYCURRENTALIGN_RIGHT) ? this.width : 0)
	y: posY * childScale
}
