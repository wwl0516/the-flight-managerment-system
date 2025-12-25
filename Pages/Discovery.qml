import QtQuick
import HuskarUI.Basic
import QtQuick.Layouts

ColumnLayout{

    Layout.fillHeight: true
    Layout.fillWidth: true
    Loader{
        id:shared_card
        Layout.alignment: Qt.AlignHCenter
        Layout.fillHeight: true
        source: "../Components/SharedCard.qml"
    }
    RowLayout{
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        HusIconButton{
            id:last_page
            text: qsTr("上一页")
            iconSource: HusIcon.LeftOutlined
            iconSize: 10
        }
        HusIconButton{
            id:next_page
            text: qsTr("下一页")
            iconSource: HusIcon.RightOutlined
            iconSize: 10
        }
    }
}
