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
      var eventType = "";
      var t;
      for(var i = 0; i < data.result.length; i ++) {
        switch(Number(data.result[i].eventCode)){
            case 0:{ eventStr = "Старт насоса автоматически (по расписанию или после сбоя питания)"; break;}
            case 1:{ eventStr = "Старт насоса вручную"; break;}
            case 2:{ eventStr = "Старт насоса удаленно"; break;}
            case 3:{ eventStr = "Отключение насоса по расписанию"; break;}
            case 4:{ eventStr = "Отключение насоса вручную"; break;}
            case 5:{ eventStr = "Отключение насоса удаленно"; break;}
            case 6:{ eventStr = "Аварийное отключение насоса"; break;}
            case 7:{ eventStr = "Отказ запуска насоса: Нет расписания"; break;}
            case 8:{ eventStr = "Отказ запуска насоса: Возможна заморозка"; break;}
            case 9:{ eventStr = "Отказ запуска насоса: Нет питания (380В)"; break;}

            case 10:{ eventStr = "Автоматическое включение обогревателя "; break;}
            case 11:{ eventStr = "Автоматическое отключение обогревателя"; break;}
            case 12:{ eventStr = "Ручное включение обогревателя"; break;}
            case 13:{ eventStr = "Ручное отключение обогревателя"; break;}
            case 14:{ eventStr = "Удаленное включение обогревателя"; break;}
            case 15:{ eventStr = "Удаленное отключение обогревателя"; break;}
            case 16:{ eventStr = "Аварийное отключение обогревателя"; break;}
            
            case 20:{ eventStr = "Отключение электропитания (380В)"; break;}
            case 21:{ eventStr = "Возобновление электропитания (380В)"; break;}

            case 30:{ eventStr = "Затопление помещения"; break;}
            case 31:{ eventStr = "Осушение помещения"; break;}

            case 40:{ eventStr = "Открытие входной двери"; break;}
            case 41:{ eventStr = "Закрытие входной двери"; break;}

            case 50:{ eventStr = "Низкий баланс"; break;}
            case 51:{ eventStr = "Модуль не может зарегистрироваться в GSM сети (нет сигнала?)"; break;} 
            case 52:{ eventStr = "Модуль не может включить GPRS (отключен интернет?)"; break;}
            case 53:{ eventStr = "Модуль не может подключиться к серверу (проблема  с сервером?)"; break;}
            case 54:{ eventStr = "С сервера получены новые настройки водяного"; break;} 

            case 60:{ eventStr = "Провалена проверка аккумулятора"; break;}
            case 61:{ eventStr = "Вручную (локально) заданы новые настройки водяного"; break;}
            case 62:{ eventStr = "Возможно заморозка"; break;}
            case 63:{ eventStr = "Перегрев"; break;}
            case 64:{ eventStr = "Переполнение приёмного буфера RX"; break;}
            case 65:{ eventStr = "Переполнение истории"; break;}
            case 66:{ eventStr = "Включение водяного"; break;}
            case 0xFF: { eventStr = "Пустое событие"; break;}
            default:{ eventStr = "Событие №" + data.result[i].eventCode;}
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

