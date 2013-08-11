 <?php
 
class Mail_Handler{
    
    private $db;
        
    public function mail_fork() {
        /*global $db;
        $db = NULL;
        $pid = pcntl_fork();
        oms_db_connect();
        return $pid;*/
       
        //Start a new thread and return the thread id!
        $pid = pcntl_fork();
        return $pid;
        
    }
    
    /*
    function oms_db_connect() {
        global $db;
        global $db_host, $db_name, $db_user, $db_pass;
        if(!isset($db)) {
            try {
                    $db = new PDO(
                    "mysql:host=$db_host;dbname=$db_name",
                    $db_user,
                    $db_pass,
                    array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES utf8")
                );
                $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
                //$db->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
                oms_error_log("oms_db_connect: database connection established", 3);
            } catch(PDOException $e) {
                print "Database error: " . $e->getMessage() . "<br/>";
                die();
            }
        } else {
            oms_error_log("oms_db_connect: already connected");
        }
    }*/
    
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
    
    
    function oms_mail_send($recipient, $subject, $body, $from = NULL) {
    
        if(is_numeric($recipient)) {
            $id_user = $recipient;
            $recipient = NULL;
        }
        db_insert('email', array('recipient' => $recipient, 'subject' => $subject, 'body' => $body, 'status' => 'NEW', 'id_user' => $id_user, 'sender' => $from));
    }
    
     
    
    function cron() {
        global $cfg;
        $inbox_username = $cfg['mail']['username'];
        $inbox_password = $cfg['mail']['password'];
        $inbox_host = $cfg['mail']['host'];
        $oms_reply_to = "OMS <oms@".$inbox_host.">";
        global $basedir;
        
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
                    $message_id = self::new_message_id();
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
                    $headers = "From: $from\r\n" . "Message-ID: " . $message_id;
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
            /*
            //$mailbox = imap_open("{localhost:110/pop3/novalidate-cert}INBOX", $inbox_username, $inbox_password);
            $mbox = imap_open($cfg['mail']['server'], $inbox_username, $inbox_password);
            $message_count = imap_num_msg($mbox);
    
            for ($i = 1; $i <= $message_count; ++$i) {
                $header = imap_header($mbox, $i);    
                $entire_email = imap_fetchheader($mbox, $i) . imap_body($mbox, $i);   
                $subject = decode_utf8($header->subject);    
                $to = $header->to[0]->mailbox;   
                $from = $header->from[0]->mailbox . '@' . $header->from[0]->host; 
                $txt = get_part($mbox, $i, "TEXT/PLAIN");   
                $html = get_part($mbox, $i, "TEXT/HTML");
                //return task ID
                $tid = oms_create_email($from, $subject, $txt, $html, $to, $from);
    
                if($tid) {     
                    $file = fopen("$basedir/mail/$tid", 'w');      
                    fwrite($file, $entire_email);     
                    fclose($file);     
                }    
                imap_delete($mbox, $i);
            }
            imap_expunge($mbox);  
            imap_close($mbox);
            */
        }
    
    }
    
     
    
    function new_message_id() {
    
        $time = time();
        $pid = getmypid();
        global $cfg;
        global $ams_mail_count;
        $ams_mail_count = ($ams_mail_count == '') ? 1 : $ams_mail_count + 1;
        return "<ams.message".$pid.$ams_mail_count.$time."@".$cfg['mail']['host'].">";
    
    }
    
     
    
    function oms_create_email($from, $subject, $body, $html, $recipient = 0, $sender = NULL) {
    
        if($recipient == 0 && !is_string($recipient)) {
            global $user;
            $recipient = $user->uid;
        }
    
        if($sender !== NULL && !is_numeric($sender)) $sender = oms_get_id_from_username($sender);
        if(!is_numeric($recipient)) $recipient = oms_get_id_from_username($recipient);
    
        oms_error_log("Sending message: '$subject' from $sender to $recipient", 3);
    
        $message = array(
        'creator' => $sender,
        'owner' => $recipient,
        'id_module' => 'email',
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
    }
    
     
    
    function oms_get_email($id) {
    
        $message = oms_task_load($id);
        if($message) {
            oms_prepare_email($message);
            return $message;
        } else {
            return FALSE; 
        }
    
    }
    
     
    
    function oms_prepare_email(&$message) {
    
        $data = $message['data'];
        $data['id_message'] = $message['id_task'];
        $data['read'] = ($message['status'] != 'NEW' && $message['status'] != 'UNREAD');
        $message = $data;
    
    }
    
     
    
    function oms_email_mark_read($mid) {
    
        db_exec("update task set status = 'READ' where id_task = ? and type = 'email' and module = 'email'", array($mid));
    
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
            if($mime_type == get_mime_type($structure)) {
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
                    $data = get_part($stream, $msg_number, $mime_type, $sub_structure,$prefix .    ($index + 1));
                    if($data) {
                        return $data;
                    }
                } // END OF WHILE
            } // END OF MULTIPART
        } // END OF STRUTURE
        return false; 
    
    } // END OF FUNCTION
    
}