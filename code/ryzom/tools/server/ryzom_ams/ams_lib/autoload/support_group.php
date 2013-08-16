<?php

class Support_Group{
    
    private $sGroupId;
    private $name;
    private $tag;
    private $groupEmail;
    private $iMAP_MailServer;
    private $iMAP_Username;
    private $iMAP_Password;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //return all groups
    public static function getGroup($id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM support_group WHERE SGroupId = :id", array('id' => $id));
        $row = $statement->fetch();
        $instanceGroup = new self();
        $instanceGroup->set($row);
        return $instanceGroup;
    
    }
    
    //return all groups
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
    
    //wrapper for creating a support group
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
                $sGroup->setIMAP_Password($values['IMAP_Password']);
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

    //check if group exists
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


    //check if group exists
    public static function supportGroup_Exists( $id) {
        $dbl = new DBLayer("lib");
        //check if supportgroup id exist
        if(  $dbl->execute("SELECT * FROM support_group WHERE SGroupId = :id",array('id' => $id ))->rowCount() ){
            return true;
        }else{
            return false;
        }
    }
    
    
    //return constructed element based on SGroupId
    public static function constr_SGroupId( $id) {
        $instance = new self();
        $instance->setSGroup($id);
        return $instance;
    }
    
    //returns list of all users that are enlisted to a support group
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
     
    //wrapper for adding user to a support group
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
    
    //wrapper for adding user to a support group
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
    
    //wrapper for adding user to a support group
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
    
    //returns list of all category objects
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
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setSGroupId($values['SGroupId']);
        $this->setName($values['Name']);
        $this->setTag($values['Tag']);
        $this->setGroupEmail($values['GroupEmail']);
        $this->setIMAP_MailServer($values['IMAP_MailServer']);
        $this->setIMAP_Username($values['IMAP_Username']);
        $this->setIMAP_Password($values['IMAP_Password']);
    }
    
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO support_group (Name, Tag, GroupEmail, IMAP_MailServer, IMAP_Username, IMAP_Password) VALUES (:name, :tag, :groupemail, :imap_mailserver, :imap_username, :imap_password)";
        $values = Array('name' => $this->getName(), 'tag' => $this->getTag(), 'groupemail' => $this->getGroupEmail(), 'imap_mailserver' => $this->getIMAP_MailServer(), 'imap_username' => $this->getIMAP_Username(), 'imap_password' => $this->getIMAP_Password());
        $dbl->execute($query, $values);
    } 

    //Load with sGroupId
    public function load_With_SGroupId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `support_group` WHERE `SGroupId` = :id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE `support_group` SET `Name` = :name, `Tag` = :tag WHERE `SGroupId` = :id";
        $values = Array('id' => $this->getSGroupId(), 'name' => $this->getName(), 'tag' => $this->getTag() );
        $statement = $dbl->execute($query, $values);
    }
    
    //delete entry
    public function delete(){
        $dbl = new DBLayer("lib");
        $query = "DELETE FROM `support_group` WHERE `SGroupId` = :id";
        $values = Array('id' => $this->getSGroupId());
        $statement = $dbl->execute($query, $values);     
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getSGroupId(){
        return $this->sGroupId;
    }
    
    public function getName(){
        return $this->name;
    }
    
    public function getTag(){
        return $this->tag;
    }
    
    public function getGroupEmail(){
        return $this->groupEmail;
    }
    
    public function getIMAP_MailServer(){
        return $this->iMAP_MailServer;
    }
    
    public function getIMAP_Username(){
        return $this->iMAP_Username;
    }
    
    public function getIMAP_Password(){
        return $this->iMap_Password;
    }
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    public function setSGroupId($id){
        $this->sGroupId = $id;
    }
    
    public function setName($n){
        $this->name = $n;
    }
    
    public function setTag($t){
        $this->tag = $t;
    }
   
    public function setGroupEmail($ge){
        $this->groupEmail = $ge;
    }
    
    public function setIMAP_MailServer($ms){
        $this->iMAP_MailServer = $ms;
    }
    
    public function setIMAP_Username($u){
        $this->iMAP_Username = $u;
    }
    
    public function setIMAP_Password($p){
        $this->iMap_Password = $p;
    }
}