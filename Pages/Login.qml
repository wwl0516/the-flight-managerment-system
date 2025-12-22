  import QtQuick
import HuskarUI.Basic
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtQuick.Effects
import QtQml
import com.flight.db 1.0


Window {
    id: loginWindow
    objectName: "login_window"
    width: 800
    height: 450
    visible: controlSignal
    title: qsTr("登录系统")

    property var rootContainerRef: null

    minimumWidth: 800
    minimumHeight: 450

    property bool controlSignal:true

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
        height: registerState === dynamicBackground.state ? 420 : 380
        color: "#80FFFFFF"
        //opacity: 0.5
        radius: 12
        border.color: "#e0e0e0"
        border.width: 1

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: -parent.width/4

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

            // 记住密码选项（仅在用户登录时显示）
            // Row {
            //     id: rememberPasswordRow
            //     width: parent.width
            //     spacing: 100
            //     visible: dynamicBackground.state === loginState
            //     leftPadding: 2

            //     CheckBox {
            //         id: rememberPasswordCheck
            //         //自动登录选中时自动选中
            //         checked: autoLoginCheck.checked
            //         indicator.width: 16
            //         indicator.height: 16

            //         contentItem: Text {
            //             text: qsTr("记住密码")
            //             font.pointSize: 9
            //             color: "#666666"
            //             verticalAlignment: Text.AlignVCenter
            //             leftPadding: rememberPasswordCheck.indicator.width + rememberPasswordCheck.spacing
            //         }
            //     }

            //     //自动登录选项（仅在用户登录显示）
            //     CheckBox {
            //         id: autoLoginCheck
            //         checked: false
            //         indicator.width: 16
            //         indicator.height: 16



            //         contentItem: Text {
            //             text: qsTr("自动登录")
            //             font.pointSize: 9
            //             color: "#666666"
            //             verticalAlignment: Text.AlignVCenter
            //             leftPadding: autoLoginCheck.indicator.width + autoLoginCheck.spacing
            //         }
            //     }
            // }







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

                onClicked: {
                    handleAction()
                }
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
                    anchors.horizontalCenterOffset: -parent.width/4
                    height: 380
                }
            },
            State {
                name: registerState
                PropertyChanges {
                    target: textBackground
                    anchors.horizontalCenterOffset: parent.width/4
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
        anchors.centerIn: parent
        width: 400
        dim:true
        closePolicy: Dialog.NoAutoClose
        onAccepted: handleForgetPassword()
        Column {
            width: parent.width
            spacing: 15

            RowLayout{
                Text {
                    text: "请输入注册的邮箱"
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
                placeholderText: "请输入注册的邮箱"
                height: 30
                background: Rectangle {
                    radius: 6
                    border.width: 1
                    border.color: resetEmailInput.activeFocus ? "#007bff" : "#dddddd"
                    color: "white"
                }
            }

            TextField{
                id:captchaInput
                width:parent.width
                placeholderText: "请输入验证码"
                height: 30
                background: Rectangle {
                    radius: 6
                    border.width: 1
                    border.color: captchaInput.activeFocus ? "#007bff" : "#dddddd"
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

    //错误弹窗
    Dialog{
        id:errorDialog
        title: "错误"
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 200
        dim: true
        closePolicy: Dialog.NoAutoClose
        Text{
            id:errorText
            anchors.centerIn: parent
            text:"登陆失败，用户名或密码错误"
            state:"loginError"
            states:[
                State{
                    name:"loginError"
                    PropertyChanges {
                        target: errorText
                        text:"登陆失败，用户名或密码错误"
                    }
                },
                State{
                    name:"passwords_different"
                    PropertyChanges {
                        target: errorText
                        text:"密码不一致"
                    }
                },
                State{
                    name:"cannot_find_username"
                    PropertyChanges {
                        target: errorText
                        text:"用户名不存在"
                    }
                },
                State{
                    name:"too_week_password"
                    PropertyChanges {
                        target: errorText
                        text:"密码至少8位，且包含字母和数字"
                    }
                },
                State{
                    name:"empty_username"
                    PropertyChanges {
                        target: errorText
                        text:"用户名为空"

                    }
                },
                State{
                    name:"empty_password"
                    PropertyChanges {
                        target: errorText
                        text:"密码为空"

                    }
                },
                State{
                    name:"username_already_exists"
                    PropertyChanges {
                        target: errorText
                        text:"注册失败，用户名已存在"

                    }
                },
                State{
                    name:"empty_email"
                    PropertyChanges {
                        target: errorText
                        text:"邮箱为空"

                    }
                },
                State{
                    name:"email_exists"
                    PropertyChanges {
                      target: errorText
                      text:"邮箱已注册"

                    }
                },
                State{
                  name:"email_invalid"
                  PropertyChanges {
                    target: errorText
                    text:"邮箱格式非法"

                    }
                },
              State{
                name:"captcha_error"
                PropertyChanges {
                  target: errorText
                  text:"验证码错误"
                  }
              },
              State{
                name:"captcha_empty"
                PropertyChanges {
                  target: errorText
                  text:"验证码为空"
                  }
              },
              State{
                name:"email_not_exists"
                PropertyChanges {
                  target: errorText
                  text:"邮箱未注册"
                  }
              }

            ]
        }
    }

    Dialog{
        id:messageDialog
        title: "提示"
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 200
        dim: true
        closePolicy: Dialog.NoAutoClose
        Text{
            id:messageText
            anchors.centerIn: parent
            text:"注册成功"
            state: "register_success"
            states:[
                State{
                    name:"register_success"
                    PropertyChanges {
                        target: messageText
                        text:"注册成功"
                    }
                },
              State{
                  name:"password_reset"
                  PropertyChanges {
                      target: messageText
                      text:"密码重制成功"
                  }
              }
            ]
        }
    }

    Loader{
        id:windowLoader

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
        //用户名非空
        if (usernameInput.text === "") {
            errorText.state="empty_username"
            errorDialog.open()
            return;
        }
        //密码非空
        if (passwordInput.text === "") {
            errorText.state="empty_password"
            errorDialog.open()
            return;
        }

        //注册特殊检测
        if(dynamicBackground.state===registerState)
        {
            //邮箱非空
            if(emailInput.text==="")
            {
                errorText.state="empty_email"
                errorDialog.open()
                return;
            }

            //密码要一致
            if(passwordInput.text!==confirmPasswordInput.text)
            {
                errorText.state="passwords_different"
                errorDialog.open()
                return ;
            }
        }

        var success=false
        if(dynamicBackground.state===adminState)
        {
            success= DBManager.verifyAdminLogin(usernameInput.text, passwordInput.text)
        }
        else if(dynamicBackground.state===loginState)
        {
            success=DBManager.userLogin(usernameInput.text, passwordInput.text)
        }
        else
        {
            success=DBManager.userRegister(emailInput.text,usernameInput.text,passwordInput.text)
        }

        //注册处理
        if(dynamicBackground.state===registerState)
        {
            if(success===5)
            {
                messageText.state="register_success"
                messageDialog.open()
                return ;
            }
            else if(success===1)
            {
                errorText.state="username_already_exists"
                errorDialog.open()
                return ;
            }
            else if(success===4)
            {
                errorText.state="email_exists"
                errorDialog.open()
                return ;
            }
            else if(success===3)
            {
              errorText.state="too_week_password"
              errorDialog.open()
              return ;
            }
            else if(success===2)
            {
              errorText.state="email_invalid"
              errorDialog.open()
              return ;
            }
        }

        //用户登录
        success=4
        if(dynamicBackground.state===loginState)
        {
            if (success===4) {
                rootContainerRef.createMainWindow()
                loginWindow.destroy()
                return ;
            }
            else
            {
                errorText.state="loginError"
                errorDialog.open()
                return ;
            }
        }

        //管理员登录
        if (success) {
            //登录到管理员界面
            rootContainerRef.createAdminMainWindow()
            loginWindow.destroy()
        }
        else
        {
            errorText.state="loginError"
            errorDialog.open()
            return ;
        }

        errorText.state="loginError"
        errorDialog.open()
        return ;

    }

    function handleForgetPassword()
    {
      if(resetEmailInput.text==="")
      {
        errorText.state="empty_email"
        errorDialog.open()

        return ;
      }

      if(captchaInput.text==="")
      {
        errorText.state="captcha_empty"
        errorDialog.open()
        return ;
      }
      if(regain_password.text==="")
      {
        errorText.state="empty_password"
        errorDialog.open()
        return ;
      }
      if(regain_confirm_password.text!==regain_password.text)
      {
        errorText.state="passwords_different"
        errorDialog.open()
        return ;
      }
      var code=DBManger.forgetPassword(resetEmailInput.text,captchaInput.text,regain_password.text)
      if(code===1)
      {
        errorText.state="email_invalid"
        errorDialog.open()
        return ;
      }
      else if(code===2)
      {
        errorText.state="email_not_exists"
        errorDialog.open()
        return ;
      }
      else if(code===3)
      {
        errorText.state="captcha_error"
        errorDialog.open()
        return ;
      }
      else if(code===4)
      {
        errorText.state="too_week_password"
        errorDialog.open()
        return ;
      }
      else if(code===7)
      {
        messageText.state="password_reset"
        messageDialog.open()
        return ;
      }

    }






    function clearInputs() {
        usernameInput.text = ""
        passwordInput.text = ""
        emailInput.text = ""
        confirmPasswordInput.text = ""
    }
}
