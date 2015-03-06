<?php
/**
* This function is beign used to add a new user to the www database.
* it will first check if the sent $_POST variables are valid for registering, if one or more rules are broken (eg the username is too short) the template will be reloaded
* but this time with the appropriate error messages. If the checking was successful it will call the write_user() function (located in this same file). That function will create
* a new www user and matching ticket_user. It will also push the newly created user to the shard. In case the shard is offline, the new user will be temporary stored in the ams_querycache,
* waiting for the sync cron job to update it.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function add_user(){
     global $INGAME_WEBPATH;
     $params = Array('Username' =>  $_POST["Username"], 'Password' =>  $_POST["Password"], 'ConfirmPass' =>  $_POST["ConfirmPass"], 'Email' =>  $_POST["Email"]);
     $webUser = new WebUsers();
     
     //check if the POST variables are valid, before actual registering
     $result = $webUser->check_Register($params);

     global $SITEBASE;
     require_once($SITEBASE . '/inc/settings.php');
     // if all are good then create user
     if ( $result == "success"){
          $edit = array(
             'name' => $_POST["Username"],
              'pass' => $_POST["Password"],
              'mail' => $_POST["Email"],
              'init' => $_POST["Email"],
              'unhashpass' => $_POST["Password"],
              'status' => 1,
              'access' => $_SERVER['REQUEST_TIME']
              );
          $status = write_user( $edit );
          if(Helpers::check_if_game_client()){
               //if registering ingame then we have to set the header and dont need to reload the template.
               header('Location: email_sent.php');
               throw new SystemExit();
          }
          $pageElements = settings();
          $pageElements['ingame_webpath'] = $INGAME_WEBPATH;
          $pageElements['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
          $pageElements['SUCCESS_ADD'] = $status;
          if (isset($_GET['page']) && $_GET['page']=="settings"){
            helpers :: loadtemplate( 'settings', $pageElements);
          }else{
            $pageElements['no_visible_elements'] = 'TRUE';
            helpers :: loadtemplate( 'register_feedback', $pageElements);
          }
          throw new SystemExit();
     }elseif (isset($_GET['page']) && $_GET['page']=="settings"){
          $pageElements = array_merge(settings(), $result);
          // pass error and reload template accordingly
          $pageElements['prevUsername'] = $_POST["Username"];
          $pageElements['prevPassword'] = $_POST["Password"];
          $pageElements['prevConfirmPass'] = $_POST["ConfirmPass"];
          $pageElements['prevEmail'] = $_POST["Email"];
          $pageElements['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
          $pageElements['do'] = "add_user";
          helpers :: loadtemplate( 'settings', $pageElements);
          throw new SystemExit();
     }else{
          // pass error and reload template accordingly
          $result['prevUsername'] = $_POST["Username"];
          $result['prevPassword'] = $_POST["Password"];
          $result['prevConfirmPass'] = $_POST["ConfirmPass"];
          $result['prevEmail'] = $_POST["Email"];
          $result['no_visible_elements'] = 'TRUE';
          $pageElements['ingame_webpath'] = $INGAME_WEBPATH;
          helpers :: loadtemplate( 'register', $result);
          throw new SystemExit();
     }
}

//use the valid userdata to create the new user.
function write_user($newUser){
              
     //create salt here, because we want it to be the same on the web/server
     $hashpass = crypt($newUser["pass"], WebUsers::generateSALT());
     
     $params = array(
    	  'Login' => $newUser["name"],
          'Password' => $hashpass,
          'Email' => $newUser["mail"]      
     );
     try{
          //make new webuser
          $user_id = WebUsers::createWebuser($params['Login'], $params['Password'], $params['Email']);
               
          //Create the user on the shard + in case shard is offline put copy of query in query db
          //returns: ok, shardoffline or liboffline
          $result = WebUsers::createUser($params, $user_id);
          Users::createPermissions(array($newUser["name"]));
    
          
     }catch (PDOException $e) {
      //go to error page or something, because can't access website db
      print_r($e);
      throw new SystemExit();
     }
     
     return $result;

}
