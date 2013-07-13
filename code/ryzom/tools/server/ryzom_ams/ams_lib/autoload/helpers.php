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
             
          
         $variables = Helpers::handle_language();
         foreach ( $variables[$template] as $key => $value ){
             $smarty -> assign( $key, $value );
             }
          if( isset($vars['permission']) && $vars['permission'] == 2 ){
               $inherited = "extends:layout_admin.tpl|";
          }else if( isset($vars['permission']) && $vars['permission'] == 1){
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
             //$AMS_LIB . '/cache',
             $SITEBASE . '/cache/',
             $SITEBASE . '/templates/',
             $SITEBASE . '/templates_c/',
             $SITEBASE . '/configs'
             );
         foreach ( $arr as & $value ){
             if ( !file_exists( $value ) ){
                 echo $value;
                 mkdir( $value);
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
       
     static public function handle_language(){
          global $DEFAULT_LANGUAGE;
          global $AMS_TRANS;
          
          //if language get param is given = set cookie
          //else if no get param is given and a cookie is set, use that language, else use default.
          if ( isset( $_GET['language'] ) ) {
               //check if the language is supported 
               if ( file_exists( $AMS_TRANS . '/' . $_GET['language'] . '.ini' ) ){
                    //if it's supported, set cookie!
                    setcookie( 'language',$_GET['language'], time() + 60*60*24*30 );
                    $language = $_GET['language'];
               }else{
                    //the language is not supported, use the default.
                    $language = $DEFAULT_LANGUAGE;
               }
          }else{
               //if no get param is given, check if a cookie value for language is set 
               if ( isset( $_COOKIE['language'] ) ) { 
                    $language = $_COOKIE['language']; 
               }
               //else use the default
               else{
                     $language = $DEFAULT_LANGUAGE; 
               }
          }
             
         return parse_ini_file( $AMS_TRANS . '/' . $language . '.ini', true );
         
         
     }
}
