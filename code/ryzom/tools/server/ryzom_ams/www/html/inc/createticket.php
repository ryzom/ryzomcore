<?php

function createticket(){
    
    //create array of category id & names
    global $cfg;
    $catArray = Ticket_Category::getAllCategories($cfg['db']['lib']);
    $result['category'] = Array();
    foreach($catArray as $catObj){
        $result['category'][$catObj->getTCategoryId()] = $catObj->getName();  
    }
    //print_r($result);
    return $result;
}