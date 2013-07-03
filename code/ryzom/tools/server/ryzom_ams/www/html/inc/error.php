<?php

function error(){
  if(isset($_SESSION['error_code'])){
    $result['error_code'] = $_SESSION['error_code'];
    unset($_SESSION['error_code']);
  }else{
    $result['error_code'] = "404";
  }
  return $result;

}