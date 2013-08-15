<?php

class Forwarded{
    
    private $group;
    private $ticket;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
  
    //AForward a ticket to a group, also removes the previous group where it was assigned to.
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
    
    public static function getSGroupOfTicket($ticket_id) {
        $forw = new self();
        $forw->load($ticket_id);
        return $forw->getGroup();
    }
    
    
    public static function isForwarded( $ticket_id) {
        $dbl = new DBLayer("lib");
        if( $dbl->execute(" SELECT * FROM `forwarded` WHERE `Ticket` = :ticket_id", array('ticket_id' => $ticket_id))->rowCount()){
            return true;
        }else{
            return false;
        } 
        
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setGroup($values['Group']);
        $this->setTicket($values['Ticket']);
    }
    
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO `forwarded` (`Group`,`Ticket`) VALUES (:group, :ticket)";
        $values = Array('group' => $this->getGroup(), 'ticket' => $this->getTicket());
        $dbl->execute($query, $values);
    }
    
    //delete entry
    public function delete() {
        $dbl = new DBLayer("lib");
        $query = "DELETE FROM `forwarded` WHERE `Group` = :group_id and `Ticket` = :ticket_id";
        $values = array('group_id' => $this->getGroup() ,'ticket_id' => $this->getTicket());
        $dbl->execute($query, $values);
    }

    //Load with sGroupId
    public function load( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `forwarded` WHERE `Ticket` = :ticket_id", Array('ticket_id' => $ticket_id));
        $row = $statement->fetch();
        $this->set($row);
    }
    

    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getGroup(){
        return $this->group;
    }
    
    public function getTicket(){
        return $this->ticket;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    public function setGroup($g){
        $this->group = $g;
    }
    
    public function setTicket($t){
        $this->ticket = $t;
    }
   
    
}