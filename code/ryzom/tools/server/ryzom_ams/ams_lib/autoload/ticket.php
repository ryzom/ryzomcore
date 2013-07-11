<?php

class Ticket{
    private $tId;
    private $timestamp;
    private $title;
    private $status;
    private $queue;
    private $ticket_category;
    private $author;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
     /*FUNCTION: getStatusArray
     * returns all possible statusses
     *
     */
    public static function getStatusArray() {
        return Array("Waiting on user reply","Waiting on support","Waiting on Dev reply","Closed");
    }
    
     /*FUNCTION: getEntireTicket
     * return all ticket of the given author's id.
     *
     */
    public static function getEntireTicket($id) {
        $ticket = new Ticket();
        $ticket->load_With_TId($id);
        $reply_array = Ticket_Reply::getRepliesOfTicket($id);
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
    public static function create_Ticket( $title, $content, $category, $author) {
        
        $ticket = new Ticket();
        $ticket->set($title,1,0,$category,$author);
        $ticket->create();
        $ticket_id = $ticket->getTId();
        Ticket_Reply::createReply($content, $author, $ticket_id); 
        return $ticket_id;
        
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    public function __construct() {

    }


    //Set ticket object
    public function set($t,$s,$q,$t_c,$a){
        $this->title = $t;
        $this->status = $s;
        $this->queue = $q;
        $this->ticket_category = $t_c;
        $this->author = $a;
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket (Timestamp, Title, Status, Queue, Ticket_Category, Author) VALUES (now(), :title, :status, :queue, :tcat, :author)";
        $values = Array('title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author);
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
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket SET Timestamp = :timestamp, Title = :title, Status = :status, Queue = :queue, Ticket_Category = :tcat, Author = :author WHERE TId=:id";
        $values = Array('id' => $this->tId, 'timestamp' => $this->timestamp, 'title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author);
        $statement = $dbl->execute($query, $values);
    }
    
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getTId(){
        return $this->tId;
    }
    
    public function getTimestamp(){
        return $this->timestamp;
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
    
}