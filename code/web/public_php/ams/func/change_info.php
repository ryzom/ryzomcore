<?php
/**
* This function is beign used to change the users personal info.
* It will first check if the user who executed this function is the person of whom the information is or if it's a mod/admin. If this is not the case the page will be redirected to an error page.
* afterwards the current info will be loaded, which will be used to determine what to update. After updating the information, the settings template will be reloaded. Errors made by invalid data will be shown
* also after reloading the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function change_info(){

 try{
	//if logged in
        if(WebUsers::isLoggedIn()){

            if(isset($_POST['target_id'])){

                // check if the user who executed this function is the person of whom the information is or if it's a mod/admin.
                if(  ($_POST['target_id'] == $_SESSION['id']) || Ticket_User::isMod(unserialize($_SESSION['ticket_user']) ) ){
                    if($_POST['target_id'] == $_SESSION['id']){
			//if the info is of the executing user himself
                        $target_username = $_SESSION['user'];
                    }else{
			//if the info is from someone else.
			$webUser = new WebUsers($_POST['target_id']);
                        $target_username = $webUser->getUsername();
                    }

                    $webUser = new WebUsers($_POST['target_id']);
                    //use current info to check for changes
                    $current_info = $webUser->getInfo();

		    $current_info['FirstName'] = filter_var($current_info['FirstName'], FILTER_SANITIZE_STRING);
		    $current_info['LastName'] = filter_var($current_info['LastName'], FILTER_SANITIZE_STRING);
		    $current_info['Country'] = filter_var($current_info['Country'], FILTER_SANITIZE_STRING);
		    $current_info['Gender'] = filter_var($current_info['Gender'], FILTER_SANITIZE_NUMBER_INT);


                    $updated = false;
                    $values = Array();
                    $values['user'] = $target_username;

		    //make the query that will update the data.
                    $query = "UPDATE ams_user SET ";
                    if(($_POST['FirstName'] != "") && ($_POST['FirstName'] != $current_info['FirstName'])){
                        $query = $query . "FirstName = :fName ";
                        $updated = true;
                        $values['fName'] = filter_var($_POST['FirstName'], FILTER_SANITIZE_STRING);
                    }
                    if(($_POST['LastName'] != "") && ($_POST['LastName'] != $current_info['LastName'])){
			if($updated){
			 $query = $query . ", LastName = :lName ";
			}else{
			 $query = $query . "LastName = :lName ";
			}
                        $updated = true;
                        $values['lName'] = filter_var($_POST['LastName'], FILTER_SANITIZE_STRING);
                    }
		    if(($_POST['Country'] != "AA") && ($_POST['Country'] != $current_info['Country'])){
			if($updated){
			 $query = $query . ", Country = :country ";
			}else{
			 $query = $query . "Country = :country ";
			}
                        $updated = true;
                        $values['country'] = filter_var($_POST['Country'], FILTER_SANITIZE_STRING);
		    }
		    if($_POST['Gender'] != $current_info['Gender']){
			if($updated){
			 $query = $query . ", Gender = :gender ";
			}else{
			 $query = $query . "Gender = :gender ";
			}
                        $updated = true;
                        $values['gender'] = filter_var($_POST['Gender'], FILTER_SANITIZE_NUMBER_INT);
		    }
                    //finish the query!
                    $query = $query . "WHERE Login = :user";

                    //if some field is update then:
                    if($updated){
                        //execute the query in the web DB.
                        $dbw = new DBLayer("web");
                        $dbw->execute($query,$values);
                    }

		    //reload the settings inc function before recalling the settings template.
                    global $SITEBASE;
                    require_once($SITEBASE . '/inc/settings.php');
                    $result = settings();
                    if($updated){
                        $result['info_updated'] = "OK";
                    }
                    $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
                    $result['username'] = $_SESSION['user'];
                    $result['no_visible_elements'] = 'FALSE';
                    $result['target_id'] = $_POST['target_id'];
		    global $INGAME_WEBPATH;
                    $result['ingame_webpath'] = $INGAME_WEBPATH;
                    helpers :: loadtemplate( 'settings', $result);
                    die();

                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    die();
                }

            }else{
                //ERROR: The form was not filled in correclty
		header("Location: index.php?page=settings");
		die();
            }
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    die();
        }

    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         die();
    }
}
