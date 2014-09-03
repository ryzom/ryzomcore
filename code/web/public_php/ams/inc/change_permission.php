<?php
/**
* This function is beign used to change the permission of a ticket_user.
* It will first check if the user who executed this function is an admin. If this is not the case the page will be redirected to an error page.
* in case the $_GET['value'] is smaller than 4 and the user whoes permission is being changed is different from the admin(id 1), the change will be executed and the page will
* redirect to the users profile page.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function change_permission(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    //if logged in
    if(WebUsers::isLoggedIn()){

        //check if user who executed this function is an admin
        if(ticket_user::isAdmin(unserialize($_SESSION['ticket_user']))){

            //in case the $_GET['value'] is smaller than 4 and the user whoes permission is being changed is different from the admin(id 1)
            if(isset($_GET['user_id']) && isset($_GET['value']) && $_GET['user_id'] != 1 && $_GET['value'] < 4 ){
                $user_id = filter_var($_GET['user_id'], FILTER_SANITIZE_NUMBER_INT);
                $value = filter_var($_GET['value'], FILTER_SANITIZE_NUMBER_INT);

                //execute change.
                Ticket_User::change_permission(Ticket_User::constr_ExternId($user_id)->getTUserId(), $value);
                if (Helpers::check_if_game_client()) {
                    header("Location: ".$INGAME_WEBPATH."?page=show_user&id=".$user_id);
                }else{
                    header("Location: ".$WEBPATH."?page=show_user&id=".$user_id);
                }
                throw new SystemExit();


            }else{
                //ERROR: GET PARAMS not given or trying to change admin
                if (Helpers::check_if_game_client()) {
                    header("Location: ".$INGAME_WEBPATH."?page=show_user&id=".$user_id);
                }else{
                    header("Location: ".$WEBPATH."?page=show_user&id=".$user_id);
                }
                throw new SystemExit();
            }

        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            throw new SystemExit();

        }

    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        throw new SystemExit();
    }


}
