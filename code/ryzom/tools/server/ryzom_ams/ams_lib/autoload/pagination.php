<?php

class Pagination{
    
    private $element_array;
    private $last;
    private $current;
    
    function __construct($query,$db,$nrDisplayed,$resultClass) {
        if (!(isset($_GET['pagenum']))){ 
            $this->current= 1; 
        }else{
            $this->current= $_GET['pagenum'];
        }
        
        //Here we count the number of results
        $db = new DBLayer($db);
        $rows = $db->executeWithoutParams($query)->rowCount();
        
        //the array hat will contain all users

        if($rows > 0){
            //This is the number of results displayed per page 
            $page_rows = $nrDisplayed;
            
            //This tells us the page number of our last page
            $this->last = ceil($rows/$page_rows); 
            
            //this makes sure the page number isn't below one, or more than our maximum pages 
            if ($this->current< 1) 
            { 
                $this->current= 1; 
            }else if ($this->current> $this->last) { 
                $this->current= $this->last; 
            } 
            
            //This sets the range to display in our query 
            $max = 'limit ' .($this->current- 1) * $page_rows .',' .$page_rows; 
            
            //This is your query again, the same one... the only difference is we add $max into it
            $data = $dbl->executeWithoutParams($query . $max); 
            
            $this->element_array = Array();
            //This is where we put the results in a resultArray to be sent to smarty
            while($row = $data->fetch(PDO::FETCH_ASSOC)){
                $element = new $resultClass();
                $element.set($row);
                $this->element_array[] = $element;
            }
        }
    }
    
    
    function getLast(){
        return $this->last;
    }
    
    
    function getElements(){
        return $this->element_array;
    }
    
    
    function getPagination($nrOfLinks){
        $pageLinks = Array();
        if ($this->last <= $nrOfLinks){
            for($var = 1; $var <= $this->last; $var++){
                $pageLinks[] = $var;
            }
        }else{
            $pageLinks[] = 1;
            $offset = (ceil($nrOfLinks/2)-1);
            $startpoint = $this->current - $offset;
            $endpoint =  $this->current + $offset;
            if($startpoint < 2){
                $startpoint = 2;
                $endpoint = $startpoint + $nrOfLinks - 2;
            }else if($endpoint > $this->last){
                $endpoint = $this->last;
                $startpoint = $this->last - $nrOfLinks -2;
            }
            for($var = $startpoint; $var <= $endpoint; $var++){
                $pageLinks[] = $var;
            }
            $pageLinks[] = $this->last;
        }
        return $pageLinks;
    }
}