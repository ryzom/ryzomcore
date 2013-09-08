<?php

function create_ticket(){
    //if logged in
    global $INGAME_WEBPATH;
    global $WEBPATH;
    if(WebUsers::isLoggedIn() && isset($_SESSION['ticket_user'])){
        
        if(isset($_POST['target_id'])){
            
            //if target_id is the same as session id or is admin
            if(  ($_POST['target_id'] == $_SESSION['id']) ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user']))  ){
                
                $category = filter_var($_POST['Category'], FILTER_SANITIZE_NUMBER_INT);
                $title = filter_var($_POST['Title'], FILTER_SANITIZE_STRING);
                $content = filter_var($_POST['Content'], FILTER_SANITIZE_STRING);
                try{
                    if($_POST['target_id'] == $_SESSION['id']){
                        $author = unserialize($_SESSION['ticket_user'])->getTUserId();
                    }else{
                        $author=  Ticket_User::constr_ExternId($_POST['target_id'])->getTUserId();
                    }
                    $ticket_id = Ticket::create_Ticket($title, $content, $category, $author, unserialize($_SESSION['ticket_user'])->getTUserId(),0, $_POST);
                    if (Helpers::check_if_game_client()) {
                        header("Location: ".$INGAME_WEBPATH."?page=show_ticket&id=".$ticket_id);
                    }else{
                        header("Location: ".$WEBPATH."?page=show_ticket&id=".$ticket_id);
                    }
                    exit;
                    
                }catch (PDOException $e) {
                    //ERROR: LIB DB is not online!
                    print_r($e);
                    exit;
                    header("Location: index.php");
                    exit;
                }
                
            }else{
                //ERROR: permission denied!
                $_SESSION['error_code'] = "403";
                header("Location: index.php?page=error");
                exit;
            }
    
        }else{
            //ERROR: The form was not filled in correclty
            header("Location: index.php?page=create_ticket");
            exit;
        }    
    }else{
        //ERROR: user is not logged in
        header("Location: index.php");
        exit;
    }                
    
}

