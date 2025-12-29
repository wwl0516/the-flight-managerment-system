import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import com.flight.globalVars
import com.flight.db 1.0


HusRectangle{
    //数据
    property var card_data:{
        "flight_id":"12345",
        "departure":"北京",
        "destination":"上海",
        "depart_time":"1145141919810",
        "arrive_time":"1145141919810",
        "price":"1145",
        "total_seats":3,
        "remain_seats":2,
        "status":2,
        "order_id":""
    }
    id:flight_information
    width: parent.width
    height: parent.height
    radius:20
    color:HusTheme.darkMode?"#FF1F1F1F":"#FFE0E0E0"
    ColumnLayout{
        height: parent.height
        width: parent.width
        RowLayout{
            Layout.leftMargin: 30
            Layout.rightMargin: 30
            Layout.topMargin: 15
            spacing: 10
            Component{
                id:direction_icon
                RowLayout{
                    width:100
                    height:25
                    //spacing: 2
                    HusText{
                        text: qsTr(card_data.flight_id)
                        font.pixelSize: 17
                        font.bold: true
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter
                    }

                    HusIconText{
                        iconSource: HusIcon.SendOutlined
                        iconSize: 25
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }
            ColumnLayout{
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 5
                HusText{
                    Layout.alignment: Qt.AlignHCenter
                    text:qsTr(card_data.departure)
                    font.pixelSize: 20
                    font.bold: true
                }
                HusText{
                    Layout.alignment: Qt.AlignHCenter
                    text:qsTr(card_data.depart_time)
                    font.pixelSize: 20
                    font.bold: true
                }
            }
            HusDivider{
                titleDelegate: direction_icon
                titleAlign: HusDivider.Align_Center
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }
            ColumnLayout{
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 5
                HusText{
                    Layout.alignment: Qt.AlignHCenter
                    text:qsTr(card_data.destination)
                    font.pixelSize: 20
                    font.bold: true
                }
                HusText{
                    Layout.alignment: Qt.AlignHCenter
                    text:qsTr(card_data.arrive_time)
                    font.pixelSize: 20
                    font.bold: true
                }
            }
        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        RowLayout{
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30
            Layout.bottomMargin: 15
            Layout.leftMargin: 30
            Layout.rightMargin: 30
            spacing: 30
            //剩余座位
            HusRectangle{
                Layout.fillHeight: true
                Layout.preferredWidth: 100
                color: "#FF999999"
                Layout.alignment: Qt.AlignVCenter
                radius: 10
                HusText{
                    anchors.centerIn: parent
                    text: qsTr(String(card_data.remain_seats))
                    font.pixelSize: 17
                    font.bold: true
                }
            }
            //状态
            HusRectangle{
                Layout.fillHeight: true
                Layout.preferredWidth: 100
                color: GlobalSettings.flight_state_color[card_data.status]
                Layout.alignment: Qt.AlignVCenter
                radius: 10
                HusText{
                    anchors.centerIn: parent
                    text: qsTr(GlobalSettings.flight_state[card_data.status])
                    font.pixelSize: 17
                    font.bold: true
                }
            }

            //空白占位符
            Item{
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            //价格&取消购买按钮
            HusButton{
                text: qsTr("取消订单")
                type: HusButton.Type_Primary
                Layout.preferredWidth: 100


                TapHandler{
                    target: parent
                    onTapped: DBManager.deleteOrder(card_data.order_id)
                }
            }
        }
    }
}
