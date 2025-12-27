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
                model: [
                    {value:"",label:qsTr("起始地")},
                    {value:"北京",label:qsTr("北京")},
                    {value:"上海",label:qsTr("上海")}
                ]
                onActivated: search_data.departure=currentValue
            }
            HusSelect{
                id:destination
                Layout.preferredWidth: 180
                Layout.maximumWidth: 180
                Layout.fillHeight: true
                clearEnabled: false
                model:[
                    {value:"",label:qsTr("目的地")},
                    {value:"北京",label:qsTr("北京")},
                    {value:"上海",label:qsTr("上海")}
                ]
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

    Item{
        Layout.fillHeight: true
        Layout.fillWidth: true
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

        // var departureText=departure.displayText==="起始地"?"":departure.displayText
        // var destinationText=destination.displayText==="目的地"?"":destination.displayText

        console.log("here");
        console.log(search_data.departure);
        console.log(search_data.destination);
        console.log(search_data.depart_time);
        let flights=DBManager.queryFlightsByCondition(search_data.departure,search_data.destination,search_data.depart_time);
        flightList.clear();
        for(let j=0;j<flights.length;j++)
        {
            console.log(flights[j]["Flight_id"]);
            flightList.append(flights[j]);
        }

    }

}
