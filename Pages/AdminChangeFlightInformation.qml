import QtQuick
import QtQuick.Layouts
import QtQml
import HuskarUI.Basic
import QtQuick.Controls


HusWindow{

    //根窗口引用
    property var rootContainerRef: null

    property var initModel :[]
    id:mainWindow
    width: 1080
    height: 720
    maximumWidth: 1080
    maximumHeight: 720
    minimumWidth: 1080
    minimumHeight: 720
    /* 首页左右布局分栏，左侧为功能选项按钮，右侧为功能显示界面 */
    RowLayout{
        anchors.fill: parent
        anchors.margins: 10
        anchors.top: parent.top
        anchors.topMargin: 30
        spacing: 10
        /* 首页左侧一栏，包含用户头像及各种功能选项 */
        ColumnLayout{
            Layout.preferredWidth: 318
            Layout.maximumWidth: 318
            //头像及简易个人信息按钮，点击可以跳转到个人中心界面
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                HusRectangle{
                    anchors.fill: parent
                    radius: 30
                    color: hoverHandler.hovered ? "#FF999999" : "transparent"
                    border.width: 0

                    HoverHandler{
                        id:hoverHandler
                        target: parent
                    }
                }
                RowLayout{
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 10
                    spacing: 15
                    //头像
                    HusAvatar{
                        size: 80
                        imageSource: ''
                        imageMipmap: true
                    }
                    //名字
                    HusCopyableText{
                        textFormat: Text.RichText
                        text: qsTr(DBManager.getCurrentAdminName()) //此处需要适配数据库
                    }
                }
            }
            //主页选择菜单
            Item{
                Layout.fillWidth: true
                Layout.fillHeight: true
                HusMenu{
                    id:menu
                    anchors.fill: parent
                    showEdge: true
                    Component{
                        id: abc
                        Item{
                            implicitHeight: 345
                            implicitWidth: parent.width
                        }
                    }
                    initModel: [
                    {
                        key:"flightManager",
                        label:qsTr("修改航班信息"),
                        iconSource:HusIcon.GlobalOutlined
                    },
                    {
                        key:"orderManager",
                        label:qsTr("修改订单信息"),
                        iconSource:HusIcon.StarOutlined
                    },
                    {
                        key:"shareManager",
                        label:qsTr("修改分享信息"),
                        iconSource:HusIcon.StarOutlined
                    },
                    {
                        enabled: false,
                        contentDelegate: abc
                    },
                    {
                        key:"my_settings",
                        label:qsTr("设置"),
                        iconSource:HusIcon.SettingOutlined
                    },
                    {
                        key:"about_us",
                        label:qsTr("关于我们"),
                        iconSource:HusIcon.TeamOutlined
                    }

                    ]
                    onClickMenu:function(deep,key,key_path,data){
                        if(key==="my_favourite_flight"){
                            right_page.source="MyFavouriteFlight.qml"
                        }
                        if(key==="about_us"){
                            right_page.source="AboutUs.qml"
                        }
                        if(key==="discover"){
                            right_page.source="Discovery.qml"
                        }
                        if(key==="search_flight"){
                            right_page.source="SearchFlight.qml"
                        }

                        console.log("jimi");
                    }
                }
            }
        }
        Loader{
            id:right_page
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
