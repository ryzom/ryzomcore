<?php
/**
* This function is beign used to modify the email related to a support group.
* It will first check if the user who executed this function is an admin. If this is not the case the page will be redirected to an error page.
* the new email will be validated and in case it's valid we'll add it to the db. Before adding it, we will encrypt the password by using the MyCrypt class. Afterwards the password gets
* updated and the page redirected again.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function modify_email_of_sgroup(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    if(WebUsers::isLoggedIn()){

        //check if user is an admin
        if( Ticket_User::isAdmin(unserialize($_SESSION['ticket_user'])) &&  isset($_POST['target_id'])){

            $sgroupid = filter_var($_POST['target_id'],FILTER_SANITIZE_NUMBER_INT);
            $group = Support_Group::getGroup($sgroupid);
            $groupemail = filter_var($_POST['GroupEmail'],FILTER_SANITIZE_STRING);
            if(Users::validEmail($groupemail) || $groupemail == ""){
                $password = filter_var($_POST['IMAP_Password'],FILTER_SANITIZE_STRING);
                $group->setGroupEmail($groupemail);
                $group->setIMAP_MailServer(filter_var($_POST['IMAP_MailServer'],FILTER_SANITIZE_STRING));
                $group->setIMAP_Username(filter_var($_POST['IMAP_Username'],FILTER_SANITIZE_STRING));

                //encrypt password!
                global $cfg;
                $crypter = new MyCrypt($cfg['crypt']);
                $enc_password = $crypter->encrypt($password);
                $group->setIMAP_Password($enc_password);
                $group->update();
                $result['RESULT_OF_MODIFYING'] = "SUCCESS";
                if($password == ""){
                    $result['RESULT_OF_MODIFYING'] = "NO_PASSWORD";
                }
            }else{
                $result['RESULT_OF_MODIFYING'] = "EMAIL_NOT_VALID";
            }

            $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
            $result['no_visible_elements'] = 'FALSE';
            $result['username'] = $_SESSION['user'];
            //global $SITEBASE;
            //require_once($SITEBASE . 'inc/show_sgroup.php');
            //$result= array_merge($result, show_sgroup());
            //helpers :: loadtemplate( 'show_sgroup', $result);
                header("Cache-Control: max-age=1");
            if (Helpers::check_if_game_client()) {
                header("Location: ".$INGAME_WEBPATH."?page=show_sgroup&id=".$sgroupid);
            }else{
                header("Location: ".$WEBPATH."?page=show_sgroup&id=".$sgroupid);
            }
            throw new SystemExit();

        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
                header("Cache-Control: max-age=1");
            header("Location: index.php?page=error");
            throw new SystemExit();
        }
    }else{
        //ERROR: not logged in!
                header("Cache-Control: max-age=1");
        header("Location: index.php");
        throw new SystemExit();
    }

}
