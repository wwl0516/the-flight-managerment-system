import QtQuick
import HuskarUI.Basic
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtQuick.Effects
import QtQml
import "Main.qml"
import "Login.qml"

ApplicationWindow{
    id:controlWindow
    visible:false

    Loader{
        id:windowLoader
        source:"Login.qml"
    }
}
