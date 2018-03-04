/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "http://drumir.16mb.com/k/user.php";

window.onload = function() {          //

  document.getElementById('saveSettings').onclick = onBtnSaveClick;  
  
  ReadSettingsFromServer();
}

function onBtnSaveClick(){
  SendSettingsOnServer();
}

function cbSqlWriteValueSuccess(){
  ReadSettingsFromServer();
} 

function ReadSettingsFromServer(){
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  {action:"readSettings"},
    success: cbSqlReadSettingsSuccess,
    error: cb16mbError
  });
}

function cbSqlReadSettingsSuccess(data, textStatus) {      // Прочитаем настройки с сервера
  if(data.status == "success"){
    document.getElementById('PumpWorkTime').valueAsNumber = data.result[0].pump_work*60000;
    document.getElementById('PumpIdleTime').valueAsNumber = data.result[0].pump_idle*60000;
    document.getElementById('WarmOnT').value = data.result[0].min_temp;
    document.getElementById('WarmOffT').value = data.result[0].max_temp;
    document.getElementById('ConnectPeriod').value = data.result[0].report_interval; 

    document.getElementById('FreezeT').value = data.result[0].cold_warning_temp;
    var bitmask = data.result[0].cold_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsFreeze').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsFreeze').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallFreeze').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallFreeze').checked = true; }

    document.getElementById('WarmT').value = data.result[0].warm_warning_temp;
    bitmask = data.result[0].warm_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsWarm').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsWarm').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallWarm').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallWarm').checked = true; }

    bitmask = data.result[0].door_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsDoor').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsDoor').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallDoor').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallDoor').checked = true; }

    bitmask = data.result[0].flooding_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsFlood').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsFlood').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallFlood').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallFlood').checked = true; }

    bitmask = data.result[0].power_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsPwrFail').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsPwrFail').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallPwrFail').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallPwrFail').checked = true; }

    bitmask = data.result[0].power_rest_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsPwrRest').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsPwrRest').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallPwrRest').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallPwrRest').checked = true; }

    document.getElementById('DisconnectionTime').value = data.result[0].offline_warning_duration;
    bitmask = data.result[0].offline_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsNoConn').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsNoConn').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallNoConn').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallNoConn').checked = true; }

    document.getElementById('Balance').value = data.result[0].balance_warning_summ;
    bitmask = data.result[0].balance_warning;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsBalans').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsBalans').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallBalans').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallBalans').checked = true; }

    document.getElementById('DailyReport').valueAsNumber = data.result[0].daily_report_time*60000;
    bitmask = data.result[0].daily_report;    
    if((bitmask & 0b00001000) != 0){ document.getElementById('OpSmsDailyReport').checked = true; }
    if((bitmask & 0b00000100) != 0){ document.getElementById('AdmSmsDailyReport').checked = true; }
    if((bitmask & 0b00000010) != 0){ document.getElementById('OpCallDailyReport').checked = true; }
    if((bitmask & 0b00000001) != 0){ document.getElementById('AdmCallDailyReport').checked = true; }

    document.getElementById('OperatorTel').value = data.result[0].operator_number;
    document.getElementById('AdminTel').value = data.result[0].admin_number;
  }  
}

function SendSettingsOnServer() {      // Сохраним ВСЕ настройки на сервере
  var params = {}; 
  var bitmask;
  
  params.pump_work = document.getElementById('PumpWorkTime').valueAsNumber/60000;
  params.pump_idle = document.getElementById('PumpIdleTime').valueAsNumber/60000;
  params.min_temp = document.getElementById('WarmOnT').value;
  params.max_temp = document.getElementById('WarmOffT').value;
  params.report_interval = document.getElementById('ConnectPeriod').value; 

  params.cold_warning_temp = document.getElementById('FreezeT').value;
  bitmask = 0;    
  if(document.getElementById('OpSmsFreeze').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsFreeze').checked == true) bitmask += 4;
  if(document.getElementById('OpCallFreeze').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallFreeze').checked == true) bitmask += 1;
  params.cold_warning = bitmask;

  params.warm_warning_temp = document.getElementById('WarmT').value;
  bitmask = 0;    
  if(document.getElementById('OpSmsWarm').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsWarm').checked == true) bitmask += 4;
  if(document.getElementById('OpCallWarm').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallWarm').checked == true) bitmask += 1;
  params.warm_warning = bitmask;    

  bitmask = 0;    
  if(document.getElementById('OpSmsDoor').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsDoor').checked == true) bitmask += 4;
  if(document.getElementById('OpCallDoor').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallDoor').checked == true) bitmask += 1;
  params.door_warning = bitmask;    

  bitmask = 0;    
  if(document.getElementById('OpSmsFlood').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsFlood').checked == true) bitmask += 4;
  if(document.getElementById('OpCallFlood').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallFlood').checked == true) bitmask += 1;
  params.flooding_warning = bitmask;    

  bitmask = 0;    
  if(document.getElementById('OpSmsPwrFail').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsPwrFail').checked == true) bitmask += 4;
  if(document.getElementById('OpCallPwrFail').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallPwrFail').checked == true) bitmask += 1;
  params.power_warning = bitmask;    

  bitmask = 0;    
  if(document.getElementById('OpSmsPwrRest').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsPwrRest').checked == true) bitmask += 4;
  if(document.getElementById('OpCallPwrRest').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallPwrRest').checked == true) bitmask += 1;
  params.power_rest_warning = bitmask;    

  params.offline_warning_duration = document.getElementById('DisconnectionTime').value;
  bitmask = 0;    
  if(document.getElementById('OpSmsNoConn').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsNoConn').checked == true) bitmask += 4;
  if(document.getElementById('OpCallNoConn').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallNoConn').checked == true) bitmask += 1;
  params.offline_warning = bitmask;    

  params.balance_warning_summ = document.getElementById('Balance').value;
  bitmask = 0;    
  if(document.getElementById('OpSmsBalans').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsBalans').checked == true) bitmask += 4;
  if(document.getElementById('OpCallBalans').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallBalans').checked == true) bitmask += 1;
  params.balance_warning = bitmask;    

  params.daily_report_time = document.getElementById('DailyReport').valueAsNumber/60000;
  bitmask = 0;    
  if(document.getElementById('OpSmsDailyReport').checked == true) bitmask += 8;
  if(document.getElementById('AdmSmsDailyReport').checked == true) bitmask += 4;
  if(document.getElementById('OpCallDailyReport').checked == true) bitmask += 2;
  if(document.getElementById('AdmCallDailyReport').checked == true) bitmask += 1;
  params.daily_report = bitmask;    

  params.operator_number = document.getElementById('OperatorTel').value;
  params.admin_number = document.getElementById('AdminTel').value;  
  
  params.action = "writeSettings";
  
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  params,
    success: cbSqlWriteValueSuccess,
    error: cb16mbError
  });

}

function cb16mbError(){
}
