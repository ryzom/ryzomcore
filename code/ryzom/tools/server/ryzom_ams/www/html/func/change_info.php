<?php

function change_info(){
    
 try{
	//if logged in
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
		
                
                if(  ($_POST['target_id'] == $_SESSION['id']) || Ticket_User::isMod(unserialize($_SESSION['ticket_user']) ) ){
                    if($_POST['target_id'] == $_SESSION['id']){
                        $target_username = $_SESSION['user'];
                    }else{
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
                    exit;
                    
                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    exit;
                }
        
            }else{
                //ERROR: The form was not filled in correclty
		header("Location: index.php?page=settings");
		exit;
            }    
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    exit;
        }
                  
    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         exit;
    }   
}
