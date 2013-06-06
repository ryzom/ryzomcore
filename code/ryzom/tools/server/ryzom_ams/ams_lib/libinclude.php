<?php
// ***********************************************
// Base include file for library functions for AMS
// ***********************************************
function __autoload( $className ){
     require_once 'autoload/' . $className . '.php';
    }

