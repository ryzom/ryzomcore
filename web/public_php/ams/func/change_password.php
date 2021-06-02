<?php
/**
* This function is beign used to change the users password.
* It will first check if the user who executed this function is the person of whom the emailaddress is or if it's a mod/admin. If this is not the case the page will be redirected to an error page.
* If the executing user tries to change someone elses password, he doesn't has to fill in the previous password. The password will be validated first. If the checking was successful the password will be updated and the settings template will be reloaded. Errors made by invalid data will be shown
* also after reloading the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function change_password(){

    try{
        //if logged in
        if(WebUsers::isLoggedIn()){

            if(isset($_POST['target_id'])){
                $adminChangesOther = false;
                //if target_id is the same as session id or is admin
                if(  ($_POST['target_id'] == $_SESSION['id']) ||  Ticket_User::isMod(unserialize($_SESSION['ticket_user']))  ){
                    if($_POST['target_id'] == $_SESSION['id']){
			//if the password is of the executing user himself
                        $target_username = $_SESSION['user'];
                    }else{
			//if the password is of someone else.
			$webUser = new WebUsers($_POST['target_id']);
                        $target_username = $webUser->getUsername();
                        //isAdmin is true when it's the admin, but the target_id != own id
                        $adminChangesOther = true;
                        $_POST["CurrentPass"] = "dummypass";
                    }

                    $webUser = new WebUsers($_POST['target_id']);
                    $params = Array( 'user' => $target_username, 'CurrentPass' => $_POST["CurrentPass"], 'NewPass' => $_POST["NewPass"], 'ConfirmNewPass' => $_POST["ConfirmNewPass"], 'adminChangesOther' => $adminChangesOther);
                    $result = $webUser->check_change_password($params);
                    if ($result == "success"){
                        //edit stuff into db
                        global $SITEBASE;
                        require_once($SITEBASE . '/inc/settings.php');
                        $succresult = settings();
                        $status = WebUsers::setPassword($target_username, $_POST["NewPass"]);
                        if($status == 'ok'){
                            $succresult['SUCCESS_PASS'] = "OK";
                        }else if($status == 'shardoffline'){
                             $succresult['SUCCESS_PASS'] = "SHARDOFF";
                        }
                        $succresult['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
                        $succresult['no_visible_elements'] = 'FALSE';
                        $succresult['username'] = $_SESSION['user'];
                        $succresult['target_id'] = $_POST['target_id'];
                        helpers :: loadtemplate( 'settings', $succresult);
                        throw new SystemExit();

                    }else{

                        $result['prevCurrentPass'] = filter_var($_POST["CurrentPass"], FILTER_SANITIZE_STRING);
                        $result['prevNewPass'] = filter_var($_POST["NewPass"], FILTER_SANITIZE_STRING);
                        $result['prevConfirmNewPass'] = filter_var($_POST["ConfirmNewPass"], FILTER_SANITIZE_STRING);
                        $result['permission'] =  unserialize($_SESSION['ticket_user'])->getPermission();
                        $result['no_visible_elements'] = 'FALSE';
                        $result['username'] = $_SESSION['user'];
                        $result['target_id'] = $_POST['target_id'];

                        global $SITEBASE;
                        require_once($SITEBASE . '/inc/settings.php');
                        $settings = settings();

                        $result = array_merge($result,$settings);
                        helpers :: loadtemplate( 'settings', $result);
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
		header("Location: index.php?page=settings");
		throw new SystemExit();
            }
        }else{
            //ERROR: user is not logged in
                header("Cache-Control: max-age=1");
	    header("Location: index.php");
	    throw new SystemExit();
        }

    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         throw new SystemExit();
    }

}

