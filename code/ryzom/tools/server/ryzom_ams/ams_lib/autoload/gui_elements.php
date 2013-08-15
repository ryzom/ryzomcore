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
    
    
    public static function time_elapsed_string($ptime){
        global $TIME_FORMAT;
        $ptime = DateTime::createFromFormat($TIME_FORMAT, $ptime)->getTimestamp();
        
        $etime = time() - $ptime;
    
        if ($etime < 1)
        {
            return '0 seconds';
        }
    
        $a = array( 12 * 30 * 24 * 60 * 60  =>  'year',
                    30 * 24 * 60 * 60       =>  'month',
                    24 * 60 * 60            =>  'day',
                    60 * 60                 =>  'hour',
                    60                      =>  'minute',
                    1                       =>  'second'
                    );
    
        foreach ($a as $secs => $str)
        {
            $d = $etime / $secs;
            if ($d >= 1)
            {
                $r = round($d);
                return $r . ' ' . $str . ($r > 1 ? 's' : '') . ' ago';
            }
        }
    }

}