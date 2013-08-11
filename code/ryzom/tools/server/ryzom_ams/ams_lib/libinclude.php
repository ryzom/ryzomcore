<?php
// ***********************************************
// Base include file for library functions for AMS
// ***********************************************
function __autoload( $className ){
     if(file_exists( '/home/daan/ryzom/ryzomcore/code/ryzom/tools/server/ryzom_ams/ams_lib/autoload/' . strtolower ( $className ) . '.php')){
          require_once 'autoload/' . strtolower ( $className ) . '.php';
     }
     if($className == "WebUsers")
     require_once '/home/daan/ryzom/ryzomcore/code/ryzom/tools/server/ryzom_ams/www/html/autoload/' . strtolower ( $className ) . '.php';
     }

