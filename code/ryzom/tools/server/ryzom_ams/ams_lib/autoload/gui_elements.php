<?php
/**
* Helper class for generating gui related elements.
* This class contains functions that generate data-arrays for tables, or other visual entities
* @author Daan Janssens, mentored by Matthew Lagoe
* 
*/
class Gui_Elements{
    
    /**
    * creates an array of information out of a list of objects which can be used to form a table.
    * The idea of this is that you can pass an array of objects, an array of functions to perform on each object and a name for the index of the returning array to store
    * the result.
    * @param $inputList the list of objects of which we want to make a table.
    * @param $funcArray a list of methods of that object we want to perform.
    * @param $fieldArray a list of strings, that will be used to store our result into.
    * @return an array with the indexes listed in $fieldArray and which holds the results of the methods in $funcArray on each object in the $inputList
    */
    public static function make_table( $inputList, $funcArray ,$fieldArray){
        $i = 0; 
        $result = Array();
        if(!empty($inputList)){
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
        }
        return $result;  
    }
    
    /**
    * creates an array of information out of a list of objects which can be used to form a table with a key as id.
    * The idea is comparable to the make_table() function, though this time the results are stored in the index that is returned by the idFunction()
    * @param $inputList the list of objects of which we want to make a table.
    * @param $funcArray a list of methods of that object we want to perform.
    * @param $idFunction a function that returns an id that will be used as index to store our result
    * @return an array which holds the results of the methods in $funcArray on each object in the $inputList, though thearrays indexes are formed by using the idFunction.
    */
    public static function make_table_with_key_is_id( $inputList, $funcArray, $idFunction){
        $result = Array();
        foreach($inputList as $element){
            foreach($funcArray as $function){
                $result[$element->$idFunction()] = $element->$function();
            }
        }
        return $result;  
    }
    
    
    /**
    * returns the elapsed time from a timestamp up till now.
    * @param $ptime a timestamp.
    * @return a string in the form of A years, B months, C days, D hours, E minutes, F seconds ago.
    */
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