<?php
/**
* Handles the assigning of a ticket to a user. This is being used to make someone responsible for the handling and solving of a ticket.
* The idea is that someone can easily assign a ticket to himself and by doing that, he makes aware to the other moderators that he will deal with the ticket in the future.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Assigned{

    private $user; /**< The id of the user being assigned */
    private $ticket; /**< The id of the ticket being assigned */


    ////////////////////////////////////////////Functions////////////////////////////////////////////////////

    /**
    * Assigns a ticket to a user or returns an error message.
    * It will first check if the ticket isn't already assigned, if not, it will create a new 'assigned' entry.
    * @param $user_id the id of the user we want to assign to the ticket
    * @param $ticket_id the id of the ticket.
    * @return A string, if assigning succeedded "SUCCESS_ASSIGNED" will be returned, else "ALREADY_ASSIGNED" will be returned.
    */
    public static function assignTicket( $user_id, $ticket_id) {
        $dbl = new DBLayer("lib");
        //check if ticket is already assigned, if so return "ALREADY ASSIGNED"
        if(! Assigned::isAssigned($ticket_id)){
            $assignation = new Assigned();
            $assignation->set(array('User' => $user_id, 'Ticket' => $ticket_id));
            $assignation->create();
            return "SUCCESS_ASSIGNED";
        }else{
            return "ALREADY_ASSIGNED";
        }

    }


    /**
    * Unassign a ticket being coupled to a user or return an error message.
    * It will first check if the ticket is assigned, if this is indeed the case it will delete the 'assigned' entry.
    * @param $user_id the id of the user we want to unassign from the ticket
    * @param $ticket_id the id of the ticket.
    * @return A string, if unassigning succeedded "SUCCESS_UNASSIGNED" will be returned, else "NOT_ASSIGNED" will be returned.
    */
    public static function unAssignTicket( $user_id, $ticket_id) {
        $dbl = new DBLayer("lib");
        //check if ticket is really assigned to that user
        if( Assigned::isAssigned($ticket_id, $user_id)){
            $assignation = new Assigned();
            $assignation->set(array('User' => $user_id, 'Ticket' => $ticket_id));
            $assignation->delete();
            return "SUCCESS_UNASSIGNED";
        }else{
            return "NOT_ASSIGNED";
        }

    }

    /**
    * Get the (external) id of the user assigned to a ticket
    * @param $ticket_id the Id of the ticket that's being queried
    * @return The (external)id of the user being assigned to the ticket
    */
    public static function getUserAssignedToTicket($ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT ticket_user.ExternId FROM `assigned` JOIN `ticket_user` ON assigned.User = ticket_user.TUserId WHERE `Ticket` = :ticket_id", Array('ticket_id' => $ticket_id));
        $user_id = $statement->fetch();
        return $user_id['ExternId'];



    }

    /**
    * Check if a ticket is already assigned (in case the user_id param is used, it will check if it's assigned to that user)
    * @param $ticket_id the Id of the ticket that's being queried
    * @param $user_id the id of the user, default parameter = 0, by using a user_id, it will check if that user is assigned to the ticket.
    * @return true in case it's assigned, false in case it isn't.
    */
    public static function isAssigned( $ticket_id, $user_id = 0) {
        $dbl = new DBLayer("lib");
        //check if ticket is already assigned

        if($user_id == 0 &&  $dbl->select("`assigned`", array('ticket_id' => $ticket_id), "`Ticket` = :ticket_id")->rowCount() ){
            return true;
        }else if( $dbl->select("`assigned`", array('ticket_id' => $ticket_id, 'user_id' => $user_id), "`Ticket` = :ticket_id and `User` = :user_id")->rowCount() ){
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
    * @param $values should be an array of the form array('User' => user_id, 'Ticket' => ticket_id).
    */
    public function set($values) {
        $this->setUser($values['User']);
        $this->setTicket($values['Ticket']);
    }


    /**
    * creates a new 'assigned' entry.
    * this method will use the object's attributes for creating a new 'assigned' entry in the database.
    */
    public function create() {
        $dbl = new DBLayer("lib");
	$dbl->insert("`assigned`", Array('User' => $this->getUser(), 'Ticket' => $this->getTicket()));
    }


    /**
    * deletes an existing 'assigned' entry.
    * this method will use the object's attributes for deleting an existing 'assigned' entry in the database.
    */
    public function delete() {
        $dbl = new DBLayer("lib");
	$dbl->delete("`assigned`", array('user_id' => $this->getUser() ,'ticket_id' => $this->getTicket()), "`User` = :user_id and `Ticket` = :ticket_id");
    }

    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a ticket_id, it will put the matching user_id and the ticket_id into the attributes.
    * @param $ticket_id the id of the ticket that should be loaded
    */
    public function load($ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("`assigned`", Array('ticket_id' => $ticket_id), "`Ticket` = :ticket_id");
        $row = $statement->fetch();
        $this->set($row);
    }


    ////////////////////////////////////////////Getters////////////////////////////////////////////////////

    /**
    * get user attribute of the object.
    */
    public function getUser(){
        return $this->user;
    }


    /**
    * get ticket attribute of the object.
    */
    public function getTicket(){
        return $this->ticket;
    }

    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    /**
    * set user attribute of the object.
    * @param $u integer id of the user
    */
    public function setUser($u){
        $this->user = $u;
    }

    /**
    * set ticket attribute of the object.
    * @param $t integer id of the ticket
    */
    public function setTicket($t){
        $this->ticket = $t;
    }


}
