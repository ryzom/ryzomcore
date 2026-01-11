<?php
/**
* This function is beign used to load info that's needed for the error page.
* if a error_code session var is set it will unset it (else 404 is used), and it will return the error code so it can be used in the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function error(){
  if(isset($_SESSION['error_code'])){
    $result['error_code'] = $_SESSION['error_code'];
    unset($_SESSION['error_code']);
  }else{
    $result['error_code'] = "404";
  }
  return $result;

}