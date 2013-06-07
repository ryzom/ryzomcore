<?php
// ***********************************************
// Base include file for library functions for AMS
// ***********************************************
function __autoload( $className ){
     require_once 'autoload/' . strtolower ($className) . '.php';
    }

