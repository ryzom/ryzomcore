 <?php
 
class Mail_Handler{
    
    private $db;
        
    public function mail_fork() {   
        //Start a new child process and return the process id!
        $pid = pcntl_fork();
        return $pid;
        
    }
    
    
    function get_email_by_user_id($id){
        $user = new Ticket_User();
        $user->load_With_TUserId($id);
        $webUser = new WebUsers($user->getExternId());
        return $webUser->getEmail();      
    }
    
    function get_username_from_id($id){
        $user = new Ticket_User();
        $user->load_With_TUserId($id);
        $webUser = new WebUsers($user->getExternId());
        return $webUser->getUsername();   
    }
    
    
    function get_id_from_username($username){
        $externId = WebUsers::getId($username);
        $user = Ticket_User::constr_ExternId($externId);
        return $user->getTUserId();   
    }
    
    function get_id_from_email($email){
        $webUserId = WebUsers::getIdFromEmail($email);
        $user = Ticket_User::constr_ExternId($webUserId);
        return $user->getTUserId();    
    }
    
    public static function send_mail($recipient, $subject, $body, $from = NULL) {
    
        if(is_numeric($recipient)) {
            $id_user = $recipient;
            $recipient = NULL;
        }
        $query = "INSERT INTO email (Recipient,Subject,Body,Status,UserId,Sender) VALUES (:recipient, :subject, :body, :status, :id_user, :sender)";
        $values = array('recipient' => $recipient, 'subject' => $subject, 'body' => $body, 'status' => 'NEW', 'id_user' => $id_user, 'sender' => $from);
        $db = new DBLayer("lib");
        $db->execute($query, $values);
        
    }
    
     
    //the main function
    function cron() {
        global $cfg;
        $inbox_username = $cfg['mail']['username'];
        $inbox_password = $cfg['mail']['password'];
        $inbox_host = $cfg['mail']['host'];
        $oms_reply_to = "OMS <oms@".$inbox_host.">";
        global $MAIL_DIR;
        
        // Deliver new mail
        echo("mail cron\n");
        
        //creates child process
        $pid = self::mail_fork();
        $pidfile = '/tmp/ams_cron_email_pid';
    
        //INFO: if $pid = 
        //-1: "Could not fork!\n";
        // 0: "In child!\n";
        //>0: "In parent!\n";
        
        if($pid) {
        
            // We're the parent process, do nothing!
        
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
                        $email['Recipient'] = self::get_email_by_user_id($email['UserId']);
                    }
                    
                    //create sending email adres based on the $sender id
                    if($email['Sender']) {
                        $username = self::get_username_from_id($email['Sender']);          
                        $from =  "$username <$username@$inbox_host>";          
                    } else {
                        $from = $oms_reply_to;          
                    }
                    $headers = "From: $from\r\n" . "Message-ID: " . $message_id ;
                    print("recip: " . $email['Recipient']);
                    print("subj: " .$email['Subject']);
                    print("body: " . $email['Body']);
                    print("headers: " . $headers);
                    if(mail($email['Recipient'], $email['Subject'], $email['Body'], $headers)) {       
                        $status = "DELIVERED";        
                        echo("Emailed {$email['Recipient']}\n");        
                    } else {       
                        $status = "FAILED";
                        echo("Email to {$email['Recipient']} failed\n");
                    }
                    //change the status of the emails.
                    $this->db->execute('update email set Status = ?, MessageId = ?, Attempts = Attempts + 1 where MailId = ?', array($status, $message_id, $email['MailId']));
                    //db_exec('update email set status = ?, message_id = ?, attempts = attempts + 1 where id_email = ?', array($status, $message_id, $email['id_email']));
                }
                unlink($pidfile);
            }
            // Check mail
            
            //$mailbox = imap_open("{localhost:110/pop3/novalidate-cert}INBOX", $inbox_username, $inbox_password);
            $mbox = imap_open($cfg['mail']['server'], $inbox_username, $inbox_password) or die('Cannot connect to mail server: ' . imap_last_error());
            $message_count = imap_num_msg($mbox);
    
            for ($i = 1; $i <= $message_count; ++$i) {
                
                //return task ID
                self::incoming_mail_handler($mbox, $i);
                $tid = 1; //self::ams_create_email($from, $subject, $txt, $html, $to, $from);
    
                if($tid) {
                    //TODO: base file on Ticket + reply id
                   /* $file = fopen($MAIL_DIR."/mail/".$tid, 'w');      
                    fwrite($file, $entire_email);     
                    fclose($file);     */
                }
                //mark message $i of $mbox for deletion!
                imap_delete($mbox, $i);
            }
            //delete marked messages
            imap_expunge($mbox);  
            imap_close($mbox);
            
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
        print('got it from subject!');
        $startpos = strpos($subject, "[Ticket #");
        $tempString = substr($subject, $startpos+9);
        $endpos = strpos($tempString, "]");
        $ticket_id = substr($tempString, 0, $endpos);
        return $ticket_id;
    }
    
    
    function incoming_mail_handler($mbox,$i){
        
        $header = imap_header($mbox, $i);
        $subject = self::decode_utf8($header->subject);
        
        print_r($header);
        
        //get ticket_id out of the message-id or else out of the subject line
        $ticket_id = 0;
        if(isset($header->references)){
            $pieces = explode(".", $header->references);
            if($pieces[0] == "<ams"){
                print('got it from message-id');
                $ticket_id = $pieces[2];
            }else{
                $ticket_id = self::get_ticket_id_from_subject($subject);
            }
        }else{
            print('elseeee');
            $ticket_id = self::get_ticket_id_from_subject($subject);
        }
       
        //if ticket id is found
        if($ticket_id){
            
            $entire_email = imap_fetchheader($mbox, $i) . imap_body($mbox, $i);   
            $subject = self::decode_utf8($header->subject);    
            $to = $header->to[0]->mailbox;   
            $from = $header->from[0]->mailbox . '@' . $header->from[0]->host; 
            $txt = self::get_part($mbox, $i, "TEXT/PLAIN");   
            $html = self::get_part($mbox, $i, "TEXT/HTML");
            
            //get the id out of the email address of the person sending the email.
            if($from !== NULL && !is_numeric($from)) $from = self::get_id_from_email($from);
            
            $user = new Ticket_User();
            $user->load_With_TUserId($from);
            $ticket = new Ticket();
            $ticket->load_With_TId($ticket_id);
            
            //if user has access to it!
            if($user->isMod() or ($ticket->getAuthor() == $user->getTUserId())){
                
            }
            /*print("================");
            print("subj: ".$subject);
            print("to: ".$to);
            print("from: ".$from);
            print("txt: " .$txt);
            print("html: ".$html);*/
        }
        
    }
    
    /*function ams_create_email($from, $subject, $body, $html, $recipient = 0, $sender = NULL) {
    
        //TODO:
        if($recipient == 0 && !is_string($recipient)) {
            global $user;
            $recipient = $user->uid;
        }
    
        if($sender !== NULL && !is_numeric($sender)) $sender = self::get_id_from_username($sender);
        if(!is_numeric($recipient)) $recipient = self::get_id_from_username($recipient);
    
        $message = array(
        'creator' => $sender,
        'owner' => $recipient,
        'type' => 'email',
        'summary' => $subject,
        'data' => array (
            'subject' => $subject,
            'body' => $body,
            'html' => $html,
            'sender' => oms_get_username_from_id($sender),
            'from' => $from,
            'recipient' => oms_get_username_from_id($recipient),
            'time' => time(),
            ),
        );
        
        //TO ASK:
        oms_task_create($message);
        oms_task_index($message, array('subject', 'body', 'sender', 'recipient'));
        //---------------------------
        return $message['id_task'];
    }*/
    
     
    
    /*function oms_get_email($id) {
    
        $message = oms_task_load($id);
        if($message) {
            oms_prepare_email($message);
            return $message;
        } else {
            return FALSE; 
        }
    
    }*/
    
     
    
    /*function oms_prepare_email(&$message) {
    
        $data = $message['data'];
        $data['id_message'] = $message['id_task'];
        $data['read'] = ($message['status'] != 'NEW' && $message['status'] != 'UNREAD');
        $message = $data;
    
    }*/
    
     
    
    /*function oms_email_mark_read($mid) {
    
        db_exec("update task set status = 'READ' where id_task = ? and type = 'email' and module = 'email'", array($mid));
    
    }*/
    
     
    
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