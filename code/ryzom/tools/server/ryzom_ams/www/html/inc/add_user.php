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
          write_user( $edit );
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
     global $WEBDBHOST;
     global $WEBDBPORT;
     global $WEBDBNAME;
     global $WEBDBUSERNAME;
     global $WEBDBPASSWORD;
     global $SHARDDBHOST;
     global $SHARDDBPORT;
     global $SHARDDBNAME; 
     global $SHARDDBUSERNAME;
     global $SHARDDBPASSWORD;
     
     $values["name"] = $newUser["name"];
     $values["pass"] = $newUser["pass"];
     $values["mail"] = $newUser["mail"];
     
     $values["webhost"] =  $WEBDBHOST;
     $values["webport"] =  $WEBDBPORT;
     $values["webdbname"] = $WEBDBNAME;
     $values["webusername"] = $WEBDBUSERNAME;
     $values["webpassword"] = $WEBDBPASSWORD ;
 
     $values["shardhost"] = $SHARDDBHOST;
     $values["shardport"] = $SHARDDBPORT;
     $values["sharddbname"] = $SHARDDBNAME;
     $values["shardusername"] = $SHARDDBUSERNAME;
     $values["shardpassword"] = $SHARDDBPASSWORD;
     
     
     $result = Users :: createUser($values);
    
     print('Awesome');
     }

