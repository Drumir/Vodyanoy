/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "https://vodyanoy.000webhostapp.com/user.php";    
var lastLinkTime;

window.onload = function() {          //

//  document.getElementById('saveSettings').onclick = onBtnSaveClick;  
  
  GetStats();
  ReadChartFromServer();  
  setInterval(oneMoreMinute, 60000); 
} 

function oneMoreMinute(){
  GetStats();
  ReadChartFromServer();  
}

function ReadChartFromServer(){
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  {action:"readChart"},
    success: cbSqlReadChartSuccess,
    error: cb16mbError
  });
}  

function GetStats(){
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  {action:"getStats"},
    success: cbSqlGetStatsSuccess,
    error: cb16mbError
  });
}

function cbSqlGetStatsSuccess(data, textStatus) {      // Прочитаем статус с сервера
  if(data.status == "success"){   
    
    document.getElementById("p_balance").innerText = data.result[0].balance + " руб.";
    var voltage = data.result[0].Vbat;
    voltage /= 200;
    voltage -= 3.3;
    voltage /= (4.2-3.3)/100;    
    voltage = Math.round(voltage*10)/10;
    document.getElementById("p_vbat").innerText = voltage + "%";
     /*   
  for(i = 0; i < data.result.length; i ++){
    data.result[i].time = new Date(data.result[i].time); 
    data.result[i].tt = Number(data.result[i].tt)/16;    
  }  */ 
  
  }  
}

function cbSqlReadChartSuccess(data, textStatus) {      // Прочитаем настройки с сервера
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
    Duration = (new Date().getTime() - newData[0].time.getTime()) / 1000;
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
          point: {
            size: 3
          },
          type: "line",
          argumentField: "time",
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
          showZero: true,
//          tickInterval: 1,
          label: {
            customizeText: function () {
            return this.valueText + '&#176C';
            }
          }

      }, 
      {
          name: "напряжение",
          position: "right",
          min: 3.2,
          max: 4.1,
//          tickInterval: 0.1,   
//          maxValueMargin: 0.1,
          label: {
            customizeText: function () {
            return this.valueText + ' В';
            }
          }
      }],
      
      argumentAxis:{
          grid:{
              visible: true
          },
//          label: {
//            format: "shortDate"
//          }

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
  document.getElementById("lastConnection").innerText = "cb16mbError"  
}

