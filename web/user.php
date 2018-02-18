﻿<?php                                                    
  header('Access-Control-Allow-Origin: *');        
  error_reporting(0);                       // 
  $act = stripslashes($_POST['action']);
  
  $answer = array("status" => "", "result" => array());
  $db = "u435400245_mermn";
  $link = mysqli_connect("mysql.hostinger.ru", "u435400245_mrnus", "tWm7D7Cg56GCv", $db);
  if(!$link) {
    $answer["status"] = "fail";
    $answer["result"][0] = "Can't connect to MySQL or to $db";
    exit(json_encode($answer));
  }   
  
  if($act == "readSettings"){
    $query = "SELECT * FROM `options` WHERE id=1";    // Подумать об экранировании с помощью mysqli_real_escape_string()
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


  if($act == "writeValue") {
    $name = stripslashes($_POST['name']);
    $value = stripslashes($_POST['value']); 
    $query = "UPDATE options SET " . $name . "=" . $value . " WHERE id = 1";
    $result = mysqli_query ($link, $query);
    if(!$result){
      $answer["status"] = "fail";
      $answer["result"][0] = mysqli_error($link); 
    } 
    else {
      $answer["status"] = "success";
    }               
  }

  if($act == "writeSettings") {
    $pump_work = stripslashes($_POST['pump_work']);
    $pump_idle = stripslashes($_POST['pump_idle']);
    $min_temp = stripslashes($_POST['min_temp']);
    $max_temp = stripslashes($_POST['max_temp']);
    $report_interval = stripslashes($_POST['report_interval']);
    $cold_warning = stripslashes($_POST['cold_warning']);
    $cold_warning_temp = stripslashes($_POST['cold_warning_temp']);
    $warm_warning = stripslashes($_POST['warm_warning']);
    $warm_warning_temp = stripslashes($_POST['warm_warning_temp']);
    $door_warning = stripslashes($_POST['door_warning']);
    $flooding_warning = stripslashes($_POST['flooding_warning']);
    
    $power_warning = stripslashes($_POST['power_warning']);
    $power_rest_warning = stripslashes($_POST['power_rest_warning']);
    $offline_warning = stripslashes($_POST['offline_warning']);
    $offline_warning_duration = stripslashes($_POST['offline_warning_duration']);
    $balance_warning = stripslashes($_POST['balance_warning']);
    $balance_warning_summ = stripslashes($_POST['balance_warning_summ']);
    $daily_report = stripslashes($_POST['daily_report']);
    $daily_report_time = stripslashes($_POST['daily_report_time']);
    $operator_number = stripslashes($_POST['operator_number']);
    $admin_number = stripslashes($_POST['admin_number']);
    
    $query = "UPDATE options SET pump_work=".$pump_work.", pump_idle=".$pump_idle.", min_temp=".$min_temp.", max_temp=".$max_temp.", report_interval=".$report_interval.", cold_warning=".$cold_warning.", cold_warning_temp=".$cold_warning_temp.", warm_warning=".$warm_warning.", warm_warning_temp=".$warm_warning_temp.", door_warning=".$door_warning.", flooding_warning=".$flooding_warning.", power_warning=".$power_warning.", power_rest_warning=".$power_rest_warning.", offline_warning=".$offline_warning.", offline_warning_duration=".$offline_warning_duration.", balance_warning=".$balance_warning.", balance_warning_summ=".$balance_warning_summ.", daily_report=".$daily_report.", daily_report_time=".$daily_report_time.", operator_number=".$operator_number.", admin_number=".$admin_number." WHERE id = 1";
    //$query = "UPDATE options SET pump_work=".$pump_work.", pump_idle=".$pump_idle.", cold_warning=".$cold_warning.", warm_warning=".$warm_warning.", door_warning=".$door_warning.", flooding_warning=".$flooding_warning.", power_warning=".$power_warning.", power_rest_warning=".$power_rest_warning.", offline_warning=".$offline_warning.", balance_warning=".$balance_warning.", daily_report=".$daily_report.", operator_number=".$operator_number.", admin_number=".$admin_number." WHERE id = 1";
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
    $answer["status"] = "null";     
  }

  echo json_encode($answer);
  mysqli_close($link);
		