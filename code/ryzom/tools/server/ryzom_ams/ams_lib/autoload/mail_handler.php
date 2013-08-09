 <?php

 

function oms_mail_send($recipient, $subject, $body, $from = NULL) {

    if(is_numeric($recipient)) {
        $id_user = $recipient;
        $recipient = NULL;
    }
    db_insert('email', array('recipient' => $recipient, 'subject' => $subject, 'body' => $body, 'status' => 'NEW', 'id_user' => $id_user, 'sender' => $from));
}

 

function oms_mail_cron() {

    $oms_inbox_username = '123';
    $oms_inbox_password = '456';
    $oms_host = 'test.com';
    $oms_reply_to = "OMS <oms@".$oms_host.">";
    global $basedir;
    
    // Deliver new mail
    echo("mail cron\n");
    
    //TO ASK:
    $pid = oms_fork();
    $pidfile = '/tmp/oms_cron_email_pid';

    if($pid) {

    // We're the main process.

    } else {

        if(!file_exists($pidfile)) {

            $pid = getmypid();
            $file = fopen($pidfile, 'w');
            fwrite($file, $pid);
            fclose($file);
            $emails = db_query("select * from email where status = 'NEW' or status = 'FAILED'");
    
            foreach($emails as $email) {
                $message_id = oms_new_message_id();
                echo("Emailing {$email['recipient']}\n");
                if(!$email['recipient']) {
                    $email['recipient'] = oms_get_email_by_user_id($email['id_user']);
                }
                
                if($email['sender']) {
                    $username = oms_get_username_from_id($email['sender']);          
                    $from =  "$username <$username@$oms_host>";          
                } else {
                    $from = $oms_reply_to;          
                }
                $headers = "From: $from\r\n" . "Message-ID: " . $message_id;

                if(mail($email['recipient'], $email['subject'], $email['body'], $headers)) {       
                    $status = "DELIVERED";        
                    echo("Emailed {$email['recipient']}\n");        
                } else {       
                    $status = "FAILED";
                    echo("Email to {$email['recipient']} failed\n");
                }
                db_exec('update email set status = ?, message_id = ?, attempts = attempts + 1 where id_email = ?', array($status, $message_id, $email['id_email']));
            }
            unlink($pidfile);
        }
        // Check mail
    
        //$mailbox = imap_open("{localhost:110/pop3/novalidate-cert}INBOX", $oms_inbox_username, $oms_inbox_password);
        $mbox = imap_open("{localhost:110/pop3/novalidate-cert}INBOX", $oms_inbox_username, $oms_inbox_password);
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
    }

}

 

function oms_new_message_id() {

    $time = time();
    $pid = getmypid();
    global $oms_mail_count;
    $oms_mail_count = ($oms_mail_count == '') ? 1 : $oms_mail_count + 1;
    return "<oms.message.$pid.$oms_mail_count.$time@dev1.dev.subrigo.net>";

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

 