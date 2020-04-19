/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "https://vodyanoy.000webhostapp.com/user.php";    
var lastLinkTime;

window.onload = function() {          //

  document.getElementById('aPumpChartSwitch').onclick = onPumpChartSwitchClick;  
  document.getElementById('aWarmChartSwitch').onclick = onWarmChartSwitchClick;  
  document.getElementById('aConnectSwitch').onclick = onConnectChartSwitchClick;  
  document.getElementById('aDoorSwitch').onclick = onDoorChartSwitchClick;  
  document.getElementById('aBatterySwitch').onclick = onBatteryChartSwitchClick;  
  
  GetStats();
  ReadChartFromServer();  
  setInterval(oneMoreMinute, 60000); 
} 

function onBatteryChartSwitchClick(){
    if(this.innerText == "Подробнее"){
        this.innerText = "Свернуть";
        document.getElementById('batteryChart').height = 200;
    }
    else{
        this.innerText = "Подробнее";
        document.getElementById('batteryChart').height = 200;
    }
}

function onDoorChartSwitchClick(){
    if(this.innerText == "Подробнее"){
        this.innerText = "Свернуть";
        document.getElementById('doorFloodChart').height = 200;
    }
    else{
        this.innerText = "Подробнее";
        document.getElementById('doorFloodChart').height = 200;
    }
}

function onConnectChartSwitchClick(){
    if(this.innerText == "Подробнее"){
        this.innerText = "Свернуть";
        document.getElementById('connectBalanceChart').height = 200;
    }
    else{
        this.innerText = "Подробнее";
        document.getElementById('connectBalanceChart').height = 200;
    }
}

function onWarmChartSwitchClick(){
    if(this.innerText == "Подробнее"){
        this.innerText = "Свернуть";
        document.getElementById('warmTempChart').height = 200;
    }
    else{
        this.innerText = "Подробнее";
        document.getElementById('warmTempChart').height = 200;
    }
}

function onPumpChartSwitchClick(){
    if(this.innerText == "Подробнее"){
        this.innerText = "Свернуть";
        document.getElementById('pumpSupplyChart').height = 200;
    }
    else{
        this.innerText = "Подробнее";
        document.getElementById('pumpSupplyChart').height = 200;
    }
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
    document.getElementById("balanceStatus").innerText = data.result[0].balance + " руб."; 
    document.getElementById("balanceStatus").style.backgroundColor = RangeToColor(0, 120, "RG", data.result[0].balance);

    var t = data.result[0].temp/16;
    //t = 7;
    document.getElementById("tempStatus").innerText = t + " °C"; 
    if(t <= 15) document.getElementById("tempStatus").style.backgroundColor = RangeToColor(-5, 15, "RG", t);
    else document.getElementById("tempStatus").style.backgroundColor = RangeToColor(15, 50, "GR", t);
    var voltage = data.result[0].Vbat;
    voltage /= 200;
    voltage -= 3.3;
    voltage /= (4.2-3.3)/100;    
    voltage = Math.round(voltage*10)/10;
    document.getElementById("batteryStatus").innerText = voltage + "%";
    
    var b = new Date(data.result[0].timestamp);
    var Duration = (new Date().getTime() - b.getTime()) / 1000;     // Полочим разницу во времени в секундах
    Duration -= 3*60*60;  // Коррекция на локальное время
    var DurationHours = Duration / 3600;            // Время прошедшее с последнего обновления в часах
    var ending = " минут ";
    if(Duration > 60*60*24*30*2){ Duration /= 60*60*24*30; ending = " месяц. ";}
    else if(Duration > 60*60*24*2){ Duration /= 60*60*24; ending = " дня ";}
    else if(Duration > 60*60*3){ Duration /= 60*60; ending = " час. ";}
    else {Duration /= 60;}   
    Duration = Math.round(Duration); 
    document.getElementById("connectStatus").innerText = Duration + ending + "назад"; 
    /*
    if(DurationHours > 12) { Rcolor = 255; Gcolor = 0;}
    if(DurationHours <= 12 && DurationHours > 6)  { Rcolor = 255; Gcolor = 255/6 * (12-DurationHours);}
    if(DurationHours <= 6)                        { Gcolor = 255; Rcolor = 255/6 * DurationHours;}
    */        
    document.getElementById("connectStatus").style.backgroundColor = RangeToColor(0, 6, "GR", DurationHours);
  }  
}

