import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

ColumnLayout{
    Layout.fillWidth: true
    Layout.fillHeight: true

    RowLayout{
        height: 80
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.topMargin: -400
        spacing: 30
        /* 团队成员头像 */
        HusAvatar{
            size:120
            imageSource: "qrc:/new/images/avatar_Amiya1145.jpg"
        }
        HusAvatar{
            size:120
            imageSource: ""
        }
        HusAvatar{
            size:120
            imageSource: ""
        }
        HusAvatar{
            size: 120
            imageSource: ""
        }
    }
}
