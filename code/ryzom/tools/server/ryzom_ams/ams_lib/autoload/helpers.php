<?php
class Helpers{
    
     public function loadTemplate( $template, $vars = array () )
    {
         global $AMS_LIB;
         global $NELTOOL_SITEBASE;
         require_once $AMS_LIB . '/smarty/libs/Smarty.class.php';
         $smarty = new Smarty;
        
         $smarty -> debugging = true;
         $smarty -> caching = true;
         $smarty -> cache_lifetime = 120;
         if ( !helpers :: check_if_game_client () ){
             $smarty -> template_dir = $AMS_LIB . '/templates/';
             $smarty->setConfigDir($AMS_LIB .'/config');
             }else{
             $smarty -> template_dir = $NELTOOL_SITEBASE . '/templates/';
             $smarty->setConfigDir($NELTOOL_SITEBASE .'/config');
             }
         $smarty -> assign( "option_selected", "NE" );
         $smarty -> display( $template . '.tpl' );
         }
    
     public function check_if_game_client()
    {
         // if HTTP_USER_AGENT is not set then its ryzom core
        if ( !isset( $_SERVER['HTTP_USER_AGENT'] ) ){
             return true;
             }else{
             return false;
             }
         }
     }

