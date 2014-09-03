<?php
/**
* This function is beign used to load info that's needed for the show_sgroup page.
* check if the person browsing this page is a mod/admin, if not he'll be redirected to an error page.
* if the $_GET['delete'] var is set and the user executing is an admin, an entry will be deleted out of the support group.
* A list of users that are member of the group will be returned, which can be used by the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function show_sgroup(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    //if logged in
    if(WebUsers::isLoggedIn()){
        if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
            if( isset($_GET['id'])){
                //['target_id'] holds the id of the group!
                $result['target_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);

                //if the $_GET['delete'] var is set and the user executing is an admin, an entry will be deleted out of the support group.
                if(isset($_GET['delete']) && Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                    $delete_id = filter_var($_GET['delete'], FILTER_SANITIZE_NUMBER_INT);
                    $result['delete'] = Support_Group::deleteUserOfSupportGroup( $delete_id, $result['target_id']  );
                    if (Helpers::check_if_game_client()) {
                        header("Location: ".$INGAME_WEBPATH."?page=show_sgroup&id=" . $result['target_id']);
                    }else{
                        header("Location: ".$WEBPATH."?page=show_sgroup&id=" . $result['target_id']);
                    }
                    die();

                }

                if(Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                    $result['isAdmin'] = "TRUE";
                }

                $group = Support_Group::getGroup($result['target_id']);
                $result['groupsname'] = $group->getName();
                $result['groupemail'] = $group->getGroupEmail();
                $result['imap_mailserver'] = $group->getIMAP_MailServer();
                $result['imap_username'] = $group->getIMAP_Username();
                $result['userlist'] = Gui_Elements::make_table(Support_Group::getAllUsersOfSupportGroup($result['target_id']), Array("getTUserId","getPermission","getExternId"), Array("tUserId","permission","externId"));
                $i = 0;
                foreach( $result['userlist'] as $user){
                    $webuser = new Webusers($user['externId']);
                    $result['userlist'][$i]['name'] = $webuser->getUsername();
                    $i++;
                }
                global $INGAME_WEBPATH;
                $result['ingame_webpath'] = $INGAME_WEBPATH;
                $result['teamlist'] = Gui_Elements::make_table(Ticket_User::getModsAndAdmins(), Array("getTUserId","getExternId"), Array("tUserId","externId"));
                $i = 0;
                foreach( $result['teamlist'] as $member){
                    $web_teammember = new Webusers($member['externId']);
                    if (!In_Support_Group::userExistsInSGroup($member['externId'], $result['target_id'])) {
                        $result['users'][$i]['name'] = $web_teammember->getUsername();
                    }
                    $i++;
                }

                return $result;


            }else{

                //ERROR: No page specified!
                $_SESSION['error_code'] = "404";
                header("Location: ams?page=error");
                die();
            }

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
