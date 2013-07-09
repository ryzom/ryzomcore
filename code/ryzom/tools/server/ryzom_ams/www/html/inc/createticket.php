<?php

function createticket(){

    //if logged in
    if(WebUsers::isLoggedIn()){
        //in case user_id-GET param set it's value as target_id, if no user_id-param is given, use the session id.
        if(isset($_GET['user_id'])){
            
            if(($_GET['user_id'] != $_SESSION['id']) && (!WebUsers::isAdmin()) ){
                
                //ERROR: No access!
                $_SESSION['error_code'] = "403";
                header("Location: index.php?page=error");
                exit;
                
            }else{
                //if user_id is given, then set it as the target_id
                $result['target_id'] = filter_var($_GET['user_id'], FILTER_SANITIZE_NUMBER_INT);
            }
        
        }else{
            //set session_id as target_id
            $result['target_id'] = $_SESSION['id'];
            
     
        }
        
        //create array of category id & names
        global $cfg;
        $catArray = Ticket_Category::getAllCategories($cfg['db']['lib']);
        $result['category'] = Gui_Elements::make_table_with_key_is_id($catArray, Array("getName"), "getTCategoryId" );
        return $result;
    
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }
    
}