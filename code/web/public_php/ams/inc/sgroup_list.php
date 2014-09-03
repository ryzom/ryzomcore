<?php
/**
* This function is beign used to load info that's needed for the sgroup_list page.
* check if the person who wants to view this page is a mod/admin, if this is not the case, he will be redirected to an error page.
* It will return all suppport groups information. Also if the $_GET['delete'] var is set and the user is an admin, he will delete a specific entry.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function sgroup_list(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    //if logged in
    if(WebUsers::isLoggedIn()){
        if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){

            //if delete GET var is set and user is admin, then delete the groups entry.
            if(isset($_GET['delete']) && Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                $delete_id = filter_var($_GET['delete'], FILTER_SANITIZE_NUMBER_INT);
                $result['delete'] = Support_Group::deleteSupportGroup( $delete_id);
                if (Helpers::check_if_game_client()) {
                    header("Location: ".$INGAME_WEBPATH."?page=sgroup_list");
                }else{
                    header("Location: ".$WEBPATH."?page=sgroup_list");
                }
                die();
            }
            if(Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                $result['isAdmin'] = "TRUE";
            }
            $result['grouplist'] = Gui_Elements::make_table(Support_Group::getGroups(), Array("getSGroupId","getName","getTag","getGroupEmail"), Array("sGroupId","name","tag","groupemail"));
            global $INGAME_WEBPATH;
            $result['ingame_webpath'] = $INGAME_WEBPATH;
            return $result;
        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            die();
        }
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        die();
    }

}
