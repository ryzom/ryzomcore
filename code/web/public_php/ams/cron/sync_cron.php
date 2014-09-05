<?php

/**
* This small piece of php code calls the syncdata() function of Sync class.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
require( '../../config.php' );
require_once( $AMS_LIB . '/libinclude.php' );
Sync::syncdata();