function RangeToColor(min, max, format, value)           // Превращает value в цвет для .style.Color / backgroundColor
{                                                   // В диапазоне от min до max, цвета указываются в строке format "RG", "GR", "RGB", "BGR"
  var Rcolor, Gcolor, Bcolor;
  if(min > max) { var tmp = min; min = max; max = tmp;} 
  if(min < 0){ max += (0 - min); value += (0 - min); min = 0 ;}   //Коррекция для работы в положительных числах и неделения на 0
  if(format == "RG" || format == "GR"){
    if(value > max)                         { Rcolor = 255; Gcolor = 0;}
    if(value <= max && value > (min+max)/2) { Rcolor = 255; Gcolor = (255/((min+max)/2)) * (min+max-value);}
    if(value <= (min+max)/2)                { Gcolor = 255; Rcolor = (255/((min+max)/2)) * value;}
    if(value < min)                         { Gcolor = 255; Rcolor = 0;}
    
    if(format == "GR")
      return "rgba(" + Math.round(Rcolor) + "," + Math.round(Gcolor) + ", 20, 1)";
    else 
      return "rgba(" + Math.round(Gcolor) + "," + Math.round(Rcolor) + ", 20, 1)";     
  }   

  if(format == "RGB" || format == "BGR"){
    if(value > max)                                       { Rcolor = 255; Gcolor = 0; Bcolor = 0;}
    if(value <= max && value > (min+max)*0.75)            { Rcolor = 255; Gcolor = (255/((min+max)/4)) * (min+max-value); Bcolor = 0;}
    if(value <= (min+max)*0.75 && value > (min+max)*0.5)  { Rcolor = (255/((min+max)/4)) * (value-(min+max)*0.5); Gcolor = 255; Bcolor = 0;}
    if(value <= (min+max)*0.5 && value > (min+max)*0.25)  { Rcolor = 0; Gcolor = 255; Bcolor = (255/((min+max)/4)) * ((min+max)*0.5-value);}
    if(value <= (min+max)*0.25 && value > min)            { Rcolor = 0; Gcolor = (255/((min+max)/4)) * value; Bcolor = 255;}
    if(value <= min)                                      { Rcolor = 0; Gcolor = 0; Bcolor = 255;}
    
    if(format == "BGR")
      return "rgba(" + Math.round(Rcolor) + "," + Math.round(Gcolor) + "," + Math.round(Bcolor) + ", 1)";
    else 
      return "rgba(" + Math.round(Gcolor) + "," + Math.round(Rcolor) + "," + Math.round(Bcolor) + ", 1)";
  }   
      
          
  //return "rgba(" + Rcolor + "," + Gcolor + ", 20, 1)";
}

function cbSqlReadChartSuccess(data, textStatus) {      // Прочитаем историю с сервера
  if(data.status == "success"){   
    var newData = [];    
    var i = 0;
    if(data.result.length > 144) i = data.result.length - 144;
    for(; i < data.result.length; i ++){
      var a = {};
      var b = new Date(data.result[i].timestamp);
      b = b.getTime() + 10459000;
      a.time = new Date(b);  
  //    a.time += a.time.getTime() + 3240120;
      a.tt = Number(data.result[i].temp)/16;
      if(data.result[i].Vbat != "0")
        a.Vbat = Number(data.result[i].Vbat)/200;   
      newData.push(a);
      
    } 
    Duration = (new Date().getTime() - newData[0].time.getTime()) / 1000;
    var ending = " минут ";
    if(Duration > 60*60*24*30*2){ Duration /= 60*60*24*30; ending = " месяцев ";}
    else if(Duration > 60*60*24*2){ Duration /= 60*60*24; ending = " дней ";}
    else if(Duration > 60*60*3){ Duration /= 60*60; ending = " часов ";}
    else Duration /= 60;   
    Duration = Math.round(Duration); 
   // document.getElementById("sp_lastConnection").innerText = Duration + ending + "назад";
  
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
  document.getElementById("lastConnection").innerText = "ServerError"  
}

