<?php                                                    
  header('Access-Control-Allow-Origin: *');        
  error_reporting(0);                       // 
  $act = stripslashes($_GET['act']);
  
  $answer = array("status" => "", "result" => array());
  $db = "id13099454_bd";
  $link = mysqli_connect("localhost", "id13099454_user", "acauLaXa+YsJET7K", $db);
  if(!$link) {
    $answer["status"] = "fail";
    $answer["result"][0] = "Can't connect to MySQL or to $db";
    exit(json_encode($answer));
  }   
  
  if($act == "GetSettings"){
    $query = "SELECT * FROM `options` WHERE id=1";    // Подумать об экранировании с помощью mysqli_real_escape_string()
    $result = mysqli_query($link, $query);
    if(!$result){
      $answer["status"] = "fail";
      $answer["result"][0] = mysqli_error($link); 
    }                
    else {
      $answer["status"] = "success";
      if(mysqli_num_rows($result) > 0){ 
        //$answer["result"][0] = mysqli_fetch_object($result); 
        $obj = mysqli_fetch_object($result);
        $answer["result"] = $obj->timestamp.",".$obj->pump_work.",".$obj->pump_idle.",".$obj->min_temp.",".$obj->max_temp.",".$obj->report_interval.",".$obj->cold_warning.",".$obj->cold_warning_temp.",".$obj->warm_warning.",".$obj->warm_warning_temp.",".$obj->door_warning.",".$obj->flooding_warning.",".$obj->power_warning.",".$obj->power_rest_warning.",".$obj->offline_warning.",".$obj->offline_warning_duration.",".$obj->balance_warning.",".$obj->balance_warning_summ.",".$obj->daily_report.",".$obj->daily_report_time.",".$obj->operator_number.",".$obj->admin_number;
      }    
    }
  }

  if($act == "gts"){      // Get TimeStamp  Вернуть таймштамп настроек
    $query = "SELECT `timestamp` FROM `options` WHERE id=1";    // Подумать об экранировании с помощью mysqli_real_escape_string()
    $result = mysqli_query($link, $query);
    if(!$result){
      $answer["status"] = "fail";
      $answer["result"][0] = mysqli_error($link); 
    }                
    else {
      $answer["status"] = "success";
      if(mysqli_num_rows($result) > 0) 
        $answer["result"][0] = mysqli_fetch_object($result); 
    }
  }


  if($act == "wT") {
    $temp = stripslashes($_GET['t']); 
    $Vbat = stripslashes($_GET['vb']); 
    $balance = stripslashes($_GET['b']); 
    //$query = "INSERT INTO `temps` (`tt`, `Vbat`) VALUES (" . $temp . "," . $Vbat . ")";    
    //$result1 = mysqli_query ($link, $query);

    $query = "UPDATE stats SET balance=".$balance.", Vbat=".$Vbat.", temp=".$temp." WHERE id = 1";
    $result = mysqli_query ($link, $query);
    if(!$result){
      $answer["status"] = "fail";
      $answer["result"][0] = mysqli_error($link); 
    } 
    else {
      $answer["status"] = "success";
    }               
  }

  if($act == "wS") {       // Принять и сохранить настройки
    $pump_work = stripslashes($_GET['pump1']);
    $pump_idle = stripslashes($_GET['pump0']);
    $min_temp = stripslashes($_GET['mint']);
    $max_temp = stripslashes($_GET['maxt']);
    
    $query = "UPDATE options SET pump_work=".$pump_work.", pump_idle=".$pump_idle.", min_temp=".$min_temp.", max_temp=".$max_temp." WHERE id = 1";
//    $query = "UPDATE options SET pump_work=".$pump_work.", pump_idle=".$pump_idle.", min_temp=".$min_temp.", max_temp=".$max_temp.", report_interval=".$report_interval.", cold_warning=".$cold_warning.", cold_warning_temp=".$cold_warning_temp.", warm_warning=".$warm_warning.", warm_warning_temp=".$warm_warning_temp.", door_warning=".$door_warning.", flooding_warning=".$flooding_warning.", power_warning=".$power_warning.", power_rest_warning=".$power_rest_warning.", offline_warning=".$offline_warning.", offline_warning_duration=".$offline_warning_duration.", balance_warning=".$balance_warning.", balance_warning_summ=".$balance_warning_summ.", daily_report=".$daily_report.", daily_report_time=".$daily_report_time.", operator_number=".$operator_number.", admin_number=".$admin_number." WHERE id = 1";
    $result = mysqli_query ($link, $query);
    if(!$result){
      $answer["status"] = "fail";
      $answer["result"][0] = mysqli_error($link); 
    } 
    else {
      $answer["status"] = "success";
    }               
  }


  if($act == null){
    $answer["status"] = "act=nuLL";     
  }

  echo json_encode($answer);
  mysqli_close($link);
		