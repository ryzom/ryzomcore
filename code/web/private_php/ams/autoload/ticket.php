<?php

/**
* class that handles most ticket related functions.
* the ticket class is used for most ticketing related functions, it also holds some wrapper functions.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket{

    private $tId; /**< The id of ticket */
    private $timestamp; /**< Timestamp of the ticket */
    private $title; /**< Title of the ticket */
    private $status; /**< Status of the ticket (0 = waiting on user reply, 1 = waiting on support, (2= not used atm), 3 = closed */
    private $queue; /**< (not in use atm) */
    private $ticket_category; /**< the id of the category belonging to the ticket */
    private $author; /**< The ticket_users id */
    private $priority; /**< The priority of the ticket where 0 = low, 3= supadupahigh */

    ////////////////////////////////////////////Functions////////////////////////////////////////////////////


    /**
    * check if a ticket exists.
    * @param $id the id of the ticket to be checked.
    * @return true if the ticket exists, else false.
    */
    public static function ticketExists($id) {
        $dbl = new DBLayer("lib");
        //check if ticket exists
        if(  $dbl->select("`ticket`", array('ticket_id' => $id), "`TId` = :ticket_id")->rowCount() ){
            return true;
        }else{
            return false;
        }
    }


    /**
    * return an array of the possible statuses
    * @return an array containing the string values that represent the different statuses.
    */
    public static function getStatusArray() {
        return Array("Waiting on user reply","Waiting on support","Waiting on Dev reply","Closed");
    }


    /**
    * return an array of the possible priorities
    * @return an array containing the string values that represent the different priorities.
    */
    public static function getPriorityArray() {
        return Array("Low","Normal","High","Super Dupa High");
    }


    /**
    * return an entire ticket.
    * returns the ticket object and an array of all replies to that ticket.
    * @param $id the id of the ticket.
    * @param $view_as_admin true if the viewer of the ticket is a mod, else false (depending on this it will also show the hidden comments)
    * @return an array containing the 'ticket_obj' and a 'reply_array', which is an array containing all replies to that ticket.
    */
    public static function getEntireTicket($id,$view_as_admin) {
        $ticket = new Ticket();
        $ticket->load_With_TId($id);
        $reply_array = Ticket_Reply::getRepliesOfTicket($id, $view_as_admin);
        return Array('ticket_obj' => $ticket,'reply_array' => $reply_array);
    }


    /**
    * return all tickets of a specific user.
    * an array of all tickets created by a specific user are returned by this function.
    * @param $author the id of the user of whom we want all tickets from.
    * @return an array containing all ticket objects related to a user.
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



    /**
    * function that creates a new ticket.
    * A new ticket will be created, in case the extra_info != 0 and the http request came from ingame, then a ticket_info page will be created.
    * A log entry will be written, depending on the $real_authors value. In case the for_support_group parameter is set, the ticket will be forwarded immediately.
    * Also the mail handler will create a new email that will be sent to the author to notify him that his ticket is freshly created.
    * @param $title the title we want to give to the ticket.
    * @param $content the content we want to give to the starting post of the ticket.
    * @param $category the id of the category that should be related to the ticket.
    * @param $author the person who's id will be stored in the database as creator of the ticket.
    * @param $real_author should be the same id, or a moderator/admin who creates a ticket for another user (this is used for logging purposes).
    * @param $for_support_group in case you directly want to forward the ticket after creating it. (default value = 0 =  don't forward)
    * @param $extra_info used for creating an ticket_info page related to the ticket, this only happens when the ticket is made ingame.
    * @return the created tickets id.
    */
    public static function create_Ticket( $title, $content, $category, $author, $real_author, $for_support_group = 0, $extra_info = 0) {

        //create the new ticket!
        $ticket = new Ticket();
        $values = array("Title" => $title, "Timestamp"=>0,  "Status"=> 1, "Queue"=> 0, "Ticket_Category" => $category, "Author" => $author, "Priority" => 0);
        $ticket->set($values);
        $ticket->create();
        $ticket_id = $ticket->getTId();

        //if ingame then add an extra info
        if(Helpers::check_if_game_client() && $extra_info != 0){
            $extra_info['Ticket'] = $ticket_id;
            Ticket_Info::create_Ticket_Info($extra_info);
        }

        //write a log entry
        if ( $author == $real_author){
            Ticket_Log::createLogEntry( $ticket_id, $author, 1);
        }else{
            Ticket_Log::createLogEntry( $ticket_id, $real_author, 2, $author);
        }
        Ticket_Reply::createReply($content, $author, $ticket_id, 0, $author);

        //forwards the ticket directly after creation to the supposed support group
        if($for_support_group){
            Ticket::forwardTicket(0, $ticket_id, $for_support_group);
        }

        //send email that new ticket has been created
        Mail_Handler::send_ticketing_mail($ticket->getAuthor(), $ticket, $content, "NEW", $ticket->getForwardedGroupId());
        return $ticket_id;

    }


    /**
    * updates the ticket's status.
    * A log entry about this will be created only if the newStatus is different from the current status.
    * @param $ticket_id the id of the ticket of which we want to change the status.
    * @param $newStatus the new status value (integer)
    * @param $author the user (id) that performed the update status action
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


    /**
    * updates the ticket's status & priority.
    * A log entry about this will be created only if the newStatus is different from the current status and also when the newPriority is different from the current priority.
    * @todo break this function up into a updateStatus (already exists) and updatePriority function and perhaps write a wrapper function for the combo.
    * @param $ticket_id the id of the ticket of which we want to change the status & priority
    * @param $newStatus the new status value (integer)
    * @param $newPriority the new priority value (integer)
    * @param $author the user (id) that performed the update
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


    /**
    * return the latest reply of a ticket
    * @param $ticket_id the id of the ticket.
    * @return a ticket_reply object.
    */
    public static function getLatestReply( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_reply WHERE Ticket =:id ORDER BY TReplyId DESC LIMIT 1 ", array('id' => $ticket_id));
        $reply = new Ticket_Reply();
        $reply->set($statement->fetch());
        return $reply;
    }

    /**
    * return the attachments list
    * @param $ticket_id the id of the ticket.
    * @return a ticket_reply object.
    */
    public static function getAttachments( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("`ticket_attachments`",array('ticket_TId' => $ticket_id), "`ticket_TId` =:ticket_TId ORDER BY Timestamp DESC");
        $fetchall = $statement->fetchall();
        $base = 0;
        foreach ($fetchall as &$value) {
            $webUser = new WebUsers($value['Uploader']);
            $fetchall[$base]['Username'] = $webUser->getUsername();
            
            $bytes = $fetchall[$base]['Filesize'];
            $precision = 2;
            $units = array('B', 'KB', 'MB', 'GB', 'TB');
         
            $bytes = max($bytes, 0);
            $pow = floor(($bytes ? log($bytes) : 0) / log(1024));
            $pow = min($pow, count($units) - 1);
         
            $bytes /= pow(1024, $pow);
            
            $fetchall[$base]['Filesize'] = round($bytes, $precision) . ' ' . $units[$pow];;
            $base++;
        }
        return $fetchall;
    }
    
    /**
    * create a new reply for a ticket.
    * A reply will only be added if the content isn't empty and if the ticket isn't closed.
    * The ticket creator will be notified by email that someone else replied on his ticket.
    * @param $content the content of the reply
    * @param $author the author of the reply
    * @param $ticket_id the id of the ticket to which we want to add the reply.
    * @param $hidden boolean that specifies if the reply should only be shown to mods/admins or all users.
    */
    public static function createReply($content, $author, $ticket_id, $hidden){
        //if not empty
        if(! ( Trim ( $content ) === '' )){
            $content = filter_var($content, FILTER_SANITIZE_STRING);
            $ticket = new Ticket();
            $ticket->load_With_TId($ticket_id);
            //if status is not closed
            if($ticket->getStatus() != 3){
                Ticket_Reply::createReply($content, $author, $ticket_id, $hidden, $ticket->getAuthor());

                //notify ticket author that a new reply is added!
                if($ticket->getAuthor() != $author){
                    Mail_Handler::send_ticketing_mail($ticket->getAuthor(), $ticket, $content, "REPLY", $ticket->getForwardedGroupId());
                }


            }else{
                //TODO: Show error message that ticket is closed
            }
        }else{
            //TODO: Show error content is empty
        }
    }


    /**
    * assign a ticket to a user.
    * Checks if the ticket exists, if so then it will try to assign the user to it, a log entry will be written about this.
    * @param $user_id the id of user trying to be assigned to the ticket.
    * @param $ticket_id the id of the ticket that we try to assign to the user.
    * @return SUCCESS_ASSIGNED, TICKET_NOT_EXISTING or ALREADY_ASSIGNED
    */
    public static function assignTicket($user_id, $ticket_id){
        if(self::ticketExists($ticket_id)){
            $returnvalue = Assigned::assignTicket($user_id, $ticket_id);
            Ticket_Log::createLogEntry( $ticket_id, $user_id, 7);
            return $returnvalue;
        }else{
            return "TICKET_NOT_EXISTING";
        }
    }


    /**
    * unassign a ticket of a user.
    * Checks if the ticket exists, if so then it will try to unassign the user of it, a log entry will be written about this.
    * @param $user_id the id of user trying to be assigned to the ticket.
    * @param $ticket_id the id of the ticket that we try to assign to the user.
    * @return SUCCESS_UNASSIGNED, TICKET_NOT_EXISTING or NOT_ASSIGNED
    */
    public static function unAssignTicket($user_id, $ticket_id){
        if(self::ticketExists($ticket_id)){
            $returnvalue = Assigned::unAssignTicket($user_id, $ticket_id);
            Ticket_Log::createLogEntry( $ticket_id, $user_id, 9);
            return $returnvalue;
        }else{
            return "TICKET_NOT_EXISTING";
        }
    }


    /**
    * forward a ticket to a specific support group.
    * Checks if the ticket exists, if so then it will try to forward the ticket to the support group specified, a log entry will be written about this.
    * if no log entry should be written then the user_id should be 0, else te $user_id will be used in the log to specify who forwarded it.
    * @param $user_id the id of user trying to forward the ticket.
    * @param $ticket_id the id of the ticket that we try to forward to a support group.
    * @param $group_id the id of the support group.
    * @return SUCCESS_FORWARDED, TICKET_NOT_EXISTING or INVALID_SGROUP
    */
    public static function forwardTicket($user_id, $ticket_id, $group_id){
        if(self::ticketExists($ticket_id)){
            if(isset($group_id) && $group_id != ""){
                //forward the ticket
                $returnvalue = Forwarded::forwardTicket($group_id, $ticket_id);

                if($user_id != 0){
                    //unassign the ticket incase the ticket is assined to yourself
                    self::unAssignTicket($user_id, $ticket_id);
                    //make a log entry of this action
                    Ticket_Log::createLogEntry( $ticket_id, $user_id, 8, $group_id);
                }
                return $returnvalue;
            }else{
                return "INVALID_SGROUP";
            }
        }else{
            return "TICKET_NOT_EXISTING";
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
    * @param $values should be an array of the form array('TId' => ticket_id, 'Title' => title, 'Status'=> status, 'Timestamp' => ts, 'Queue' => queue,
    * 'Ticket_Category' => tc, 'Author' => author, 'Priority' => priority).
    */
    public function set($values){
        if(isset($values['TId'])){
            $this->tId = $values['TId'];
        }
        $this->title = $values['Title'];
        $this->status = $values['Status'];
        $this->timestamp = $values['Timestamp'];
        $this->queue = $values['Queue'];
        $this->ticket_category = $values['Ticket_Category'];
        $this->author = $values['Author'];
        $this->priority = $values['Priority'];
    }


    /**
    * creates a new 'ticket' entry.
    * this method will use the object's attributes for creating a new 'ticket' entry in the database.
    */
    public function create(){
        $dbl = new DBLayer("lib");
        $this->tId = $dbl->executeReturnId("ticket", Array('Title' => $this->title, 'Status' => $this->status, 'Queue' => $this->queue, 'Ticket_Category' => $this->ticket_category, 'Author' => $this->author, 'Priority' => $this->priority), array('Timestamp'=>'now()'));
    }


    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a TId (ticket id).
    * @param $id the id of the ticket that should be loaded
    */
    public function load_With_TId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("ticket", array('id' => $id), "TId=:id");
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


    /**
    * update the objects attributes to the db.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $dbl->update("ticket", Array('Timestamp' => $this->timestamp, 'Title' => $this->title, 'Status' => $this->status, 'Queue' => $this->queue, 'Ticket_Category' => $this->ticket_category, 'Author' => $this->author, 'Priority' => $this->priority), "TId=$this->tId");
    }


    /**
    * check if a ticket has a ticket_info page or not.
    * @return true or false
    */
    public function hasInfo(){
        return Ticket_Info::TicketHasInfo($this->getTId());
    }


    ////////////////////////////////////////////Getters////////////////////////////////////////////////////

    /**
    * get tId attribute of the object.
    */
    public function getTId(){
        return $this->tId;
    }

    /**
    * get timestamp attribute of the object in the format defined in the outputTime function of the Helperclass.
    */
    public function getTimestamp(){
        return Helpers::outputTime($this->timestamp);
    }

    /**
    * get title attribute of the object.
    */
    public function getTitle(){
        return $this->title;
    }

    /**
    * get status attribute of the object.
    */
    public function getStatus(){
        return $this->status;
    }

    /**
    * get status attribute of the object in the form of text (string).
    */
    public function getStatusText(){
        $statusArray = Ticket::getStatusArray();
        return $statusArray[$this->getStatus()];
    }

    /**
    * get category attribute of the object in the form of text (string).
    */
    public function getCategoryName(){
        $category = Ticket_Category::constr_TCategoryId($this->getTicket_Category());
        return $category->getName();
    }

    /**
    * get queue attribute of the object.
    */
    public function getQueue(){
        return $this->queue;
    }

    /**
    * get ticket_category attribute of the object (int).
    */
    public function getTicket_Category(){
        return $this->ticket_category;
    }

    /**
    * get author attribute of the object (int).
    */
    public function getAuthor(){
        return $this->author;
    }

    /**
    * get priority attribute of the object (int).
    */
    public function getPriority(){
        return $this->priority;
    }

    /**
    * get priority attribute of the object in the form of text (string).
    */
    public function getPriorityText(){
        $priorityArray = Ticket::getPriorityArray();
        return $priorityArray[$this->getPriority()];
    }

    /**
    * get the user assigned to the ticket.
    * or return 0 in case not assigned.
    */
    public function getAssigned(){
        $user_id = Assigned::getUserAssignedToTicket($this->getTId());
        if ($user_id == ""){
            return 0;
        }else{
            return $user_id;
        }
    }

    /**
    * get the name of the support group to whom the ticket is forwarded
    * or return 0 in case not forwarded.
    */
    public function getForwardedGroupName(){
        $group_id = Forwarded::getSGroupOfTicket($this->getTId());
        if ($group_id == ""){
            return 0;
        }else{
            return Support_Group::getGroup($group_id)->getName();
        }
    }

    /**
    * get the id of the support group to whom the ticket is forwarded
    * or return 0 in case not forwarded.
    */
    public function getForwardedGroupId(){
        $group_id = Forwarded::getSGroupOfTicket($this->getTId());
        if ($group_id == ""){
            return 0;
        }else{
            return $group_id;
        }
    }
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    /**
    * set tId attribute of the object.
    * @param $id integer id of the ticket
    */
    public function setTId($id){
        $this->tId = $id;
    }

    /**
    * set timestamp attribute of the object.
    * @param $ts timestamp of the ticket
    */
    public function setTimestamp($ts){
        $this->timestamp = $ts;
    }

    /**
    * set title attribute of the object.
    * @param $t title of the ticket
    */
    public function setTitle($t){
        $this->title = $t;
    }

    /**
    * set status attribute of the object.
    * @param $s status of the ticket(int)
    */
    public function setStatus($s){
        $this->status = $s;
    }

    /**
    * set queue attribute of the object.
    * @param $q queue of the ticket
    */
    public function setQueue($q){
        $this->queue = $q;
    }

    /**
    * set ticket_category attribute of the object.
    * @param $tc ticket_category id of the ticket(int)
    */
    public function setTicket_Category($tc){
        $this->ticket_category = $tc;
    }

    /**
    * set author attribute of the object.
    * @param $a author of the ticket
    */
    public function setAuthor($a){
        $this->author = $a;
    }

    /**
    * set priority attribute of the object.
    * @param $p priority of the ticket
    */
    public function setPriority($p){
        $this->priority = $p;
    }

    /**
    * function that creates a ticket Attachment.
    */
    public static function add_Attachment($TId,$filename,$author,$tempFile){

        global $FILE_STORAGE_PATH;
        $length = mt_rand(20, 25);
        $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$-_.+!*\'(),';
        $randomString = '';
        for ($i = 0; $i < $length; $i++) {
            $randomString .= $characters[rand(0, strlen($characters) - 1)];
        }
        $targetFile = $FILE_STORAGE_PATH . $randomString . "/" . $filename;
        
        if(file_exists($targetFile)) { return self::add_Attachment($TId,$filename,$author,$tempFile); }
        
        $ticket = new Ticket();
        $ticket->load_With_TId($TId);
    
        //create the attachment!
        $dbl = new DBLayer("lib");
        $dbl->insert("`ticket_attachments`", Array('ticket_TId' => $TId, 'Filename' => $filename, 'Filesize' => filesize($tempFile), 'Uploader' => $author, 'Path' => $randomString . "/" . $filename));
        mkdir($FILE_STORAGE_PATH . $randomString);
        move_uploaded_file($tempFile,$targetFile);
        
        //write a log entry
        Ticket_Log::createLogEntry( $TId, $author, 10);

        return true;
    }
    
}
