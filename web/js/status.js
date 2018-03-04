/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "http://drumir.16mb.com/k/user.php";

window.onload = function() {          //

//  document.getElementById('saveSettings').onclick = onBtnSaveClick;  
  
  ReadStatusFromServer(); 
  setInterval(oneMoreMinute, 60000); 
}  

function oneMoreMinute(){
  ReadStatusFromServer();  
}

function ReadStatusFromServer(){
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  {action:"readChart"},
    success: cbSqlReadStatusSuccess,
    error: cb16mbError
  });
}

function cbSqlReadStatusSuccess(data, textStatus) {      // Прочитаем настройки с сервера
  if(data.status == "success"){   
  var newData = [];    
  var i = 0;
  if(data.result.length > 120) i = data.result.length - 120;
  for(; i < data.result.length; i ++){
    var a = {};
    var b = new Date(data.result[i].time);
    b = b.getTime() + 10459000;
    a.time = new Date(b);  
//    a.time += a.time.getTime() + 3240120;
    a.tt = Number(data.result[i].tt)/16;    
    newData.push(a);
    
  }  /*   
  for(i = 0; i < data.result.length; i ++){
    data.result[i].time = new Date(data.result[i].time); 
    data.result[i].tt = Number(data.result[i].tt)/16;    
  }  */ 
  
    $('#chart').dxChart({
      dataSource: newData,  
//      dataSource: data.result,  
      zoomingMode: "mouse",
      
      
      commonSeriesSettings: {
          type: "line",
          argumentField: "time",
          line: {
              point: {
                  visible: true
              }
          }
      },    
      
      series: [
        { valueField: "tt", name: "Температура" },
      ],    
      
      argumentAxis:{
          grid:{
              visible: true
          }
      },
      tooltip:{
          enabled: true
      },
      legend: {
          verticalAlignment: "bottom",
          horizontalAlignment: "center"
      },
      commonPaneSettings: {
          border:{
              visible: true,
              right: false
          }       
      },
      animation: {
        enabled: false
      }
    });
  }  
}

function cb16mbError(){
}
