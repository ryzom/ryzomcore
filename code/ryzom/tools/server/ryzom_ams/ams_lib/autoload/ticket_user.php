<?php
/**
* user entry point in the ticket system.
* The ticket_user makes a link between the entire ticket system's lib db and the www user, which is stored in another db (this is the external ID).
* The externalID could be the ID of a drupal user or wordpress user,.. The ticket_user also stores the permission of that user, this way the permission system
* is inside the lib itself and can be used in any www version that you like. permission 1 = user, 2 = mod, 3 = admin.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_User{
    
    private $tUserId;  /**< The id of the user inside the ticket system*/ 
    private $permission;  /**< The permission of the user */ 
    private $externId;  /**< The id of the user account in the www (could be drupal,...) that is linked to the ticket_user */ 
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /**
    * create a new ticket user.
    * @param $extern_id the id of the user account in the www version (drupal,...)
    * @param $permission the permission that will be given to the user. 1=user, 2=mod, 3=admin
    */
    public static function createTicketUser( $extern_id, $permission) {
        $dbl = new DBLayer("lib");
	$dbl->insert("ticket_user",array('Permission' => $permission, 'ExternId' => $extern_id));
    }
    
    
    /**
    * check if a ticket_user object is a mod or not.
    * @param $user the ticket_user object itself
    * @return true or false
    */
    public static function isMod($user){
        if(isset($user) && $user->getPermission() > 1){
            return true;
        }
        return false;
    }
    
    
    /**
    * check if a ticket_user object is an admin or not.
    * @param $user the ticket_user object itself
    * @return true or false
    */
    public static function isAdmin($user){
        if(isset($user) && $user->getPermission() == 3){
            return true;
        }
        return false;
    }
    
    
    /**
    * return constructed ticket_user object based on TUserId.
    * @param $id the TUserId of the entry.
    * @return constructed ticket_user object
    */
    public static function constr_TUserId( $id) {
        $instance = new self();
        $instance->setTUserId($id);
        return $instance;
    
    }
    
    
    /**
    * return a list of all mods/admins.
    * @return an array consisting of ticket_user objects that are mods & admins.
    */
    public static function getModsAndAdmins() {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("ticket_user", array(null), "`Permission` > 1" );
        $rows = $statement->fetchAll();
        $result = Array();
        foreach($rows as $user){
            $instanceUser = new self();
            $instanceUser->set($user);
            $result[] = $instanceUser;
        }
        return $result; 
    }
    
    
    /**
    * return constructed ticket_user object based on ExternId.
    * @param $id the ExternId of the entry.
    * @return constructed ticket_user object
    */
    public static function constr_ExternId( $id) {
        $instance = new self();
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("ticket_user" ,array('id'=>$id) ,"ExternId=:id");
        $row = $statement->fetch();
        $instance->tUserId = $row['TUserId'];
        $instance->permission = $row['Permission'];
        $instance->externId = $row['ExternId'];
        return $instance;
    }
    
    
    /**
    * change the permission of a ticket_user.
    * @param $user_id the TUserId of the entry.
    * @param $perm the new permission value.
    */
    public static function change_permission($user_id, $perm){
        $user = new Ticket_User();
        $user->load_With_TUserId($user_id);
        $user->setPermission($perm);
        $user->update();
    }
    
    
    /**
    * return the email address of a ticket_user.
    * @param $id the TUserId of the entry.
    * @return string containing the email address of that user.
    */
    public static function get_email_by_user_id($id){
        $user = new Ticket_User();
        $user->load_With_TUserId($id);
        $webUser = new WebUsers($user->getExternId());
        return $webUser->getEmail();      
    }
    
    
    /**
    * return the username of a ticket_user.
    * @param $id the TUserId of the entry.
    * @return string containing username of that user.
    */
    public static function get_username_from_id($id){
        $user = new Ticket_User();
        $user->load_With_TUserId($id);
        $webUser = new WebUsers($user->getExternId());
        return $webUser->getUsername();   
    }
    
    
    /**
    * return the TUserId of a ticket_user by giving a username.
    * @param $username the username of a user.
    * @return the TUserId related to that username.
    */
    public static function get_id_from_username($username){
        $externId = WebUsers::getId($username);
        $user = Ticket_User::constr_ExternId($externId);
        return $user->getTUserId();   
    }
    
    /**
    * return the ticket_user id from an email address.
    * @param $email the emailaddress of a user.
    * @return the ticket_user id related to that email address, in case none, return "FALSE".
    */
    public static function get_id_from_email($email){
        $webUserId = WebUsers::getIdFromEmail($email);
        if($webUserId != "FALSE"){
            $user = Ticket_User::constr_ExternId($webUserId);
            return $user->getTUserId();
        }else{
            return "FALSE";
        }
    }
    
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    
    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }
    
    
    /**
    * sets the object's attributes.
    * @param $values should be an array of the form array('TUserId' => id, 'Permission' => perm, 'ExternId' => ext_id).
    */
    public function set($values) {
        $this->setTUserId($values['TUserId']);
        $this->setPermission($values['Permission']);
        $this->setExternId($values['ExternId']);
    }
    
    
    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a TUserId.
    * @param $id the id of the ticket_user that should be loaded
    */
    public function load_With_TUserId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("ticket_user" ,array('id'=>$id), "TUserId=:id" );
        $row = $statement->fetch();
        $this->tUserId = $row['TUserId'];
        $this->permission = $row['Permission'];
        $this->externId = $row['ExternId'];
    } 
    
    
    /**
    * update the object's attributes to the db.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $dbl->update("ticket_user" ,array('Permission' => $this->permission, 'ExternId' => $this->externId) ,"TUserId=$this->tUserId");
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get permission attribute of the object.
    */
    public function getPermission(){
        return $this->permission;
    }
   
    /**
    * get externId attribute of the object.
    */
    public function getExternId(){
        return $this->externId;
    }
    
    /**
    * get tUserId attribute of the object.
    */
    public function getTUserId(){
        return $this->tUserId;
    }
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    /**
    * set permission attribute of the object.
    * @param $perm integer that indicates the permission level. (1= user, 2= mod, 3= admin)
    */
    public function setPermission($perm){
        $this->permission = $perm;
    }
   
   
    /**
    * set externId attribute of the object.
    * @param $id the external id.
    */  
    public function setExternId($id){
        $this->externId = $id;
    }
    
    /**
    * set tUserId attribute of the object.
    * @param $id the ticket_user id
    */
    public function setTUserId($id){
        $this->tUserId= $id;
    }
    
    
}
