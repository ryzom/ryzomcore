<?php

function add_user(){
     $result = Users :: check_Register();
     //print_r($result);
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
          //header( 'Location: email_sent.php' );
          $status = write_user( $edit );
          $pageElements['status'] = $status;
          //TODO: perhaps send email!
          $pageElements['no_visible_elements'] = 'TRUE';
          helpers :: loadtemplate( 'register_feedback', $pageElements);
          exit;
     }else{
          // pass error
          $result['prevUsername'] = $_POST["Username"];
          $result['prevPassword'] = $_POST["Password"];
          $result['prevConfirmPass'] = $_POST["ConfirmPass"];
          $result['prevEmail'] = $_POST["Email"];
          $result['no_visible_elements'] = 'TRUE';
          helpers :: loadtemplate( 'register', $result);
          exit;
     }
}


function write_user($newUser){
     
     //get the db specifics out of the config file
     global $cfg;
     
     //create salt here, because we want it to be the same on the web/server
     $hashpass = crypt($newUser["pass"], Users::generateSALT());
     
     $params = array(
          'name' => $newUser["name"],
          'pass' => $hashpass,
          'mail' => $newUser["mail"]      
     );
     
     //print_r($params);
     //make a $values array for passing all data to the Users::createUser() function.
     $values["params"] = $params;
     $values["db"] =  $cfg['db'];
     
     //Create the user on the shard + in case shard is offline put copy of query in query db
     //returns: ok, shardoffline or liboffline
     $result = Users :: createUser($values);
  
     try{
          //make connection with web db and put it in there
          $dbw = new DBLayer($cfg['db']['web']);
          $dbw->execute("INSERT INTO ams_user (Login, Password, Email) VALUES (:name, :pass, :mail)",$params);
          
     }catch (PDOException $e) {
      //go to error page or something, because can't access website db
      print_r($e);
      exit;
     }
     
     return $result;

}
