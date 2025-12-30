import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import "../Components"

ColumnLayout{
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 10

    property var search_data: {
        "flight_id":"",
        "departure":"",
        "destination":"",
        "depart_time":""
    }

    // 航班号结果
    ListModel{
        id:flightList
    }

    ListView{
        Layout.fillHeight: true
        Layout.fillWidth: true
        clip: true
        Layout.topMargin: 30
        spacing: 5
        model: flightList

        delegate: MyFavouriteFlightCard{
            required property var modelData
            height: 150
            card_data: {
                "flight_id":modelData.Flight_id,
                "departure":modelData.Departure,
                "destination":modelData.Destination,
                "depart_time":modelData.depart_time,
                "arrive_time":modelData.arrive_time,
                "price":modelData.price,
                "total_seats":modelData.total_seats,
                "remain_seats":modelData.remain_seats,
                "status":modelData.status
            }
        }
    }


    Item{
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    Connections{
        target: DBManager

        function onOperateResult(success,message){
            if((message.includes("取消收藏成功") || message.includes("创建订单成功")) && success){
                get_favourite_flights();
            }
        }
    }

    Component.onCompleted: {
        get_favourite_flights();
    }

    function get_favourite_flights(){
        var flights = DBManager.queryCollectedFlights(DBManager.getCurrentUserId());
        flightList.clear();
        for(let i=0;i<flights.length;++i){
            flightList.append(flights[i]);
        }
    }

}
