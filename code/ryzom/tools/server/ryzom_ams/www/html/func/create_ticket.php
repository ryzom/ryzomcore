<?php

function create_ticket(){
    try{
        //if logged in
        if(WebUsers::isLoggedIn() && isset($_SESSION['ticket_user'])){
            
            if(isset($_POST['target_id'])){
                
                //if target_id is the same as session id or is admin
                if(  ($_POST['target_id'] == $_SESSION['id']) ||  WebUsers::isAdmin()  ){
                    
                    global $cfg;
                    $category = filter_var($_POST['Category'], FILTER_SANITIZE_NUMBER_INT);
                    $title = filter_var($_POST['Title'], FILTER_SANITIZE_STRING);
                    $content = filter_var($_POST['Content'], FILTER_SANITIZE_STRING);
                    if($_POST['target_id'] == $_SESSION['id']){
                        $author = $_SESSION['ticket_user']->getTUserId();
                    }else{
                        $author=  Ticket_User::constr_ExternId($_POST['target_id'], $cfg['db']['lib'])->getTUserId();
                    }
                    $ticket = new Ticket($cfg['db']['lib']);
                    $ticket->setTicket($title,0,0,$category,$author);
                    $ticket->create();
                    
                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    exit;
                }
        
            }else{
                //ERROR: The form was not filled in correclty
		header("Location: index.php?page=settings");
		exit;
            }    
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    exit;
        }
                  
    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         exit;
    }
    
}

