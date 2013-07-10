<?php

class Gui_Elements{
    
    public static function make_table( $inputList, $funcArray ,$fieldArray){
        $i = 0; 
        $result = Array();
        foreach($inputList as $element){
            $j = 0;
            foreach($funcArray as $function){
                $fnames = explode('->', $function);
                $intermediate_result = NULL;
                foreach($fnames as $fname) {
                    if(substr($fname, -2) == "()") {
                        $fname = substr($fname, 0, strlen($fname)-2);
                        if($intermediate_result == NULL) {
                            $intermediate_result = $element->$fname();
                        } else {
                            $intermediate_result = $intermediate_result->$fname();
                        }
                    } else {
                        if($intermediate_result == NULL) {
                            $intermediate_result = $element->$fname();
                        } else {
                            $intermediate_result = $intermediate_result->$fname();
                        }
                    }
                }
                $result[$i][$fieldArray[$j]] = $intermediate_result;
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