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
    
    public static function make_table_with_key_is_id( $inputList, $funcArray, $idFunction){
        $result = Array();
        foreach($inputList as $element){
            foreach($funcArray as $function){
                $result[$element->$idFunction()] = $element->$function();
            }
        }
        return $result;  
    }
}