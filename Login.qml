import QtQuick
import HuskarUI.Basic
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQml

Window {
    id: loginWindow
    width: 800
    height: 450
    visible: true
    title: qsTr("登录系统")


    minimumWidth: 800
    minimumHeight: 450


    // 定义状态枚举
    readonly property string loginState: "login"
    readonly property string registerState: "register"
    readonly property string adminState: "admin_login"

    // 动态背景面板
    Rectangle {
        id: dynamicBackground
        width: parent.width/2
        height: parent.height
        z: 1

        gradient: Gradient {
            GradientStop { position: 0.0; color: dynamicBackground.color }
            GradientStop { position: 1.0; color: Qt.darker(dynamicBackground.color, 1.2) }
        }

        state: loginState

        // 欢迎文本
        Column {
            id: welcomeText
            anchors.centerIn: parent
            spacing: 20

            Text {
                id: announcement
                text: qsTr("欢迎回来")
                font.pointSize: 28
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: subText
                text: qsTr("让我们继续前行")
                font.pointSize: 16
                color: "white"
                opacity: 0.9
                horizontalAlignment: Text.AlignHCenter
            }
        }


        states: [
            State {
                name: loginState
                PropertyChanges {
                    target: dynamicBackground
                    color: "#FF6B35"
                    x: parent.width/2
                    width: parent.width/2
                    z: 1
                }
                PropertyChanges {
                    target: announcement
                    text: qsTr("欢迎回来")
                }
                PropertyChanges {
                    target: subText
                    text: qsTr("让我们继续前行")
                }
                PropertyChanges {
                    target: welcomeText
                    visible: true
                }

            },
            State {
                name: registerState
                PropertyChanges {
                    target: dynamicBackground
                    color: "#2E8B57"
                    x: 0
                    width: parent.width/2
                    z: 1
                }
                PropertyChanges {
                    target: announcement
                    text: qsTr("初次见面")
                }
                PropertyChanges {
                    target: subText
                    text: qsTr("让我们一起出发")
                }
                PropertyChanges {
                    target: welcomeText
                    visible: true
                }

            },
            State {
                name: adminState
                PropertyChanges {
                    target: dynamicBackground
                    color: "#17a2b8"
                    x: 0
                    width: parent.width
                    z: -1
                }
                PropertyChanges {
                    target: welcomeText
                    visible: false
                }
                PropertyChanges {
                    target: adminTitle
                    visible: true
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation {
                properties: "x, width, color, z"
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }
    }

    // 表单半透明背板
    Rectangle {
        id: textBackground
        width: 280
        height: registerState === dynamicBackground.state ? 420 : 350
        color: "#80FFFFFF"
        //opacity: 0.5
        radius: 12
        border.color: "#e0e0e0"
        border.width: 1

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: -200

        state: loginState

        // 阴影效果
        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#40000000"
            shadowBlur: 0.5
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 2
        }

        // 表单输入界面
        Column {
            id: inputForm
            anchors.centerIn: parent
            spacing: 20
            width: parent.width - 40

            // 标题
            Text {
                id: formTitle
                text: {
                    if (dynamicBackground.state === loginState) return "用户登录"
                    if (dynamicBackground.state === registerState) return "用户注册"
                    return "管理员登录"
                }
                font.pointSize: 15
                font.bold: true
                color: "#333333"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 用户名
            Column {
                width: parent.width
                spacing: 5

                Text {
                    text: qsTr("用户名")
                    font.pointSize: 10
                    color: "#666666"
                }

                TextField {
                    id: usernameInput
                    width: parent.width
                    placeholderText: qsTr("请输入用户名")
                    font.pointSize: 11
                    height: 30
                    background: Rectangle {
                        radius: 8
                        border.width: 1
                        border.color: usernameInput.activeFocus ? "#007bff" : "#dddddd"
                        color: "white"
                    }
                }
            }

            // 邮箱（仅在注册时显示）
            Column {
                id: emailColumn
                width: parent.width
                spacing: 5
                visible: dynamicBackground.state === registerState

                Text {
                    text: qsTr("邮箱")
                    font.pointSize: 10
                    color: "#666666"
                }

                TextField {
                    id: emailInput
                    width: parent.width
                    placeholderText: qsTr("请输入邮箱地址")
                    font.pointSize: 11
                    height: 30
                    background: Rectangle {
                        radius: 8
                        border.width: 1
                        border.color: emailInput.activeFocus ? "#007bff" : "#dddddd"
                        color: "white"
                    }
                }
            }

            // 密码
            Column {
                width: parent.width
                spacing: 5

                Text {
                    text: qsTr("密码")
                    font.pointSize: 10
                    color: "#666666"
                }

                TextField {
                    id: passwordInput
                    width: parent.width
                    placeholderText: qsTr("请输入密码")
                    font.pointSize: 11
                    height: 30
                    echoMode: TextInput.Password
                    background: Rectangle {
                        radius: 8
                        border.width: 1
                        border.color: passwordInput.activeFocus ? "#007bff" : "#dddddd"
                        color: "white"
                    }
                }
            }

            // 确认密码（仅在注册时显示）
            Column {
                id: confirmPasswordColumn
                width: parent.width
                spacing: 5
                visible: dynamicBackground.state === registerState

                Text {
                    text: qsTr("确认密码")
                    font.pointSize: 10
                    color: "#666666"
                }

                TextField {
                    id: confirmPasswordInput
                    width: parent.width
                    placeholderText: qsTr("请再次输入密码")
                    font.pointSize: 11
                    height: 30
                    echoMode: TextInput.Password
                    background: Rectangle {
                        radius: 8
                        border.width: 1
                        border.color: confirmPasswordInput.activeFocus ? "#007bff" : "#dddddd"
                        color: "white"
                    }
                }
            }

            // 操作按钮
            Button {
                id: actionButton
                width: parent.width
                height: 45
                text: {
                    if (dynamicBackground.state === registerState) return "注册"
                    if (dynamicBackground.state === adminState) return "管理员登录"
                    return "登录"
                }
                font.pointSize: 14
                font.bold: true

                background: Rectangle {
                    radius: 8
                    color: actionButton.down ? Qt.darker(actionButton.hovered ? "#0056b3" : "#007bff", 1.2) :
                           actionButton.hovered ? "#0056b3" : "#007bff"
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                contentItem: Text {
                    text: actionButton.text
                    font: actionButton.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: handleAction()
            }

            // 底部链接
            RowLayout {
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    Layout.alignment: Qt.AlignLeft
                    text: dynamicBackground.state === loginState ? "注册账号" : "返回登录"
                    flat: true
                    visible:dynamicBackground.state===adminState?false:true
                    font.pointSize: 9
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#007bff"
                        horizontalAlignment: Text.AlignLeft
                    }
                    background: Item {}

                    onClicked: toggleForm()
                }

                Button {
                    id: adminButton
                    Layout.alignment: Qt.AlignRight
                    text: dynamicBackground.state === adminState ? "返回用户登录" : "管理员登录"
                    flat: true
                    font.pointSize: 9
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "Black"
                        horizontalAlignment: Text.AlignRight
                    }
                    background: Item {}

                    onClicked: toggleAdmin()
                }
            }

            // 忘记密码链接（仅在登录状态显示）
            Button {
                id: forgetPassword
                width: parent.width
                text: "忘记密码？"
                flat: true
                font.pointSize: 9
                visible: dynamicBackground.state === loginState||dynamicBackground.state===adminState
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "Black"
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Item {}

                onClicked: forgotPasswordDialog.open()
            }
        }

        states: [
            State {
                name: loginState
                PropertyChanges {
                    target: textBackground
                    anchors.horizontalCenterOffset: -200
                    height: 350
                }
            },
            State {
                name: registerState
                PropertyChanges {
                    target: textBackground
                    anchors.horizontalCenterOffset: 200
                    height: 420
                }
            },
            State {
                name: adminState
                PropertyChanges {
                    target: textBackground
                    anchors.horizontalCenterOffset: 0
                    height: 350
                }
            }
        ]

        transitions: [
            Transition {
                PropertyAnimation {
                    properties: "anchors.horizontalCenterOffset, height"
                    duration: 400
                    easing.type: Easing.InOutQuad
                }
            }
        ]
    }

    // 背景图片
    Rectangle {
        id: loginPicture
        x: 0
        z: -2
        width: parent.width/2
        height: parent.height
        Image {
            id: loginImage
            source: "qrc:/new/images/login.png"
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            visible: false
        }
        MultiEffect {
            id: loginEffect
            anchors.fill: parent
            source: loginImage
            blurEnabled: true
            blur: 1.0
            saturation: -0.1
            brightness: -0.1
        }
    }

    Rectangle {
        id: registerPicture
        x: parent.width/2
        z: -2
        width: parent.width/2
        height: parent.height
        Image {
            id: registerImage
            source: "qrc:/new/images/register.png"
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            visible: false
        }
        MultiEffect {
            id: registerEffect
            anchors.fill: parent
            source: registerImage
            blurEnabled: true
            blur: 1.0
            saturation: -0.1
            brightness: -0.1
        }
    }

    // 忘记密码弹窗
    Dialog {
        id: forgotPasswordDialog
        title: "找回密码"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: Overlay.overlay
        width: 400

        Column {
            width: parent.width
            spacing: 15

            RowLayout{
                Text {
                    text: "请输入用户名"
                    wrapMode: Text.WordWrap
                    width: parent.width
                    color: "#333333"
                }

                Button{
                    text:"发送验证码"
                    font.pointSize: 9
                    flat:true
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "Blue"
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }

            TextField {
                id: resetEmailInput
                width: parent.width
                placeholderText: "请输入用户名"
                height: 30
                background: Rectangle {
                    radius: 6
                    border.width: 1
                    border.color: resetEmailInput.activeFocus ? "#007bff" : "#dddddd"
                    color: "white"
                }
            }

            TextField{
                id:regain_password
                width:parent.width
                placeholderText: "请输入新密码"
                height:30
                background: Rectangle {
                    radius: 6
                    border.width: 1
                    border.color: regain_password.activeFocus ? "#007bff" : "#dddddd"
                    color: "white"
                }

            }

            TextField{
                id:regain_confirm_password
                width:parent.width
                placeholderText: "请确认新密码"
                height:30
                background: Rectangle {
                    radius: 6
                    border.width: 1
                    border.color: regain_confirm_password.activeFocus ? "#007bff" : "#dddddd"
                    color: "white"
                }
            }

        }


    }

    // 状态切换函数
    function toggleForm() {
        if (dynamicBackground.state === loginState) {
            dynamicBackground.state = registerState
            textBackground.state = registerState
        } else if (dynamicBackground.state === registerState) {
            dynamicBackground.state = loginState
            textBackground.state = loginState
        }
        clearInputs()
    }

    function toggleAdmin() {
        if (dynamicBackground.state === adminState) {
            dynamicBackground.state = loginState
            textBackground.state = loginState
        } else {
            dynamicBackground.state = adminState
            textBackground.state = adminState
        }
        clearInputs()
    }

    function handleAction() {
        switch(dynamicBackground.state) {
            case loginState:
                if (validateLogin()) {
                    console.log("用户登录:", usernameInput.text)
                    // 这里添加登录逻辑
                }
                break
            case registerState:
                if (validateRegister()) {
                    console.log("用户注册:", usernameInput.text, "邮箱:", emailInput.text)
                    // 这里添加注册逻辑
                }
                break
            case adminState:
                if (validateLogin()) {
                    console.log("管理员登录:", usernameInput.text)
                    // 这里添加管理员登录逻辑
                }
                break
        }
    }

    function validateLogin() {
        if (usernameInput.text.trim() === "") {
            showError("请输入用户名")
            return false
        }
        if (passwordInput.text === "") {
            showError("请输入密码")
            return false
        }
        return true
    }

    function validateRegister() {
        if (usernameInput.text.trim() === "") {
            showError("请输入用户名")
            return false
        }
        if (emailInput.text.trim() === "") {
            showError("请输入邮箱")
            return false
        }
        if (!isValidEmail(emailInput.text)) {
            showError("请输入有效的邮箱地址")
            return false
        }
        if (passwordInput.text === "") {
            showError("请输入密码")
            return false
        }
        if (passwordInput.text.length < 6) {
            showError("密码长度至少6位")
            return false
        }
        if (passwordInput.text !== confirmPasswordInput.text) {
            showError("两次输入的密码不一致")
            return false
        }
        return true
    }

    function isValidEmail(email) {
        var emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
        return emailRegex.test(email)
    }

    function showError(message) {
        // 这里可以添加错误提示UI
        console.log("错误:", message)
    }

    function clearInputs() {
        usernameInput.text = ""
        passwordInput.text = ""
        emailInput.text = ""
        confirmPasswordInput.text = ""
    }
}
