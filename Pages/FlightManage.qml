import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import com.flight.db 1.0
import "../Components"
ColumnLayout{
    Layout.fillHeight: true
    Layout.fillWidth: true
    spacing:10

    property var add_data:{
        "flight_id":"",
        "departure":"",
        "destination":"",
        "depart_time":"",
        "arrive_time":"",
        "price":"",
        "total_seats":3,
        "remain_seats":2,
        "status":2
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

    function validateAddData(data) {
        // 检查所有必需字段是否为空
        if (!data.flight_id || data.flight_id.trim() === "") {
            console.error("航班号不能为空")
            return false
        }

        if (!data.departure || data.departure.trim() === "") {
            console.error("出发地不能为空")
            return false
        }

        if (!data.destination || data.destination.trim() === "") {
            console.error("目的地不能为空")
            return false
        }

        if (!data.depart_time || data.depart_time.trim() === "") {
            console.error("出发时间不能为空")
            return false
        }

        if (!data.arrive_time || data.arrive_time.trim() === "") {
            console.error("到达时间不能为空")
            return false
        }

        if (!data.price || data.price.trim() === "") {
            console.error("价格不能为空")
            return false
        }

        // 对于数字类型，检查是否为0或空
        if (!data.total_seats || data.total_seats === 0) {
            console.error("总座位数不能为空或0")
            return false
        }

        if (data.remain_seats === undefined || data.remain_seats === null) {
            console.error("剩余座位数不能为空")
            return false
        }



        return true
    }

    RowLayout{
        Layout.fillHeight: true
        Layout.fillWidth: true
        spacing:50
        HusButton{
            id:addFlight
            text: "添加航班"
            onClicked: {
                addFlightDialog.open()
            }
        }

        HusButton{
            id:updateButton
            text:"刷新航班"
            onClicked: {
                updateData()
            }
        }
    }

    //添加弹窗
    HusModal{
        id:addFlightDialog
        title:"添加航班"
        width:500

        descriptionDelegate: ColumnLayout{
            spacing:10
            Layout.fillHeight: true
            Layout.fillWidth: true
            Row{
                spacing:10
                HusText{
                    text:"航班号:"
                }

                HusInput{
                    id:flight_idInput
                    width:200
                    Component.onCompleted: {
                        add_data.flight_id=""
                        flight_idInput.text=""
                    }
                    onTextChanged: {
                        add_data.flight_id=flight_idInput.text
                    }
                }

            }

            Row{
                spacing:10
                HusText{
                    text:"出发地:"
                }

                // HusInput{
                //     id:departureInput
                //     width:200
                //     Component.onCompleted: {
                //         add_data.departure=""
                //         departureInput.text=""
                //     }
                //     onTextChanged: {
                //         add_data.departure=departureInput.text
                //     }
                // }
                HusSelect{
                    id:departure
                    Layout.preferredWidth: 180
                    Layout.maximumHeight: 180
                    Layout.fillHeight: true
                    clearEnabled: false
                    model: departureList
                    onActivated: add_data.departure=currentValue
                    Component.onCompleted: {
                        add_data.departure=""
                    }
                }

            }

            Row{
                spacing:10
                HusText{
                    text:"目的地:"
                }

                // HusInput{
                //     id:destinationInput
                //     width:200
                //     Component.onCompleted: {
                //         add_data.destination=""
                //         destinationInput.text=""
                //     }
                //     onTextChanged: {
                //         add_data.destination=destinationInput.text
                //     }
                // }

                HusSelect{
                    id:destination
                    Layout.preferredWidth: 180
                    Layout.maximumWidth: 180
                    Layout.fillHeight: true
                    clearEnabled: false
                    model: destinationList
                    onActivated: add_data.destination=currentValue
                    Component.onCompleted: {
                        add_data.destination=""
                    }
                }
            }

            Row{
                spacing:10
                HusText{
                    text:"始发时间:"
                }

                HusDateTimePicker{
                    id:depart_timeInput
                    Layout.preferredWidth: 280
                    Layout.maximumWidth: 280
                    Layout.fillHeight: true
                    placeholderText: qsTr("请选择始发时间")
                    format: qsTr("yyyy-MM-dd hh:mm:ss")
                    Component.onCompleted: {
                        add_data.depart_time=""
                    }

                    onTextChanged: add_data.depart_time=depart_timeInput.text
                }
            }

            Row{
                spacing:10
                HusText{
                    text:"到达时间:"
                }

                HusDateTimePicker{
                    id:arrive_timeInput
                    Layout.preferredWidth: 280
                    Layout.maximumWidth: 280
                    Layout.fillHeight: true
                    placeholderText: qsTr("请选择到达时间")
                    format: qsTr("yyyy-MM-dd hh:mm:ss")
                    Component.onCompleted: {
                        add_data.arrive_time=""
                    }

                    onTextChanged: add_data.arrive_time=arrive_timeInput.text
                }
            }

            Row{
                spacing:10
                HusText{
                    text:"总座位数:"
                }

                HusInput{
                    id:totalseatInput
                    width:200
                    //console.log(card_data.remain_seats)
                    Component.onCompleted: {
                        add_data.total_seats=""
                        totalseatInput.text=""
                    }
                    onTextChanged: {
                        add_data.total_seats=totalseatInput.text
                    }
                }
            }

            Row{
                spacing:10
                HusText{
                    text:"空座位数:"
                }

                HusInput{
                    id:remainseatInput
                    width:200

                    //console.log(card_data.remain_seats)
                    Component.onCompleted: {
                        add_data.remain_seats=""
                        remainseatInput.text=""
                    }
                    onTextChanged: {
                        add_data.remain_seats=remainseatInput.text
                    }
                }
            }

            Row{
                spacing:10
                HusText{
                    text:"价格:"
                }

                HusInput{
                    id:priceInput
                    width:200

                    Component.onCompleted: {
                        add_data.price=""
                        priceInput.text=""
                    }
                    onTextChanged: {
                        add_data.price=priceInput.text
                    }
                }
            }


        }

        confirmText: "添加"
        cancelText: "取消"
        onConfirm: {
            //非法输出检测
            if(!validateAddData(add_data))
            {
                console.log("非法")
                return ;
            }

            let succ=DBManager.addFlight(add_data.flight_id,add_data.departure,add_data.destination,add_data.depart_time,add_data.arrive_time,add_data.price,add_data.total_seats,add_data.remain_seats)
            if(!succ)
            {
                console.log("添加失败")
                console.log(add_data.flight_id)
                console.log(add_data.departure)
                console.log(add_data.destination)
                console.log(add_data.depart_time)
                console.log(add_data.arrive_time)
                console.log(add_data.price)
                console.log(add_data.total_seats)
                console.log(add_data.remain_seats)
            }

            close()
        }

        onCancel: close()
    }


    //分割线
    HusDivider{
        Layout.fillWidth: true
    }
    //列表数据
    ListModel{
        id:flightList
    }
    ListView{
        Layout.fillHeight: true
        Layout.fillWidth: true
        spacing: 5
        clip:true
        model: flightList

        delegate: FlightManageCard{
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

    //更新数据函数
    function updateData()
    {
        flightList.clear()
        let flights=DBManager.queryAllFlights()
        for(let i=0;i<flights.length;i++)
        {
            flightList.append(flights[i])
        }
    }

    //初始化
    Component.onCompleted: updateData()

    Connections{
        target:DBManager

        function onOperateResult(success,message)
        {
            if(message.includes("航班添加成功！航班号") && success)
            {
                updateData()
                return ;
            }
            if((message.includes("状态更新为")||message.includes("价格更新为")||message.includes("剩余座位更新为"))&&success)
            {
                updateData()
                return ;
            }
            if(message.includes("航班删除成功！")&&success)
            {
                updateData()
                return ;
            }
        }
    }
}
