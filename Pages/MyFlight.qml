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

        delegate: MyFlightCard{
            required property var modelData
            height: 150
            card_data: {
                "flight_id":modelData.flight_id,
                "departure":modelData.departure,
                "destination":modelData.destination,
                "depart_time":modelData.depart_time,
                "arrive_time":modelData.arrive_time,
                "price":modelData.price,
                "total_seats":modelData.total_seats,
                "remain_seats":modelData.remain_seats,
                "status":modelData.f_status,
                "order_id":modelData.order_id
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
            if(message.includes("删除订单成功") && success){
                get_order_flights();
            }
        }
    }

    Component.onCompleted: {
        get_order_flights();
    }

    function get_order_flights(){
        var flights = DBManager.queryMyOrders(DBManager.getCurrentUserId());
        flightList.clear();
        for(let i=0;i<flights.length;++i){
            flightList.append(flights[i]);
        }
    }

}
