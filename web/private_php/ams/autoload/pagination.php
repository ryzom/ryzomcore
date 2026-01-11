<?php
/**
* Handles returning arrays based on a given pagenumber.
* By specifing a $_GET['pagenum'] or if not(page = 1 will be used) a few elements from a specific query will be returned. Not all elements have to be loaded into objects, only
* the elements needed for that specific page, this is a good thing performance wise. This is done by passign the query to the constructor and specifying how many you want to display.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Pagination{
    
    private $element_array; /**< Array containing the elements that are extracted for that specific page number */ 
    private $last; /**< The last page number */ 
    private $current; /**< The current page number (read from $_GET['pagenum']) */ 
    private $amountOfRows;  /**< Total amount of rows that a query would return (if no limits would be used) */ 
    
    /**
    * Constructor.
    * will fetch the correct elements that match to a specific page (specified by the $_GET['pagenum'] variable). The query has to be passed as a string to the function
    * that way it will only load the specific elements that are related to the pagenumber. The $params, parameter is optional and is used to pass the parameters for the query.
    * The result class will be used to instantiate the found elements with, their set() function will be called. The class  its getters can be later used to get the info out of the object.
    * @param $query the query to be paginated
    * @param $db the db on which the query should be performed
    * @param $nrDisplayed the amount of elements that should be displayed /page
    * @param $resultClass the elements that should be returned should be of that specific class.
    * @param $params the parameters used by the query (optional)
    */
    function __construct($query, $db, $nrDisplayed, $resultClass, $params = array()) {
        if (!(isset($_GET['pagenum']))){ 
            $this->current= 1; 
        }else{
            $this->current= $_GET['pagenum'];
        }
        
        //Here we count the number of results
        $db = new DBLayer($db);
        $rows = $db->execute($query, $params)->rowCount();
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
            $data = $db->execute($query . " " . $max, $params); 
            
            $this->element_array = Array();
            //This is where we put the results in a resultArray to be sent to smarty
            while($row = $data->fetch(PDO::FETCH_ASSOC)){
                $element = new $resultClass();
                $element->set($row);
                $this->element_array[] = $element;
            }
        }
    }
    
    
    /**
    * return the number of the 'last' object attribute
    * @return the number of the last page
    */
    public function getLast(){
        return $this->last;
    }
    
    
    /**
    * return the number of the 'current' object attribute
    * @return the number of the current page
    */
    public function getCurrent(){
        return $this->current;
    }
    
    
    /**
    * return the elements array of the object
    * @return the elements of a specific page (these are instantiations of the class passed as parameter ($resultClass) to the constructor)
    */
    public function getElements(){
        return $this->element_array;
    }
    
    
    /**
    * return total amount of rows for the original query
    * @return the total amount of rows for the original query
    */
    public function getAmountOfRows(){
        return $this->amountOfRows;
    }
    
    /**
    * return the page links.
    * (for browsing the pages, placed under a table for example) the $nrOfLinks parameter specifies the amount of links you want to return.
    * it will show the links closest to the current page on both sides (in case one side can't show more, it will show more on the other side)
    * @return an array of integerswhich refer to the clickable pagenumbers for browsing other pages.
    */
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