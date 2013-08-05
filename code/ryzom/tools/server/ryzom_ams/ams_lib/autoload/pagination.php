<?php

class Pagination{
    
    private $element_array;
    private $last;
    private $current;
    private $amountOfRows;
    
    function __construct($query,$db,$nrDisplayed,$resultClass) {
        if (!(isset($_GET['pagenum']))){ 
            $this->current= 1; 
        }else{
            $this->current= $_GET['pagenum'];
        }
        
        //Here we count the number of results
        $db = new DBLayer($db);
        $rows = $db->executeWithoutParams($query)->rowCount();
        $this->amountOfRows = $rows;
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
            $data = $db->executeWithoutParams($query . " " . $max); 
            
            $this->element_array = Array();
            //This is where we put the results in a resultArray to be sent to smarty
            while($row = $data->fetch(PDO::FETCH_ASSOC)){
                $element = new $resultClass();
                $element->set($row);
                $this->element_array[] = $element;
            }
        }
    }
    
    
    public function getLast(){
        return $this->last;
    }
    
    public function getCurrent(){
        return $this->current;
    }
    
    public function getElements(){
        return $this->element_array;
    }
    
    public function getAmountOfRows(){
        return $this->amountOfRows;
    }
    
    public function getLinks($nrOfLinks){
        $pageLinks = Array();
        //if amount of showable links is greater than the amount of pages: show all!
        if ($this->last <= $nrOfLinks){
            for($var = 1; $var <= $this->last; $var++){
                $pageLinks[] = $var;
            }
        }else{
            $offset = ($nrOfLinks-1)/2 ;
            $startpoint = $this->current - $offset;
            $endpoint =  $this->current + $offset;
            
            if($startpoint < 1){
                $startpoint = 1;
                $endpoint = $startpoint + $nrOfLinks - 1;
            }else if($endpoint > $this->last){
                $endpoint = $this->last;
                $startpoint = $endpoint - ($nrOfLinks -1);
            }
           
            for($var = $startpoint; $var <= $endpoint; $var++){
                $pageLinks[] = $var;
            }
        }
        return $pageLinks;
    }
}