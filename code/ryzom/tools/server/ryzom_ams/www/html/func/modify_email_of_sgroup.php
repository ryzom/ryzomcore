<?php

function modify_email_of_sgroup(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    if(WebUsers::isLoggedIn()){
        
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
            if (Helpers::check_if_game_client()) {
                header("Location: ".$INGAME_WEBPATH."?page=show_sgroup&id=".$sgroupid);
            }else{
                header("Location: ".$WEBPATH."?page=show_sgroup&id=".$sgroupid);
            }
            exit;
            
        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            exit;
        }
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }

}