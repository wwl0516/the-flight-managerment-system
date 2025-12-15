import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import com.flight.globalVars 1.0
import com.flight.db 1.0


ColumnLayout{
    Layout.fillHeight: true
    Layout.fillWidth: true
    spacing: 10

    RowLayout{
        Layout.fillWidth: true
        Layout.maximumHeight: 30
        Layout.topMargin: 120
        spacing: 5
        HusText{
            text: qsTr("深色模式")
            font.pixelSize: 20
            Layout.fillHeight: true
        }
        HusSwitch{
            id:dark_mode_switch
            Layout.fillHeight: true
            checked: GlobalSettings.is_dark_mode
            onCheckedChanged:{
                GlobalSettings.is_dark_mode=checked
                HusTheme.darkMode=checked?HusTheme.Dark:HusTheme.Light
            }
        }
    }

    HusButton{
        Layout.preferredHeight: 40
        Layout.preferredWidth: 120
        type: HusButton.Type_Outlined
        colorBorder: "red"
        colorBg: "#80ff0000"
        colorText: HusTheme.isDark?"white":"black"
        text: qsTr("退出登录")
        onClicked:{
            DBManager.userLogout()
        }
    }
    Item {
        Layout.fillHeight: true
    }
}
