<?php
// ***********************************************
// Base include file for library functions for AMS
// ***********************************************
function __autoload( $className ){
     global $AMS_LIB;
     global $SITEBASE;
     if(file_exists( $AMS_LIB.'/autoload/' . strtolower ( $className ) . '.php')){
          require_once 'autoload/' . strtolower ( $className ) . '.php';
     }
     if($className == "WebUsers"){
     require_once $SITEBASE.'/autoload/' . strtolower ( $className ) . '.php';
     }
}


