/*-----------------------------------------------------------------------------+
|  Project: Управления водяным 2.0
|  Copyright (c) 2018 drumir@mail.ru
|  All rights reserved.
+-----------------------------------------------------------------------------*/
var sqlServerAdress = "https://vodyanoy.000webhostapp.com/user.php";  
var historyLength = 100;
var resultFromServer;

window.onload = function() {          //

  $('.checkbox').each(function(index, value) { // Всем элементам с классом checkbox Установить обработчик onclick = ShowHistory
    value.onclick = ShowHistory; 
  });   
  
   
  $('#showMore')[0].onclick = onShowMoreClick; 
  GetHistory();
  setInterval(oneMoreMinute, 60000); 
} 

function oneMoreMinute(){
  $("#backgroundPopup").fadeOut("fast"); 
  GetHistory();
}
function onShowMoreClick(){
  historyLength += 100;
  $("#backgroundPopup").fadeIn("fast");
  GetHistory();
}

function GetHistory(){
  $.ajax({
    url: sqlServerAdress,
    type: 'post',
    dataType: 'json',
    data:  {action:"getHistory", length:historyLength},
    success: cbSqlGetHistorySuccess,
    error: cb16mbError
  });
}

function cbSqlGetHistorySuccess(data, textStatus) {      // Прочитаем историю с сервера
  if(data.status == "success"){  
    resultFromServer = data.result; 
    ShowHistory();
    $("#backgroundPopup").fadeOut("fast"); 
  }
}

function ShowHistory() {      // Прочитаем историю с сервера
  if(resultFromServer.length < historyLength)  // Спрячем надпись "далее" если выведена уже вся имеющаяся история
    $('#showMore')[0].hidden = true;
  var innerHtml = "";
  var eventStr = "";
  var eventType = "";
  var t;
  for(var i = 0; i < resultFromServer.length; i ++) {
    switch(Number(resultFromServer[i].eventCode)){
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
        default:{ eventStr = "Событие №" + resultFromServer[i].eventCode;}
    }  
    switch(Math.floor(Number(resultFromServer[i].eventCode)/10))
    {
      case 0: {eventType = "Насос"; if($('#pump')[0].checked === false) continue; break;}
      case 1: {eventType = "Обогреватель"; if($('#warm')[0].checked === false) continue; break;}
      case 2: {eventType = "Электропитание"; if($('#supply')[0].checked === false) continue; break;}
      case 3: {eventType = "Затопление"; if($('#flood')[0].checked === false) continue; break;}
      case 4: {eventType = "Дверь"; if($('#door')[0].checked === false) continue; break;}
      case 5: {eventType = "Связь"; if($('#connect')[0].checked === false) continue; break;}
      case 6: {eventType = "Прочее"; if($('#other')[0].checked === false) continue; break;}
      default: {eventType = "Неизвестно"; break;}
    }
    t = new Date(resultFromServer[i].timestamp).getTime();
    t += 3*3600*1000;
    t = new Date(t);
    innerHtml += "<tr><td>" + t.toLocaleString('en-GB') + "</td><td>" + eventType + "</td><td>" + eventStr + "</td></tr>";
  }
  var a = document.getElementById("historyTbody");
  a.innerHTML = innerHtml;
}


function cb16mbError(){
 // document.getElementById("lastConnection").innerText = "ServerError"  
}

