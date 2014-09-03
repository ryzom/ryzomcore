<?php
/**
* This function is beign used to add a new Support Group to the database.
* What it will do is check if the user who executed the function is an Admin, if so then it will filter all POST'ed data and use it to create a new Support_Group entry.
* if not logged in or not an admin, an appropriate redirection to an error page will take place.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function add_sgroup(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    if(WebUsers::isLoggedIn()){

        //check if admin
        if( Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
            $name = filter_var($_POST['Name'],FILTER_SANITIZE_STRING);
            $inner_tag = filter_var($_POST['Tag'], FILTER_SANITIZE_STRING);
            $tag = "[" . $inner_tag . "]";
            $inner_tag = filter_var($_POST['Tag'], FILTER_SANITIZE_STRING);
            $groupemail = filter_var($_POST['GroupEmail'], FILTER_SANITIZE_STRING);
            $imap_mailserver = filter_var($_POST['IMAP_MailServer'], FILTER_SANITIZE_STRING);
            $imap_username = filter_var($_POST['IMAP_Username'], FILTER_SANITIZE_STRING);
            $imap_password = filter_var($_POST['IMAP_Password'], FILTER_SANITIZE_STRING);

            //create a new support group
            $result['RESULT_OF_ADDING'] = Support_Group::createSupportGroup($name, $tag, $groupemail, $imap_mailserver, $imap_username, $imap_password);
            $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
            $result['no_visible_elements'] = 'FALSE';
            $result['username'] = $_SESSION['user'];
            global $SITEBASE;
            require($SITEBASE . '/inc/sgroup_list.php');
            $result= array_merge($result, sgroup_list());
            return $result;
            /*if (Helpers::check_if_game_client()) {
                header("Location: ".$INGAME_WEBPATH."?page=sgroup_list");
            }else{
                header("Location: ".$WEBPATH."?page=sgroup_list");
            }
            exit;
            */
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
