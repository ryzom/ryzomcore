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
     $login = $newUser["name"];
     $pass = $newUser["pass"];
     $email = $newUser["mail"];
 
     $salt = Users::generateSALT();
     $hashpass = crypt($pass, $salt);
 
     $params = array(
         $login,
         $hashpass,
         $email
     );
     
     $result = Users :: create_Server_User($params);
     //test purpose
     $result = "fail";
     
     $hostname = 'localhost';
     $port     = '3306';
     $dbname   = 'ryzom_ams';
     $username = 'shard';
     $password = '';
     
     $dbh = new PDO("mysql:host=$hostname;port=$port;dbname=$dbname", $username, $password);
     $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
     try {
          $statement = $dbh->prepare("INSERT INTO ams_user (Login, Password, Email) VALUES (?, ?, ?)");
          $statement->execute($params);
          
          if($result == "fail"){
               print('so far');
               $params = array("type" => "createUser","query" => json_encode(array($login,$pass,$email)));
               $statement = $dbh->prepare("INSERT INTO ams_querycache (type, query) VALUES (:type, :query)");
               $statement->execute($params);
          }
     }
     catch (PDOException $e) {
           //go to error page or something
           print_r($e);
           exit;
     }
    
 
     // add user locally here
     print('Awesome');
     }

