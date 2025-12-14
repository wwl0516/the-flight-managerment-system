import QtQuick
import QtQuick.Layouts
import QtQml
import HuskarUI.Basic
import QtQuick.Controls

//主题风格保持与Main.qml一致
HusWindow{
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
                        text: qsTr("<b>名字</b>") //此处需要适配数据库
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
                            implicitHeight: 195
                            implicitWidth: parent.width
                        }
                    }
                    initModel: [
                    {
                        key:"flight_manage",
                        label:qsTr("航班管理"),
                        iconSource:HusIcon.GlobalOutlined
                    },
                    {
                        key:"my_favourite_flight",
                        label:qsTr("收藏航班"),
                        iconSource:HusIcon.StarOutlined
                    },
                    {
                        key:"search_flight",
                        label:qsTr("航班查询"),
                        iconSource:HusIcon.SearchOutlined
                    },
                    {
                        key:"my_flight",
                        label:qsTr("我的订单"),
                        iconSource:HusIcon.ShopOutlined
                    },
                    {
                        key:"share_trip",
                        label:qsTr("旅程分享"),
                        iconSource:HusIcon.SmileOutlined
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

                        console.log("那就战个未来吧");
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
