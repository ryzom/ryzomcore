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
    
    
    public function getElements(){
        return $this->element_array;
    }
    
    
    public function getLinks($nrOfLinks){
        $pageLinks = Array();
        $pageLinks[] = 1;
        //if amount of showable links is greater than the amount of pages: show all!
        if ($this->last <= $nrOfLinks){
            for($var = 2; $var <= $this->last; $var++){
                $pageLinks[] = $var;
            }
        }else{
            $offset = ($nrOfLinks-3)/2 ;
            print "<font color='purple'>offset:" . $offset . "</font>";
            $startpoint = $this->current - $offset;
            $endpoint =  $this->current + $offset;
            print "<font color='blue'>startpointX:" . $startpoint . "</font>";
            if($startpoint < 2){
                $startpoint = 2;
                $endpoint = $startpoint + $nrOfLinks - 3;
            }else if($endpoint > $this->last-1){
                $endpoint = $this->last-1;
                $startpoint = $endpoint - ($nrOfLinks -3);
            }
            print "<font color='blue'>startpoint:" . $startpoint . "</font>";
            print "<font color='orange'>endpoint:" . $endpoint . "</font>";
            for($var = $startpoint; $var <= $endpoint; $var++){
                $pageLinks[] = $var;
            }
            $pageLinks[] = $this->last;
        }
        return $pageLinks;
    }
}