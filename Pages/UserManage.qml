import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import com.flight.db 1.0

import "../Components"


ColumnLayout{
    spacing:10
    fillHeight: true
    fillWidth: true
    ListModel{
        id:userList
    }

    ListView{
        Layout.fillHeight: true
        Layout.fillWidth: true
        spacing: 5
        model: userList

        delegate: UserCard{
            required property var modelData
            height: 150
            user_data: {
                "uid":modelData.uid,
                "username":modelData.username,
                "email":modelData.email,
                "phone":modelData.phone,
                "idcard":modelData.idcard
            }
        }
    }
}
