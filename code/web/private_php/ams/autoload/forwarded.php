<?php
/**
* Handles the forwarding of a ticket to a support_group. This is being used to transfer tickets to different groups (eg Developers, Website-Team, SupportGroup etc..)
* The idea is that someone can easily forward a ticket to a group and by doing that, the moderators that are in that group will receive the ticket in their todo queue.
* @author Daan Janssens, mentored by Matthew Lagoe
* 
*/
class Forwarded{
    
    private $group; /**< The id of the group to which the ticket is being forwarded */ 
    private $ticket; /**< The id of the ticket being forwarded */ 
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    
    /**
    * Forward a ticket to a group, also removes the previous group where it was forwarded to.
    * It will first check if the ticket is already forwarded, if that's the case, it will delete that entry.
    * Afterwards it creates the new forward entry
    * @param $group_id the id of the support group we want to forward the ticket to.
    * @param $ticket_id the id of the ticket.
    * @return A string, if assigning succeedded "SUCCESS_FORWARDED" will be returned.
    */
    public static function forwardTicket( $group_id, $ticket_id) {
        $dbl = new DBLayer("lib");
        if (forwarded::isForwarded($ticket_id)){
            $forw = new Forwarded();
            $forw->load($ticket_id);
            $forw->delete();
        }
        $forward = new Forwarded();
        $forward->set(array('Group' => $group_id, 'Ticket' => $ticket_id));
        $forward->create();
        return "SUCCESS_FORWARDED";
      
    }
    
    
    /**
    * get the id of the group a ticket is forwarded to.
    * @param $ticket_id the id of the ticket.
    * @return the id of the group
    */
    public static function getSGroupOfTicket($ticket_id) {
        $forw = new self();
        $forw->load($ticket_id);
        return $forw->getGroup();
    }
    
    
    /**
    * check if the ticket is forwarded
    * @param $ticket_id the id of the ticket.
    * @return returns true if the ticket is forwarded, else return false;
    */
    public static function isForwarded( $ticket_id) {
        $dbl = new DBLayer("lib");
        if( $dbl->execute(" SELECT * FROM `forwarded` WHERE `Ticket` = :ticket_id", array('ticket_id' => $ticket_id))->rowCount()){
            return true;
        }else{
            return false;
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
    * @param $values should be an array of the form array('Group' => group_id, 'Ticket' => ticket_id).
    */
    public function set($values) {
        $this->setGroup($values['Group']);
        $this->setTicket($values['Ticket']);
    }
    
    
    /**
    * creates a new 'forwarded' entry.
    * this method will use the object's attributes for creating a new 'forwarded' entry in the database.
    */
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO `forwarded` (`Group`,`Ticket`) VALUES (:group, :ticket)";
        $values = Array('group' => $this->getGroup(), 'ticket' => $this->getTicket());
        $dbl->execute($query, $values);
    }
    
    
    /**
    * deletes an existing 'forwarded' entry.
    * this method will use the object's attributes for deleting an existing 'forwarded' entry in the database.
    */
    public function delete() {
        $dbl = new DBLayer("lib");
        $query = "DELETE FROM `forwarded` WHERE `Group` = :group_id and `Ticket` = :ticket_id";
        $values = array('group_id' => $this->getGroup() ,'ticket_id' => $this->getTicket());
        $dbl->execute($query, $values);
    }


    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a ticket_id, it will put the matching group_id and the ticket_id into the attributes.
    * @param $ticket_id the id of the ticket that should be loaded
    */
    public function load( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `forwarded` WHERE `Ticket` = :ticket_id", Array('ticket_id' => $ticket_id));
        $row = $statement->fetch();
        $this->set($row);
    }
    

    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
     
    /**
    * get group attribute of the object.
    */
    public function getGroup(){
        return $this->group;
    }
    
    /**
    * get ticket attribute of the object.
    */
    public function getTicket(){
        return $this->ticket;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    /**
    * set group attribute of the object.
    * @param $g integer id of the group
    */
    public function setGroup($g){
        $this->group = $g;
    }
    
    /**
    * set ticket attribute of the object.
    * @param $t integer id of the ticket
    */
    public function setTicket($t){
        $this->ticket = $t;
    }
   
    
}