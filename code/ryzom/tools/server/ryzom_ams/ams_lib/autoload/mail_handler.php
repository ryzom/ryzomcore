 <?php
 
class Mail_Handler{
    
    private $db;
        
    public function mail_fork() {   
        //Start a new child process and return the process id!
        $pid = pcntl_fork();
        return $pid;
        
    }
    
    
    public static function send_ticketing_mail($ticketObj, $content, $type, $sendingGroupId = 0) {
        global $TICKET_MAILING_SUPPORT;
        if($TICKET_MAILING_SUPPORT){
            //$txt = "";
            //$subject = "";
            if($sendingGroupId == 0){
                //if it is not forwarded (==public == which returns 0) then make it NULL which is needed to be placed in the DB.
                $sendingGroupId = NULL;
            }
            if($type == "REPLY"){
                $txt = "---------- Ticket #". $ticketObj->getTId() . " ----------\n You received a new reply on your ticket: " . $ticketObj->getTitle() .
                "\n --------------------\n\n";
                $subject = "New reply on [Ticket #" . $ticketObj->getTId() ."]";
                $endTxt = "\n\n----------\nYou can reply on this message to answer directly on the ticket!";
                $txt = $txt . $content . $endTxt;
                self::send_mail($ticketObj->getAuthor(),$subject,$txt, $ticketObj->getTId(),$sendingGroupId);
            }else if($type == "NEW"){
                $txt = "---------- Ticket #". $ticketObj->getTId() . " ----------\n Your ticket: " . $ticketObj->getTitle() . " is newly created";
                $txt = $txt . "\n --------------------\n\n";
                $subject = "New ticket created [Ticket #" . $ticketObj->getTId() ."]";
                $endTxt = "\n\n----------\nYou can reply on this message to answer directly on the ticket!";
                $txt = $txt . $content . $endTxt;
                self::send_mail($ticketObj->getAuthor(),$subject,$txt, $ticketObj->getTId(), $sendingGroupId);
            }
            
        }
    } 
    
    
    public static function send_mail($recipient, $subject, $body, $ticket_id = 0, $from = NULL) {
        if(is_numeric($recipient)) {
            $id_user = $recipient;
            $recipient = NULL;
        }
        $query = "INSERT INTO email (Recipient,Subject,Body,Status,Attempts,Sender,UserId,MessageId,TicketId) VALUES (:recipient, :subject, :body, :status, :attempts, :sender, :id_user, :messageId, :ticketId)";
        $values = array('recipient' => $recipient, 'subject' => $subject, 'body' => $body, 'status' => 'NEW', 'attempts'=> 0, 'sender' => $from,'id_user' => $id_user,  'messageId' => 0, 'ticketId'=> $ticket_id);
        $db = new DBLayer("lib");
        $db->execute($query, $values);
        
    }
    
     
    //the main function
    function cron() {
        global $cfg;
        $default_groupemail = $cfg['mail']['default_groupemail'];
        $default_groupname = $cfg['mail']['default_groupname'];
        /*
        $inbox_host = $cfg['mail']['host'];
        $oms_reply_to = "Ryzom Ticketing Support <ticketing@".$inbox_host.">";*/
        global $MAIL_DIR;
        
        
        // Deliver new mail
        echo("mail cron\n");
        
        //creates child process
        $pid = self::mail_fork();
        $pidfile = '/tmp/ams_cron_email_pid';
        
        if($pid) {
        
            // We're the parent process, do nothing!
            //INFO: if $pid = 
            //-1: "Could not fork!\n";
            // 0: "In child!\n";
            //>0: "In parent!\n";
        
        } else {
            //make db connection here because the children have to make the connection.
            $this->db = new DBLayer("lib");
            
            //if $pidfile doesn't exist yet, then start sending the mails that are in the db.
            if(!file_exists($pidfile)) {
                
                //create the file and write the child processes id in it!
                $pid = getmypid();
                $file = fopen($pidfile, 'w');
                fwrite($file, $pid);
                fclose($file);
                
                //select all new & failed emails & try to send them
                //$emails = db_query("select * from email where status = 'NEW' or status = 'FAILED'");
                $statement = $this->db->executeWithoutParams("select * from email where Status = 'NEW' or Status = 'FAILED'");
                $emails = $statement->fetchAll();

                foreach($emails as $email) {
                    $message_id = self::new_message_id($email['TicketId']);

                    //if recipient isn't given, then use the email of the id_user instead!
                    echo("Emailing {$email['Recipient']}\n");
                    if(!$email['Recipient']) {
                        $email['Recipient'] = Ticket_User::get_email_by_user_id($email['UserId']);
                    }
                    
                    //create sending email adres based on the $sender id which refers to the department id
                    if($email['Sender'] == NULL) {
                        $from =  $default_groupname ." <".$default_groupemail.">";
                    } else {
                        $group = Support_Group::getGroup($email['Sender']);
                        $from = $group->getName()." <".$group->getGroupEmail().">";
                    }
                   
                    $headers = "From: $from\r\n" . "Message-ID: " . $message_id ;
                   
                    if(mail($email['Recipient'], $email['Subject'], $email['Body'], $headers)) {       
                        $status = "DELIVERED";        
                        echo("Emailed {$email['Recipient']}\n");        
                    } else {       
                        $status = "FAILED";
                        echo("Email to {$email['Recipient']} failed\n");
                    }
                    //change the status of the emails.
                    $this->db->execute('update email set Status = ?, MessageId = ?, Attempts = Attempts + 1 where MailId = ?', array($status, $message_id, $email['MailId']));
                   
                }
                unlink($pidfile);
            }
            // Check mail
            $sGroups = Support_Group::getGroups();
            $defaultGroup = new Support_Group();
            $defaultGroup->setSGroupId(0);
            $defaultGroup->setGroupEmail($default_groupemail);
            $defaultGroup->setIMAP_MailServer($cfg['mail']['default_mailserver']);
            $defaultGroup->setIMAP_Username($cfg['mail']['default_username']);
            $defaultGroup->setIMAP_Password($cfg['mail']['default_password']);

            $sGroups[] = $defaultGroup;
            
            foreach($sGroups as $group){
                $mbox = imap_open($group->getIMAP_MailServer(), $group->getIMAP_Username(), $group->getIMAP_Password()) or die('Cannot connect to mail server: ' . imap_last_error());
                $message_count = imap_num_msg($mbox);
        
                for ($i = 1; $i <= $message_count; ++$i) {
                    
                    //return task ID
                    $tid = self::incoming_mail_handler($mbox, $i,$group);
        
                    if($tid) {
                        //TODO: base file on Ticket + timestamp
                        $file = fopen($MAIL_DIR."/mail/ticket".$tid.".".time(), 'w');      
                        fwrite($file, imap_fetchheader($mbox, $i) . imap_body($mbox, $i));     
                        fclose($file);
                        
                        //mark message $i of $mbox for deletion!
                        imap_delete($mbox, $i);
                    }

                }
                //delete marked messages
                imap_expunge($mbox);  
                imap_close($mbox);
            }
        }
    
    }
    
     
    
