<?php
class Helpers{
    
    public function loadTemplate( $template, $vars = array () )
    {
         extract( $vars );
         include( $template );
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

