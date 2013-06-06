<?php
class Helpers {
  
public function loadTemplate( $template, $vars = array () )
{
     extract( $vars );
     include( $template );
    }
}

