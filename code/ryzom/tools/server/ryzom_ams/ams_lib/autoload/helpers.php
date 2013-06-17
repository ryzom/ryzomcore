<?php
class Helpers{

     static public function loadTemplate( $template, $vars = array (), $forcelibrender = false )
    {
         global $AMS_LIB;
         global $SITEBASE;
         global $AMS_TRANS;
         require_once $AMS_LIB . '/smarty/libs/Smarty.class.php';
         $smarty = new Smarty;

         // turn smarty debugging on/off
        $smarty -> debugging = false;
         // caching must be disabled for multi-language support
        $smarty -> caching = false;
         $smarty -> cache_lifetime = 120;

         helpers :: create_folders ();

         if ( helpers :: check_if_game_client () or $forcelibrender = false ){
             $smarty -> template_dir = $AMS_LIB . '/ingame_templates/';
             $smarty -> setConfigDir( $AMS_LIB . '/configs' );
             }else{
             $smarty -> template_dir = $SITEBASE . '/templates/';
             $smarty -> setConfigDir( $SITEBASE . '/configs' );
             }

         foreach ( $vars as $key => $value ){
             $smarty -> assign( $key, $value );
             }
         if ( isset( $_GET["language"] ) ){
             $language = $_GET["language"];
             if ( file_exists( $AMS_TRANS . '/' . $language . '.ini' ) ){

                 }else{
                 global $DEFAULT_LANGUAGE;
                 $language = $DEFAULT_LANGUAGE;
                 }
             }else{
             global $DEFAULT_LANGUAGE;
             $language = $DEFAULT_LANGUAGE;
             }
         $variables = parse_ini_file( $AMS_TRANS . '/' . $language . '.ini', true );
         foreach ( $variables[$template] as $key => $value ){
             $smarty -> assign( $key, $value );
             }
          if( $vars['permission'] == 2 ){
               $inherited = "extends:layout_admin.tpl|";
          }else if($vars['permission'] == 1){
               $inherited = "extends:layout_user.tpl|";
          }else{
               $inherited ="";
          }
         // extends:' . $inherited .'|register.tpl
        $smarty -> display( $inherited . $template . '.tpl' );
         }

     static public function create_folders(){
         global $AMS_LIB;
         global $SITEBASE;
         $arr = array( $AMS_LIB . '/ingame_templates/',
             $AMS_LIB . '/configs',
             $AMS_LIB . '/cache',
             $SITEBASE . '/cache/',
             $SITEBASE . '/templates/',
             $SITEBASE . '/templates_c/',
             $SITEBASE . '/configs'
             );
         foreach ( $arr as & $value ){
             if ( !file_exists( $value ) ){
                 mkdir( $value );
                 }
             }

         }

     static public function check_if_game_client()
    {
         // if HTTP_USER_AGENT is not set then its ryzom core
        if ( !isset( $_SERVER['HTTP_USER_AGENT'] ) ){
             return true;
             }else{
             return false;
             }
         }
     }

