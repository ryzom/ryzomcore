<?php
/**
* handles CMS/WWW related functions regarding user management & registration.
* inherits from the Users class. The methods of this class have to be rewritten according to the CMS's functionality that you wish to use.
* The drupal_module has a webusers class of its own in the module itself.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class WebUsers extends Users{

       private $uId; /**< The user id */ 
       private $login; /**< The username */ 
       private $email; /**< The email address */ 
       private $firstname; /**< The users first name */ 
       private $lastname; /**< The users last name */ 
       private $gender; /**< The gender */ 
       private $country; /**< 2 letter word matching the country of the user */ 
       private $receiveMail; /**< configuration regarding if the user wants to receive email notifications or not. */ 
       private $language; /**< Language of the user */ 
       
       /**
       * A constructor.
       * loads the object with the UID, if none is given it will use 0.
       * @param $UID the UID of the user you want to instantiate.
       */
       function __construct($UId = 0) {
              $this->uId = $UId;
       }
       
       
       /**
       * sets the object's attributes.
       * @param $values should be an array.
       */
       public function set($values){
              $this->uId = $values['uid'];
              $this->login = $values['name'];
              $this->email = $values['mail'];
              //$this->firstname = $values['FirstName'];
              //$this->lastname = $values['LastName'];
              //$this->gender = $values['Gender'];
              //$this->country = $values['Country'];
              //$this->receiveMail = $values['ReceiveMail'];
              //$this->language = $values['Language'];
       }
    

       /**
       * function that checks if a username exists already or not.
       * This function overrides the function of the base class.
       * @takes $username the username in question
       * @return string Info: Returns 0 if the user is not in the web db, else a positive number is returned.
       */
       protected function checkUserNameExists($username){
              return db_query("SELECT COUNT(*) FROM {users} WHERE name = :name", array(':name' => $username))->fetchField();  
       }
    
    
       /**
       * function that checks if a email exists already or not.
       * This function overrides the function of the base class.
       * @takes $email the email address in question.
       * @return string Info: Returns 0 if the email address is not in the web db, else a positive number is returned.
       */
       protected function checkEmailExists($email){
              return db_query("SELECT COUNT(*) FROM {users} WHERE mail = :mail", array(':mail' => $email))->fetchField();
       }
     
     
       /**
       * check if the login username and password match the db.
       * @param $username the inserted username
       * @param $password the inserted password (unhashed)
       * @return the logged in user's db row as array if login was a success, else "fail" will be returned.
       */
       public function checkLoginMatch($username,$password){
         if(!user_authenticate($username, $password)){
                return 'fail';
         }else{
                return db_query("SELECT * FROM {users} WHERE name = :name", array(':name' => $username))->fetchAssoc();
         }
       }
       
       
       /**
       * returns te id for a given username
       * @param $username the username
       * @return the user's id linked to the username
       */
       public static function getId($username){
         $row = db_query("SELECT * FROM {users} WHERE name = :name", array(':name' => $username))->fetchAssoc(); 
         return $row['uid'];
       }
    
    
       /**
       * returns te id for a given emailaddress
       * @param $email the emailaddress
       * @return the user's id linked to the emailaddress
       */
       public static function getIdFromEmail($email){
          $dbw = new DBLayer("web");  
          $statement = $dbw->execute("SELECT * FROM ams_user WHERE Email=:email", array('email' => $email));
          $row = $statement->fetch();
          if(!empty($row)){
              return $row['UId'];
          }else{
              return "FALSE";
          }
       }
    
    
       /**
       * get uId attribute of the object.
       */
       public function getUId(){
              return $this->uId;
       }


       /**
       * get login attribute of the object.(username)
       */
       public function getUsername(){
                 
          if(! isset($this->login) || $this->login == ""){
                 $row = db_query("SELECT * FROM {users} WHERE uid = :id", array(':id' => $this->uId))->fetchAssoc();
                 $this->set($row);
          }
          return $this->login;
       }
       
       
       /**
       * get email attribute of the object.
       */ 
       public function getEmail(){
          if(! isset($this->email) || $this->email == ""){
                 $row = db_query("SELECT * FROM {users} WHERE uid = :id", array(':id' => $this->uId))->fetchAssoc();
                 $this->set($row);
          }
          return $this->email;
       }
       
       
       /**
       * get basic info of the object.
       * @return returns an array in the form of Array('FirstName' => $this->firstname, 'LastName' => $this->lastname, 'Gender' => $this->gender, 'Country' => $this->country, 'ReceiveMail' => $this->receiveMail)
       */
       public function getInfo(){
          $dbw = new DBLayer("web");
          if(! (isset($this->firstname) && isset($this->lastname) && isset($this->gender) && isset($this->country) && isset($this->receiveMail) ) ||
             $this->firstname == "" || $this->lastname == "" || $this->gender == "" || $this->country == "" || $this->receiveMail == ""){
                $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
                $row = $statement->fetch();
                $this->set($row);
          }
          $result = Array('FirstName' => $this->firstname, 'LastName' => $this->lastname, 'Gender' => $this->gender, 'Country' => $this->country, 'ReceiveMail' => $this->receiveMail);
          return $result;
       }
    
    
       /**
       * get receiveMail attribute of the object.
       */
       public function getReceiveMail(){
              $dbw = new DBLayer("web");
              if(! isset($this->receiveMail) || $this->receiveMail == ""){
                     $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
                     $row = $statement->fetch();
                     $this->set($row);
              }
              return $this->receiveMail;
       }
    
    
       /**
       * get language attribute of the object.
       */
       public function getLanguage(){
              return $DEFAULT_LANGUAGE;
       }
    
       
       /**
       * check if the user is logged in.
       * @return true or false
       */
       public function isLoggedIn(){
           if(isset($_SESSION['user'])){
               return true;
           }
           return false;
       }
    
    
       /**
       * update the password.
       * update the password in the shard + update the password in the www/CMS version.
       * @param $user the username
       * @param $pass the new password.
       * @return ok if it worked, if the lib or shard is offline it will return liboffline or shardoffline.
       */
       public function setPassword($user, $pass){
           $hashpass = crypt($pass, WebUsers::generateSALT());
           $reply = WebUsers::setAmsPassword($user, $hashpass);
           $drupal_pass = user_hash_password($pass);
           $values = Array('user' => $user, 'pass' => $drupal_pass);
            try {
                  //make connection with and put into shard db
                  db_query("UPDATE {users} SET pass = :pass WHERE name = :user", $values);
             }
             catch (PDOException $e) {
               //ERROR: the web DB is offline
             }
           return $reply;
       }
    
    
       /**
       * update the emailaddress.
       * update the emailaddress in the shard + update the emailaddress in the www/CMS version.
       * @param $user the username
       * @param $mail the new emailaddress.
       * @return ok if it worked, if the lib or shard is offline it will return liboffline or shardoffline.
       */
       public function setEmail($user, $mail){
          $reply = WebUsers::setAmsEmail($user, $mail);
          $values = Array('user' => $user, 'mail' => $mail);
           try {
                 //make connection with and put into shard db
                 db_query("UPDATE {users} SET mail = :mail WHERE name = :user", $values);
                 
            }
            catch (PDOException $e) {
              //ERROR: the web DB is offline
            }
          return $reply;
       }
      
      
       /**
       * update the setReceiveMail value in the db.
       * update the receiveMail in the www/CMS version.
       * @param $user the username
       * @param $receivemail the receivemail setting .
       */
       public static function setReceiveMail($user, $receivemail){
          $values = Array('user' => $user, 'receivemail' => $receivemail);
           try {
                 //make connection with and put into shard db
                 $dbw = new DBLayer("web");
                 $dbw->execute("UPDATE ams_user SET ReceiveMail = :receivemail WHERE UId = :user ",$values);
            }
            catch (PDOException $e) {
              //ERROR: the web DB is offline
            }
       }
    
    
       /**
       * update the language value in the db.
       * update the language in the www/CMS version.
       * @param $user the username
       * @param $language the new language value.
       */
       public static function setLanguage($user, $language){
              $values = Array('user' => $user, 'language' => $language);
               try {
                     //make connection with and put into shard db
                     $dbw = new DBLayer("web");
                     $dbw->execute("UPDATE ams_user SET Language = :language WHERE UId = :user ",$values);
                }
                catch (PDOException $e) {
                  //ERROR: the web DB is offline
                }
       }
    
    
       /**
       * return all users.
       * @return return an array of users
       */
       public function getUsers(){
           $dbl = new DBLayer("web");
           $data = $dbl->executeWithoutParams("SELECT * FROM ams_user");
           return $data;
       }
    
    
       /**
       * return the query that should get all users.
       * @return string: the query to receive all users.
       */
       public static function getAllUsersQuery(){
          return "SELECT * FROM users WHERE `uid` > 0";
       }
       
       
       /**
       * creates a webuser.
       * it will set the language matching to the language cookie setting and add it to the www/CMS's DB.
       * @param $name the username
       * @param $pass the unhashed password
       * @param $mail the email address
       */
       public static function createWebuser($name, $pass, $mail){
          
          //register account with the correct language (check if cookie is already set)!
          if ( isset( $_COOKIE['Language'] ) ) { 
                 $lang = $_COOKIE['Language'];
          }else{
                 global $DEFAULT_LANGUAGE;
                 $lang = $DEFAULT_LANGUAGE;
          }
          
          $values = Array('name' => $name, 'pass' => $pass, 'mail' => $mail, 'lang' => $lang);
          
          try {
             $dbw = new DBLayer("web");
             return $dbw->executeReturnId("INSERT INTO ams_user (Login, Password, Email, Language) VALUES (:name, :pass, :mail, :lang)",$values);
          }
          catch (PDOException $e) {
               //ERROR: the web DB is offline
          }
       }
       
}