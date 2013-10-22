<?php
/**
* groups moderators & admins together. A Support Group is a group of people with the same skills or knowledge. A typical example will be the (Developers group, webteam group, management, etc..)
* The idea is that tickets can be forwarded to a group of persons that might be able to answer that specific question. Support Groups are also the key of handling the emails, because the email addresses
* of the support groups will be used by the Mail_Handler class.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Support_Group{
    
    private $sGroupId;  /**< The id of the support group */ 
    private $name;  /**< The name of the support group */ 
    private $tag; /**< The tag of the support group, a tag is max 4 letters big, and will be used in the future as easy reference to indicate what group it is refered to (eg [DEV]) */ 
    private $groupEmail; /**< The email address of the group */ 
    private $iMAP_MailServer; /**< The imap server connection string */ 
    private $iMAP_Username; /**< The imap username of the account */ 
    private $iMAP_Password; /**< The imap matching password*/ 
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /**
    * return a specific support_group object.
    * @param $id the id of the support group that we want to return
    * @return a support_group object.
    */
    public static function getGroup($id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM support_group WHERE SGroupId = :id", array('id' => $id));
        $row = $statement->fetch();
        $instanceGroup = new self();
        $instanceGroup->set($row);
        return $instanceGroup;
    
    }
    
    
    /**
    * return all support_group objects.
    * @return an array containing all support_group objects.
    */
    public static function getGroups() {
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM support_group ORDER BY Name ASC");
        $rows = $statement->fetchAll();
        $result = Array();
        foreach($rows as $group){

            $instanceGroup = new self();
            $instanceGroup->set($group);
            $result[] = $instanceGroup;
        }
        return $result; 
    }
    
    
    /**
    * Wrapper for creating a support group.
    * It will check if the support group doesn't exist yet, if the tag or name already exists then NAME_TAKEN  or TAG_TAKEN will be returned.
    * If the name is bigger than 20 characters or smaller than 4 and the tag greater than 7 or smaller than 2 a SIZE_ERROR will be returned.
    * Else it will return SUCCESS
    * @return a string that specifies if it was a success or not (SUCCESS, SIZE_ERROR, NAME_TAKEN or TAG_TAKEN )
    */
    public static function createSupportGroup( $name, $tag, $groupemail, $imap_mailserver, $imap_username, $imap_password) {
        
        if(strlen($name) < 21 && strlen($name) > 4 &&strlen($tag) < 8  && strlen($tag) > 1 ){
            $notExists = self::supportGroup_EntryNotExists($name, $tag);
            if ( $notExists == "SUCCESS" ){
                $sGroup = new self();
                $values = array('Name' => $name, 'Tag' => $tag, 'GroupEmail' => $groupemail, 'IMAP_MailServer' => $imap_mailserver, 'IMAP_Username' => $imap_username, 'IMAP_Password' => $imap_password);
                $sGroup->setName($values['Name']);
                $sGroup->setTag($values['Tag']);
                $sGroup->setGroupEmail($values['GroupEmail']);
                $sGroup->setIMAP_MailServer($values['IMAP_MailServer']);
                $sGroup->setIMAP_Username($values['IMAP_Username']);
                
                //encrypt password!
                global $cfg;
                $crypter = new MyCrypt($cfg['crypt']);
                $enc_password = $crypter->encrypt($values['IMAP_Password']);
                $sGroup->setIMAP_Password($enc_password);
                $sGroup->create();
                
                return "SUCCESS";
            }else{
                //return NAME_TAKEN  or TAG_TAKEN
                return $notExists;
            }
        }else{
            //RETURN ERROR that indicates SIZE
            return "SIZE_ERROR";
        } 
    }

    /**
    * check if support group name/tag doesn't exist yet.
    * @param $name the name of the group we want to check
    * @param $tag the tag of the group we want to check
    * @return if name is already taken return NAME_TAKEN, else if tag is already taken return TAG_TAKEN, else return success.
    */
    public static function supportGroup_EntryNotExists( $name, $tag) {
        $dbl = new DBLayer("lib");
        //check if name is already used
        if(  $dbl->execute("SELECT * FROM support_group WHERE Name = :name",array('name' => $name))->rowCount() ){
            return "NAME_TAKEN";
        }
        else if(  $dbl->execute("SELECT * FROM support_group WHERE Tag = :tag",array('tag' => $tag))->rowCount() ){
            return "TAG_TAKEN";
        }else{
            return "SUCCESS";
        } 
    }


    /**
    * check if support group entry coupled to a given id exist or not.
    * @param $id the id of the group we want to check
    * @return true or false.
    */
    public static function supportGroup_Exists( $id) {
        $dbl = new DBLayer("lib");
        //check if supportgroup id exist
        if(  $dbl->execute("SELECT * FROM support_group WHERE SGroupId = :id",array('id' => $id ))->rowCount() ){
            return true;
        }else{
            return false;
        }
    }
    
    
    /**
    * construct an object based on the SGroupId.
    * @param $id the id of the group we want to construct
    * @return the constructed support group object
    */
    public static function constr_SGroupId( $id) {
        $instance = new self();
        $instance->setSGroup($id);
        return $instance;
    }
    

    /**
    * get list of all users that are enlisted to a support group.
    * @param $group_id the id of the group we want to query
    * @return an array of ticket_user objects that are in the support group.
    */
    public static function getAllUsersOfSupportGroup($group_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `in_support_group` INNER JOIN `ticket_user` ON ticket_user.TUserId = in_support_group.User WHERE in_support_group.Group=:id", array('id' => $group_id));
        $rows = $statement->fetchAll();
        $result = Array();
        foreach($rows as $row){
            $userInstance = new Ticket_User();
            $userInstance->setTUserId($row['TUserId']);
            $userInstance->setPermission($row['Permission']);
            $userInstance->setExternId($row['ExternId']);
            $result[] = $userInstance;
        }
        return $result; 
    }
     
    
    /**
    * wrapper for deleting a support group.
    * We will first check if the group really exists, if not than "GROUP_NOT_EXISING" will be returned.
    * @param $group_id the id of the group we want to delete
    * @return an array of ticket_user objects that are in the support group.
    */
    public static function deleteSupportGroup($group_id) {
        
        //check if group id exists
        if (self::supportGroup_Exists($group_id)){
            $sGroup = new self();
            $sGroup->setSGroupId($group_id);
            $sGroup->delete();  
        }else{
            //return that group doesn't exist
            return "GROUP_NOT_EXISTING";
        }
            
    }
    
    /**
    * wrapper for deleting a user that's in a specified support group.
    * We will first check if the group really exists, if not than "GROUP_NOT_EXISING" will be returned.
    * Afterwards we will check if the user exists in the support group, if not "USER_NOT_IN_GROUP" will be returned.
    * Else the users entry in the in_support_group table will be deleted and "SUCCESS" will be returned.
    * @param $user_id the id of the user we want to remove out of the group.
    * @param $group_id the id of the group the user should be in
    * @return a string (SUCCESS, USER_NOT_IN_GROUP or GROUP_NOT_EXISTING)
    */
    public static function deleteUserOfSupportGroup( $user_id, $group_id) {
        
        //check if group id exists
        if (self::supportGroup_Exists($group_id)){
            
            //check if user is in supportgroup
            //if so, delete entry and return SUCCESS
            if(In_Support_Group::userExistsInSGroup($user_id, $group_id) ){
                //delete entry
                $inSGroup = new In_Support_Group();
                $inSGroup->setUser($user_id);
                $inSGroup->setGroup($group_id);
                $inSGroup->delete();
                return "SUCCESS";
            }
            else{
                //else return USER_NOT_IN_GROUP
                return "USER_NOT_IN_GROUP";
            }
            
            
        }else{
            //return that group doesn't exist
            return "GROUP_NOT_EXISTING";
        }
            
    }
    
    
    /**
    * wrapper for adding a user to a specified support group.
    * We will first check if the group really exists, if not than "GROUP_NOT_EXISING" will be returned.
    * Afterwards we will check if the user exists in the support group, if so "ALREADY_ADDED" will be returned.
    * Else the user will be added to the in_support_group table and "SUCCESS" will be returned.
    * @param $user_id the id of the user we want to add to the group.
    * @param $group_id the id of the group the user wants to be in
    * @return a string (SUCCESS, ALREADY_ADDED or GROUP_NOT_EXISTING)
    */
    public static function addUserToSupportGroup( $user_id, $group_id) {
        //check if group id exists
        if (self::supportGroup_Exists($group_id)){
            //check if user isn't in supportgroup yet
            //if not, create entry and return SUCCESS
            if(! In_Support_Group::userExistsInSGroup($user_id, $group_id) ){
                //create entry
                $inSGroup = new In_Support_Group();
                $inSGroup->setUser($user_id);
                $inSGroup->setGroup($group_id);
                $inSGroup->create();
                return "SUCCESS";
            }
            else{
                //else return ALREADY_ADDED
                return "ALREADY_ADDED";
            }
            
            
        }else{
            //return that group doesn't exist
            return "GROUP_NOT_EXISTING";
        }
            
    }
    
    
    /**
    * return all support_group objects.
    * @return an array containing all support_group objects.
    * @deprecated should be removed in the future, because getGroups does the same.
    */
    public static function getAllSupportGroups() {
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM `support_group`");
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $group){
            $instance = new self();
            $instance->set($group);
            $result[] = $instance;
        }
        return $result; 
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
    * @param $values should be an array of the form array('SGroupId' => groupid, 'Name' => name, 'Tag' => tag, 'GroupEmail' => mail, 'IMAP_MailServer' => server, 'IMAP_Username' => username,'IMAP_Password' => pass).
    */
    public function set($values) {
        $this->setSGroupId($values['SGroupId']);
        $this->setName($values['Name']);
        $this->setTag($values['Tag']);
        $this->setGroupEmail($values['GroupEmail']);
        $this->setIMAP_MailServer($values['IMAP_MailServer']);
        $this->setIMAP_Username($values['IMAP_Username']);
        $this->setIMAP_Password($values['IMAP_Password']);
    }
    
    
    /**
    * creates a new 'support_group' entry.
    * this method will use the object's attributes for creating a new 'support_group' entry in the database.
    */
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO support_group (Name, Tag, GroupEmail, IMAP_MailServer, IMAP_Username, IMAP_Password) VALUES (:name, :tag, :groupemail, :imap_mailserver, :imap_username, :imap_password)";
        $values = Array('name' => $this->getName(), 'tag' => $this->getTag(), 'groupemail' => $this->getGroupEmail(), 'imap_mailserver' => $this->getIMAP_MailServer(), 'imap_username' => $this->getIMAP_Username(), 'imap_password' => $this->getIMAP_Password());
        $dbl->execute($query, $values);
    } 

    
    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a group id, it will put the matching groups attributes in the object.
    * @param $id the id of the support group that should be loaded
    */
    public function load_With_SGroupId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `support_group` WHERE `SGroupId` = :id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    /**
    * update the objects attributes to the db.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE `support_group` SET `Name` = :name, `Tag` = :tag, `GroupEmail` = :groupemail, `IMAP_MailServer` = :mailserver, `IMAP_Username` = :username, `IMAP_Password` = :password WHERE `SGroupId` = :id";
        $values = Array('id' => $this->getSGroupId(), 'name' => $this->getName(), 'tag' => $this->getTag(), 'groupemail' => $this->getGroupEmail(), 'mailserver' => $this->getIMAP_MailServer(), 'username' => $this->getIMAP_Username(), 'password' => $this->getIMAP_Password() );
        $statement = $dbl->execute($query, $values);
    }
    
    
    /**
    * deletes an existing 'support_group' entry.
    * this method will use the object's attributes for deleting an existing 'support_group' entry in the database.
    */
    public function delete(){
        $dbl = new DBLayer("lib");
        $query = "DELETE FROM `support_group` WHERE `SGroupId` = :id";
        $values = Array('id' => $this->getSGroupId());
        $statement = $dbl->execute($query, $values);     
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get sGroupId attribute of the object.
    */
    public function getSGroupId(){
        return $this->sGroupId;
    }
    
    /**
    * get name attribute of the object.
    */
    public function getName(){
        return $this->name;
    }
    
    /**
    * get tag attribute of the object.
    */
    public function getTag(){
        return $this->tag;
    }
    
    /**
    * get groupEmail attribute of the object.
    */
    public function getGroupEmail(){
        return $this->groupEmail;
    }
    
    /**
    * get iMAP_MailServer attribute of the object.
    */
    public function getIMAP_MailServer(){
        return $this->iMAP_MailServer;
    }
    
    /**
    * get iMAP_Username attribute of the object.
    */
    public function getIMAP_Username(){
        return $this->iMAP_Username;
    }
    
    /**
    * get iMAP_Password attribute of the object.
    */
    public function getIMAP_Password(){
        return $this->iMap_Password;
    }
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    /**
    * set sGroupId attribute of the object.
    * @param $id integer id of the group
    */
    public function setSGroupId($id){
        $this->sGroupId = $id;
    }
    
    /**
    * set name attribute of the object.
    * @param $n name of the group
    */
    public function setName($n){
        $this->name = $n;
    }
    
    /**
    * set tag attribute of the object.
    * @param $t tag of the group
    */
    public function setTag($t){
        $this->tag = $t;
    }
   
    /**
    * set groupEmail attribute of the object.
    * @param $ge email of the group
    */
    public function setGroupEmail($ge){
        $this->groupEmail = $ge;
    }
    
    /**
    * set iMAP_MailServer attribute of the object.
    * @param $ms mailserver of the group
    */
    public function setIMAP_MailServer($ms){
        $this->iMAP_MailServer = $ms;
    }
    
    /**
    * set iMAP_Username attribute of the object.
    * @param $u imap username of the group
    */
    public function setIMAP_Username($u){
        $this->iMAP_Username = $u;
    }
    
    /**
    * set iMAP_Password attribute of the object.
    * @param $p imap password of the group
    */
    public function setIMAP_Password($p){
        $this->iMap_Password = $p;
    }
}