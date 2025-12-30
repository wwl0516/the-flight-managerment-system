import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import com.flight.db 1.0

import "../Components"

ColumnLayout{
    property var search_data:{
        "flight_id":"",
        "departure":"",
        "destination":"",
        "depart_time":""
    }

    property var departureList: [
        {value:"",label:qsTr("起始地")},
        {value:"北京",label:qsTr("北京")},
        {value:"上海",label:qsTr("上海")},
        {value:"广州",label:qsTr("广州")},
        {value:"长沙",label:qsTr("长沙")},
        {value:"深圳",label:qsTr("深圳")}
    ]

    property var destinationList: [
        {value:"",label:qsTr("目的地")},
        {value:"北京",label:qsTr("北京")},
        {value:"上海",label:qsTr("上海")},
        {value:"广州",label:qsTr("广州")},
        {value:"长沙",label:qsTr("长沙")},
        {value:"深圳",label:qsTr("深圳")}
    ]

    // 航班号结果
    ListModel{
        id:flightList
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 10
    ColumnLayout {
        Layout.fillWidth: true
        //Layout.fillHeight: true
        spacing: 30

        RowLayout {
            id: searchRow
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.topMargin: 30
            spacing: 30

            HusInput {
                id: search_my_flight_input
                Layout.preferredWidth: 300
                Layout.maximumWidth: 300
                Layout.fillHeight: true
                radiusBg.all: 5
                placeholderText: "输入航班号"
                onTextChanged: search_data.flight_id=text
            }

            HusIconButton {
                id: search_my_flight_button
                Layout.preferredWidth: 75
                Layout.maximumWidth: 75
                Layout.fillHeight: true
                radiusBg.all: 5
                iconSource: HusIcon.SearchOutlined
                text: qsTr("搜索")
                onClicked: searchFlight()
            }
        }

        // 第二个RowLayout
        RowLayout {
            id: rectangleRow
            Layout.preferredHeight: 30
            Layout.maximumHeight: 30
            Layout.fillWidth: true

            HusSelect{
                id:departure
                Layout.preferredWidth: 180
                Layout.maximumHeight: 180
                Layout.fillHeight: true
                clearEnabled: false
                model: departureList
                onActivated: search_data.departure=currentValue
            }
            HusSelect{
                id:destination
                Layout.preferredWidth: 180
                Layout.maximumWidth: 180
                Layout.fillHeight: true
                clearEnabled: false
                model: destinationList
                onActivated: search_data.destination=currentValue
            }
            HusDateTimePicker{
                id:pick
                Layout.preferredWidth: 280
                Layout.maximumWidth: 280
                Layout.fillHeight: true
                datePickerMode: HusDateTimePicker.Mode_Day
                showTime: false
                placeholderText: qsTr("请选择始发日期")
                format: qsTr("yyyy-MM-dd")
                onTextChanged: search_data.depart_time=text
            }
        }
    }
    HusDivider{
        Layout.fillWidth: true
    }
    ListView{
        Layout.fillHeight: true
        Layout.fillWidth: true
        clip: true
        spacing: 5
        model: flightList

        delegate: FlightInformationCard{
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

    function searchFlight(){
        //优先处理航班号
        if(search_my_flight_input.text!==""){
            let flight =DBManager.queryFlightByNum(search_data.flight_id)
            flightList.clear();
            for(let i=0;i<flight.length;i++)
            {
                console.log(flight[i]["Flight_id"]);
                flightList.append(flight[i]);
            }

            return ;
        }
        //console.log(destination.currentValue)
        let flights=DBManager.queryFlightsByCondition(departure.currentValue,destination.currentValue,pick.text);
        flightList.clear();
        for(let j=0;j<flights.length;j++)
        {
            console.log(flights[j]["Flight_id"]);
            flightList.append(flights[j]);
        }

    }


    Connections{
        target: DBManager

        function onOperateResult(success,message){
            if(message.includes("创建订单成功") && success){
                searchFlight();
            }

        }
    }


}