    function new_message_id($ticketId) {
        $time = time();
        $pid = getmypid();
        global $cfg;
        global $ams_mail_count;
        $ams_mail_count = ($ams_mail_count == '') ? 1 : $ams_mail_count + 1;
        return "<ams.message".".".$ticketId.".".$pid.$ams_mail_count.".".$time."@".$cfg['mail']['host'].">";
    
    }
    
    function get_ticket_id_from_subject($subject){
        $startpos = strpos($subject, "[Ticket #");
        if($startpos){
            $tempString = substr($subject, $startpos+9);
            $endpos = strpos($tempString, "]");
            if($endpos){
                $ticket_id = substr($tempString, 0, $endpos);
            }else{
                $ticket_id = 0;
            }
        }else{
            $ticket_id = 0;
        }
        return $ticket_id;
    }
    
    
    function incoming_mail_handler($mbox,$i,$group){
        
        $header = imap_header($mbox, $i);
        $subject = self::decode_utf8($header->subject);
        $entire_email = imap_fetchheader($mbox, $i) . imap_body($mbox, $i);   
        $subject = self::decode_utf8($header->subject);    
        $to = $header->to[0]->mailbox;   
        $from = $header->from[0]->mailbox . '@' . $header->from[0]->host; 
        $txt = self::get_part($mbox, $i, "TEXT/PLAIN");   
        //$html = self::get_part($mbox, $i, "TEXT/HTML");
        
        //get the id out of the email address of the person sending the email.
        if($from !== NULL && !is_numeric($from)){
            $from = Ticket_User::get_id_from_email($from);
        }
        
        //get ticket_id out of the message-id or else out of the subject line
        $ticket_id = 0;
        if(isset($header->references)){
            $pieces = explode(".", $header->references);
            if($pieces[0] == "<ams"){
                $ticket_id = $pieces[2];
            }else{
                $ticket_id = self::get_ticket_id_from_subject($subject);
            }
        }else{
            $ticket_id = self::get_ticket_id_from_subject($subject);
        }
       
        //if ticket id is found, that means it is a reply on an existing ticket
        if($ticket_id){
            
            //use the line ---------- Ticket # to make a distincton between the old message and the reply
            /*$endpos = strpos($txt, ">---------- Ticket #");
            if($endpos){
                $txt = substr($txt, 0, $endpos);
            }else{
                $endpos = strpos($txt, "---------- Ticket #");
                if($endpos){
                    $txt = substr($txt, 0, $endpos);
                }
            }*/
            
            //if email is sent from an existing email address in the db (else it will give an error while loading the user object)
            if($from != "FALSE"){
                $user = new Ticket_User();
                $user->load_With_TUserId($from);
                $ticket = new Ticket();
                $ticket->load_With_TId($ticket_id);
                
                //if user has access to it!
                if((Ticket_User::isMod($user) or ($ticket->getAuthor() == $user->getTUserId())) and $txt != ""){
                    Ticket::createReply($txt, $user->getTUserId(), $ticket->getTId(),  0);              
                }
                
            }
            print("\n Email found that is a reply to a ticket at:".$group->getGroupEmail());
            return $ticket_id;
            
        }else if($from != "FALSE"){
            
            //if ticket_id isn't found, create a new ticket!
            //if an existing email address mailed the ticket
            
            //if not default group, then forward it by giving the $group->getSGroupId's param
            $newTicketId = Ticket::create_Ticket($subject, $txt,1, $from, $from, $group->getSGroupId());
            
            print("\n Email regarding new ticket found at:".$group->getGroupEmail());
            return $newTicketId;
            
            
        }else{
            //if it's a email that has nothing to do with ticketing, return 0;
            print("\n Email found that isn't a reply or new ticket, at:".$group->getGroupEmail());
            return 0;
        }
        
    }
    
     
    
