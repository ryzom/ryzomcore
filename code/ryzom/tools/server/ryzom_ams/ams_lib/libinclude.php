<?php
/**
* Base include file for library functions for AMS.
* Autoload function that loads the classes in case they aren't loaded yet.
*/
function __autoload( $className ){
     global $AMS_LIB;
     global $SITEBASE;
     //if the class exists in the lib's autload dir, load that one
     if(file_exists( $AMS_LIB.'/autoload/' . strtolower ( $className ) . '.php')){
          require_once 'autoload/' . strtolower ( $className ) . '.php';
     }
     //if the classname is WebUsers, use the sitebase location for the autoload dir.
     if($className == "WebUsers"){
          require_once $SITEBASE.'/autoload/' . strtolower ( $className ) . '.php';
     }
}


