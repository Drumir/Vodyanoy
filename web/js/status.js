/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "http://drumir.16mb.com/k/user.php";    
var lastLinkTime;

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
  if(data.result.length > 144) i = data.result.length - 144;
  for(; i < data.result.length; i ++){
    var a = {};
    var b = new Date(data.result[i].time);
    b = b.getTime() + 10459000;
    a.time = new Date(b);  
//    a.time += a.time.getTime() + 3240120;
    a.tt = Number(data.result[i].tt)/16;
    if(data.result[i].Vbat != "0")
      a.Vbat = Number(data.result[i].Vbat)/200;   
    newData.push(a);
    
  } 
    Duration = (new Date().getTime() - a.time.getTime()) / 1000;
    var ending = " минут ";
    if(Duration > 60*60*24*30*2){ Duration /= 60*60*24*30; ending = " месяцев ";}
    if(Duration > 60*60*24*2){ Duration /= 60*60*24; ending = " дней ";}
    if(Duration > 60*60*3){ Duration /= 60*60; ending = " часов ";}
    else Duration /= 60;   
    Duration = Math.round(Duration); 
    document.getElementById("lastConnection").innerText = "Последнее обновление: " + Duration + ending + "назад";
  
     /*   
  for(i = 0; i < data.result.length; i ++){
    data.result[i].time = new Date(data.result[i].time); 
    data.result[i].tt = Number(data.result[i].tt)/16;    
  }  */ 
  
    $('#chart').dxChart({
      dataSource: newData,  
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
        { valueField: "tt", 
          name: "Температура", 
          type: "spline", 
          axis: "температура",
        },
        { valueField: "Vbat", 
          name: "Аккумулятор", 
          type: "spline", 
          axis: "напряжение",
        },
      ],    
      valueAxis: [{
          name: "температура",
          position: "left",  
          label: {
            customizeText: function () {
            return this.valueText + '&#176C';
            }
          }

//          showZero: true,
//          tickInterval: 1,
      }, 
      {
          name: "напряжение",
          position: "right",
          min: 3,
          max: 4.3,
          tickInterval: 0.1,   
          maxValueMargin: 0.1,
          label: {
            customizeText: function () {
            return this.valueText + ' В';
            }
          }
      }],
      
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