    function decode_utf8($str) {
    
        preg_match_all("/=\?UTF-8\?B\?([^\?]+)\?=/i",$str, $arr);
        for ($i=0;$i<count($arr[1]);$i++){ 
            $str=ereg_replace(ereg_replace("\?","\?",
               $arr[0][$i]),base64_decode($arr[1][$i]),$str);
        }
        return $str;
    
    }
    
     
    
     
    
    function get_mime_type(&$structure) {
    
        $primary_mime_type = array("TEXT", "MULTIPART","MESSAGE", "APPLICATION", "AUDIO","IMAGE", "VIDEO", "OTHER");
        if($structure->subtype) {
            return $primary_mime_type[(int) $structure->type] . '/' .$structure->subtype;
        }
        return "TEXT/PLAIN";
    
    }
    
     
    
    function get_part($stream, $msg_number, $mime_type, $structure = false, $part_number = false) {
    
        if(!$structure) {
            $structure = imap_fetchstructure($stream, $msg_number);
        }
    
        if($structure) {
            if($mime_type == self::get_mime_type($structure)) {
                if(!$part_number) {
                    $part_number = "1";
                }
                $text = imap_fetchbody($stream, $msg_number, $part_number);
                if($structure->encoding == 3) {
                    return imap_base64($text);
                } else if($structure->encoding == 4) {
                    return imap_qprint($text);
                } else {
                    return $text;
                }
            }
    
            if($structure->type == 1) /* multipart */ {
                while(list($index, $sub_structure) = each($structure->parts)) {
                    if($part_number) {
                        $prefix = $part_number . '.';
                    } else {
                        $prefix = '';
                    }
                    $data = self::get_part($stream, $msg_number, $mime_type, $sub_structure,$prefix .    ($index + 1));
                    if($data) {
                        return $data;
                    }
                } // END OF WHILE
            } // END OF MULTIPART
        } // END OF STRUTURE
        return false; 
    
    } // END OF FUNCTION
    
}