var sqlServerAdress = "https://vodyanoy.000webhostapp.com/r.php";

window.onload = function() {          //
  document.getElementById('sendStats').onclick = SendStatsOnServer;  
}

function SendStatsOnServer() {      // Сохраним ВСЕ настройки на сервере
  var params = {}; 
  params.act = "wT";

  params.t = document.getElementById('temp').value*16;
  params.vb = document.getElementById('vBat').value*200;
  params.b = document.getElementById('balance').value;

  $.ajax({
    url: sqlServerAdress,
    type: 'get',
    dataType: 'json',
    data:  params
  });
}
