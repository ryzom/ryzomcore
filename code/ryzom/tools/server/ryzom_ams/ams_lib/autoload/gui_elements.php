<?php

class Gui_Elements{
    
    public static function make_table( $inputList, $funcArray ,$fieldArray){
        $i = 0; 
        $result = Array();
        foreach($inputList as $element){
            $j = 0;
            foreach($funcArray as $function){
                $result[$i][$fieldArray[$j]] = $element->$function();
                $j++;
            }
            $i++;
        }
        return $result;  
    }
    
}