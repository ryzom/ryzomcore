<?php

class Ticket{
    private $tId;
    private $timestamp;
    private $title;
    private $status;
    private $queue;
    private $ticket_category;
    private $author;
    private $priority;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /*FUNCTION: ticketExists
    * returns true if ticket exists
    *
    */
    public static function ticketExists($id) {
        $dbl = new DBLayer("lib");
        //check if ticket is already assigned
        if(  $dbl->execute(" SELECT * FROM `ticket` WHERE `TId` = :ticket_id", array('ticket_id' => $id) )->rowCount() ){
            return true;
        }else{
            return false;
        } 
    }
    
    /*FUNCTION: getStatusArray
    * returns all possible statusses
    *
    */
    public static function getStatusArray() {
        return Array("Waiting on user reply","Waiting on support","Waiting on Dev reply","Closed");
    }
    
    /*FUNCTION: getPriorityArray
    * returns all possible statusses
    *
    */
    public static function getPriorityArray() {
        return Array("Low","Normal","High","Super Dupa High");
    }
    
    
    /*FUNCTION: getEntireTicket
    * return all ticket of the given author's id.
    *
    */
    public static function getEntireTicket($id,$view_as_admin) {
        $ticket = new Ticket();
        $ticket->load_With_TId($id);
        $reply_array = Ticket_Reply::getRepliesOfTicket($id, $view_as_admin);
        return Array('ticket_obj' => $ticket,'reply_array' => $reply_array); 
    }
    
    
    /*FUNCTION: getTicketTitlesOf
     * return all ticket of the given author's id.
     *
     */
    public static function getTicketsOf($author) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket_user.ExternId=:id", array('id' => $author));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $ticket){
            $instance = new self();
            $instance->setTId($ticket['TId']);
            $instance->setTimestamp($ticket['Timestamp']);
            $instance->setTitle($ticket['Title']);
            $instance->setStatus($ticket['Status']);
            $instance->setQueue($ticket['Queue']);
            $instance->setTicket_Category($ticket['Ticket_Category']);
            $instance->setAuthor($ticket['Author']);
            $result[] = $instance;
        }
        return $result; 
    }
    
    
    /*FUNCTION: create_Ticket()
     * creates a ticket + first initial reply and fills in the content of it!
     *
     */
    public static function create_Ticket( $title, $content, $category, $author, $real_author) {
        
        $ticket = new Ticket();
        $values = array("Title" => $title, "Status"=> 1, "Queue"=> 0, "Ticket_Category" => $category, "Author" => $author, "Priority" => 0);
        $ticket->set($values);
        $ticket->create();
        $ticket_id = $ticket->getTId();
        
        if ( $author == $real_author){
            Ticket_Log::createLogEntry( $ticket_id, $author, 1);
        }else{
            Ticket_Log::createLogEntry( $ticket_id, $real_author, 2, $author);
        }
        Ticket_Reply::createReply($content, $author, $ticket_id, 0);
        return $ticket_id;
        
    }
        
    /*FUNCTION: updateTicketStatus()
     *
     *
     */
    public static function updateTicketStatus( $ticket_id, $newStatus, $author) {
        
        $ticket = new Ticket();
        $ticket->load_With_TId($ticket_id);
        if ($ticket->getStatus() != $newStatus){
            $ticket->setStatus($newStatus);
            Ticket_Log::createLogEntry( $ticket_id, $author, 5, $newStatus);
        }
        $ticket->update();
        
    }
    
    
    /*FUNCTION: updateTicketStatusAndPriority()
     * creates a ticket + first initial reply and fills in the content of it!
     *
     */
    public static function updateTicketStatusAndPriority( $ticket_id, $newStatus, $newPriority, $author) {
        
        $ticket = new Ticket();
        $ticket->load_With_TId($ticket_id);
        if ($ticket->getStatus() != $newStatus){
            $ticket->setStatus($newStatus);
            Ticket_Log::createLogEntry( $ticket_id, $author, 5, $newStatus);
        }
        if ($ticket->getPriority() != $newPriority){
            $ticket->setPriority($newPriority);
            Ticket_Log::createLogEntry( $ticket_id, $author, 6, $newPriority);
        }
        $ticket->update();
        
    }
    
    //return the latest reply.
    public static function getLatestReply( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_reply WHERE Ticket =:id ORDER BY TReplyId DESC LIMIT 1 ", array('id' => $ticket_id));
        $reply = new Ticket_Reply();
        $reply->set($statement->fetch());
        return $reply;
    }
    
    public static function createReply($content, $author, $ticket_id, $hidden){
        //if not empty
        if(! ( Trim ( $content ) === '' )){
            $content = filter_var($content, FILTER_SANITIZE_STRING);
            $ticket = new Ticket();
            $ticket->load_With_TId($ticket_id);
            //if status is not closed
            if($ticket->getStatus() != 3){
                Ticket_Reply::createReply($content, $author, $ticket_id, $hidden, $ticket->getAuthor());
            }else{
                //TODO: Show error message that ticket is closed
            }
        }else{
            //TODO: Show error content is empty
        }
    }
    
    //returns SUCCESS_ASSIGNED, TICKET_NOT_EXISTING or ALREADY_ASSIGNED
    public static function assignTicket($user_id, $ticket_id){
        if(self::ticketExists($ticket_id)){
            $returnvalue = Assigned::assignTicket($user_id, $ticket_id);
            Ticket_Log::createLogEntry( $ticket_id, $user_id, 7);
            return $returnvalue;
        }else{
            return "TICKET_NOT_EXISTING";
        }
    }
    
    //returns SUCCESS_UNASSIGNED, TICKET_NOT_EXISTING or NOT_ASSIGNED
    public static function unAssignTicket($user_id, $ticket_id){
        if(self::ticketExists($ticket_id)){
            $returnvalue = Assigned::unAssignTicket($user_id, $ticket_id);
            Ticket_Log::createLogEntry( $ticket_id, $user_id, 9);
            return $returnvalue;
        }else{
            return "TICKET_NOT_EXISTING";
        }
    }
    
    public static function forwardTicket($user_id, $ticket_id, $group_id){
        if(self::ticketExists($ticket_id)){
            if(isset($group_id) && $group_id != ""){
                //unassign the ticket incase the ticket is assined to yourself
                self::unAssignTicket($user_id, $ticket_id);
                //forward the ticket
                $returnvalue = Forwarded::forwardTicket($group_id, $ticket_id);
                //make a log entry of this action
                Ticket_Log::createLogEntry( $ticket_id, $user_id, 8, $group_id);
                return $returnvalue;
            }else{
                return "INVALID_SGROUP";
            }
        }else{
            return "TICKET_NOT_EXISTING";
        }
    }
    
    
    
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    public function __construct() {

    }


    //Set ticket object
    public function set($values){
        $this->tId = $values['TId'];
        $this->title = $values['Title'];
        $this->status = $values['Status'];
        $this->queue = $values['Queue'];
        $this->ticket_category = $values['Ticket_Category'];
        $this->author = $values['Author'];
        $this->priority = $values['Priority'];
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket (Timestamp, Title, Status, Queue, Ticket_Category, Author, Priority) VALUES (now(), :title, :status, :queue, :tcat, :author, :priority)";
        $values = Array('title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author, 'priority' => $this->priority);
        $this->tId = $dbl->executeReturnId($query, $values); ;
    }

    //return constructed element based on TId
    public function load_With_TId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket WHERE TId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tId = $row['TId'];
        $this->timestamp = $row['Timestamp'];
        $this->title = $row['Title'];
        $this->status = $row['Status'];
        $this->queue = $row['Queue'];
        $this->ticket_category = $row['Ticket_Category'];
        $this->author = $row['Author'];
        $this->priority = $row['Priority'];
    }
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket SET Timestamp = :timestamp, Title = :title, Status = :status, Queue = :queue, Ticket_Category = :tcat, Author = :author, Priority = :priority WHERE TId=:id";
        $values = Array('id' => $this->tId, 'timestamp' => $this->timestamp, 'title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author, 'priority' => $this->priority);
        $statement = $dbl->execute($query, $values);
    }
    
    /*FUNCTION: postreply
    * returns all possible statusses
    *
    *
    public function postReply() {
        return Array("Waiting on user reply","Waiting on support","Waiting on Dev reply","Closed");
    }*/
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getTId(){
        return $this->tId;
    }
    
    public function getTimestamp(){
        return Helpers::outputTime($this->timestamp);
    }
    
    public function getTitle(){
        return $this->title;
    }
    
    public function getStatus(){
        return $this->status;
    }
    
    public function getStatusText(){
        $statusArray = Ticket::getStatusArray();
        return $statusArray[$this->getStatus()];
    }
    
    public function getCategoryName(){
        $category = Ticket_Category::constr_TCategoryId($this->getTicket_Category());
        return $category->getName();  
    }
    
    public function getQueue(){
        return $this->queue;
    }
    
    public function getTicket_Category(){
        return $this->ticket_category;
    }
    
    public function getAuthor(){
        return $this->author;
    }
    
    public function getPriority(){
        return $this->priority;
    }
    
    public function getPriorityText(){
        $priorityArray = Ticket::getPriorityArray();
        return $priorityArray[$this->getPriority()];
    }
    
    public function getAssigned(){
        $user_id = Assigned::getUserAssignedToTicket($this->getTId());
        if ($user_id == ""){
            return 0;
        }else{
            return $user_id;
        }
    }
    
    public function getForwardedGroupName(){
        $group_id = Forwarded::getSGroupOfTicket($this->getTId());
        if ($group_id == ""){
            return 0;
        }else{
            return Support_Group::getGroup($group_id)->getName();
        }
    }
    
    public function getForwardedGroupId(){
        $group_id = Forwarded::getSGroupOfTicket($this->getTId());
        if ($group_id == ""){
            return 0;
        }else{
            return $group_id;
        }
    }
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    public function setTId($id){
        $this->tId = $id;
    }
    
    public function setTimestamp($ts){
        $this->timestamp = $ts;
    }
    
    public function setTitle($t){
        $this->title = $t;
    }
    
    public function setStatus($s){
        $this->status = $s;
    }
    
    public function setQueue($q){
        $this->queue = $q;
    }
    
    public function setTicket_Category($tc){
        $this->ticket_category = $tc;
    }
    
    public function setAuthor($a){
        $this->author = $a;
    }
    
    public function setPriority($p){
        $this->priority = $p;
    }
    
}