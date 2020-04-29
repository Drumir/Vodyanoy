/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "https://vodyanoy.000webhostapp.com/user.php";  

window.onload = function() {          //

//  document.getElementById('aBatterySwitch').onclick = onBatteryChartSwitchClick;  
  
  GetHistory();
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

function oneMoreMinute(){
  GetHistory();
}


function GetHistory(){
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  {action:"getHistory"},
    success: cbSqlGetHistorySuccess,
    error: cb16mbError
  });
}

function cbSqlGetHistorySuccess(data, textStatus) {      // Прочитаем историю с сервера
  if(data.status == "success"){   
      var innerHtml = "";
      var eventStr = "";
      var t;
      for(var i = 0; i < data.result.length; i ++) {
        switch(Number(data.result[i].eventCode)){
            case 0:{ eventStr = "Пустое событие"; break;}
            case 1:{ eventStr = "Провалена проверка аккумулятора"; break;}
            case 2:{ eventStr = "Модуль не может зарегистрироваться в GSM сети (нет сигнала?)"; break;} 
            case 3:{ eventStr = "Модуль не может включить GPRS (отключен интернет?)"; break;}
            case 4:{ eventStr = "Модуль не может подключиться к серверу (проблема  с сервером?)"; break;}
            case 5:{ eventStr = "Вручную (локально) заданы новые настройки водяного"; break;}
            case 6:{ eventStr = "Удаленно (через интернет) заданы новые настройки водяного"; break;} 
            case 7:{ eventStr = "Старт насоса автоматически (по расписанию или после сбоя AC)"; break;}
            case 8:{ eventStr = "Старт насоса вручную"; break;}
            case 9:{ eventStr = "Старт насоса удаленно"; break;}
            case 10:{ eventStr = "Отключение насоса по расписанию"; break;}
            case 11:{ eventStr = "Отключение насоса вручную"; break;}
            case 12:{ eventStr = "Отключение насоса удаленно"; break;}
            case 13:{ eventStr = "Аварийное отключение насоса"; break;}
            case 15:{ eventStr = "Автоматическое включение обогревателя "; break;}
            case 16:{ eventStr = "Ручное включение обогревателя"; break;}
            case 17:{ eventStr = "Удаленное включение обогревателя"; break;}
            case 18:{ eventStr = "Автоматическое отключение обогревателя"; break;}
            case 19:{ eventStr = "Ручное отключение обогревателя"; break;}
            case 20:{ eventStr = "Аварийное отключение обогревателя"; break;}
            case 21:{ eventStr = "Открытие входной двери"; break;}
            case 22:{ eventStr = "Закрытие входной двери"; break;}
            case 23:{ eventStr = "Отключение электропитания (380В)"; break;}
            case 24:{ eventStr = "Возобновление электропитания (380В)"; break;}
            case 25:{ eventStr = "Возможно заморозка"; break;}
            case 26:{ eventStr = "Перегрев"; break;}
            case 27:{ eventStr = "Переполнение приёмного буфера RX"; break;}
            case 28:{ eventStr = "Переполнение истории"; break;}
            case 29:{ eventStr = "Включение водяного"; break;}
            case 32:{ eventStr = "Другое событие"; break;}
            default:{ eventStr = "Неизвестное событие";}
        }
        t = new Date(data.result[i].timestamp).getTime();
        t += 3*3600*1000;
        t = new Date(t);
        innerHtml += "<tr><td>" + t.toLocaleString('en-GB') + "</td><td>Остальное</td><td>" + eventStr + "</td></tr>";
      }
      var a = document.getElementById("historyTbody");
      a.innerHTML = innerHtml;
  }
}


function cb16mbError(){
 // document.getElementById("lastConnection").innerText = "ServerError"  
}

