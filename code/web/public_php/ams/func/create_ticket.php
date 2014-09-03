<?php
/**
* This function is beign used to create a new ticket.
* It will first check if the user who executed this function is the person of whom the setting is or if it's a mod/admin. If this is not the case the page will be redirected to an error page.
* next it will filter the POST data and it will try to create the new ticket. Afterwards a redirecion to the ticket will occur.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
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
                        //if the ticket is being made for the executing user himself
                        $author = unserialize($_SESSION['ticket_user'])->getTUserId();
                    }else{
                        //if a mod tries to make a ticket for someone else
                        $author=  Ticket_User::constr_ExternId($_POST['target_id'])->getTUserId();
                    }
                    //create the ticket & return the id of the newly created ticket.
                    $ticket_id = Ticket::create_Ticket($title, $content, $category, $author, unserialize($_SESSION['ticket_user'])->getTUserId(),0, $_POST);
                    //redirect to the new ticket.
                    if (Helpers::check_if_game_client()) {
                header("Cache-Control: max-age=1");
                        header("Location: ".$INGAME_WEBPATH."?page=show_ticket&id=".$ticket_id);
                    }else{
                header("Cache-Control: max-age=1");
                        header("Location: ".$WEBPATH."?page=show_ticket&id=".$ticket_id);
                    }
                    throw new SystemExit();

                }catch (PDOException $e) {
                    //ERROR: LIB DB is not online!
                    print_r($e);
                    throw new SystemExit();
                header("Cache-Control: max-age=1");
                    header("Location: index.php");
                    throw new SystemExit();
                }

            }else{
                //ERROR: permission denied!
                $_SESSION['error_code'] = "403";
                header("Cache-Control: max-age=1");
                header("Location: index.php?page=error");
                throw new SystemExit();
            }

        }else{
            //ERROR: The form was not filled in correclty
                header("Cache-Control: max-age=1");
            header("Location: index.php?page=create_ticket");
            throw new SystemExit();
        }
    }else{
        //ERROR: user is not logged in
                header("Cache-Control: max-age=1");
        header("Location: index.php");
        throw new SystemExit();
    }

}

