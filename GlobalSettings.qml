pragma Singleton
import QtQuick 2.15

QtObject{
    property bool is_dark_mode:false

    enum FlightState{
        ON_TIME,
        DELAYED,
        CANCEL
    }

    readonly property var flight_state_color: [
        "#FF00BF00",
        "#FFAA56FF",
        "#FFDD0000"
    ]

    readonly property var flight_state: [
        "准 点",
        "延 误",
        "取 消"
    ]

}
