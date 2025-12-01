import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

ColumnLayout{
    Layout.fillWidth: true
    Layout.fillHeight: true
    RowLayout{
        height: 50
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        Layout.topMargin: 30
        spacing: 30
        HusInput{
            id:search_my_flight_input
            Layout.preferredWidth: 300
            Layout.maximumWidth: 300
            height: parent.height
            radiusBg.all: 5
        }
        HusIconButton{
            id:search_my_flight_button
            Layout.preferredWidth: 75
            Layout.maximumWidth: 75
            height: parent.height
            radiusBg.all: 5
            iconSource: HusIcon.SearchOutlined
            text: qsTr("搜索")
        }
    }
}
