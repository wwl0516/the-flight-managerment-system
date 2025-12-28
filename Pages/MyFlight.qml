import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic


ColumnLayout{
    //订单直接全部展示
    function handle()
    {
        let orders=DBManager.queryMyOrders(DBManager.getCurrentUserId())
    }
}
