<?php
/**
* handler for performing changes when shard is back online after being offline.
* the sync class is responsible for the syncdata function, which will synchronise the website with the shard
* (when the shard is offline, users can still change their password, email or even register)
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Sync{

    const OS_UNKNOWN = 1;
    const OS_WIN = 2;
    const OS_LINUX = 3;
    const OS_OSX = 4;

    /**
     * performs the actions listed in the querycache.
     * All entries in the querycache will be read and performed depending on their type.
     * This is done because the shard could have been offline and we want changes made on the website (which is still online) to eventually hit the shard.
     * These changes are: createPermissions, createUser, change_pass, change_mail
     */
    static public function syncdata ($display = false) {

    if (function_exists('pcntl_fork')) {
        $pid = pcntl_fork();
    }
    global $AMS_TMPDIR;
    $pidfile = $AMS_TMPDIR.'/ams_cron_pid';

        if(isset($pid) and function_exists('pcntl_fork') ) {
        // We're the main process.
        } else {
            $pid = getmypid();
            if(Sync::check_for_pid(@file_get_contents($pidfile))) {
                $file = fopen($pidfile, 'w+');
                if (!$file) {
                    echo $pidfile.' is not writeable.';
                    error_log($pidfile.' is not writeable.');
                    throw new SystemExit();
                }
                fwrite($file, $pid);
                fclose($file);

                try {
                    $dbl = new DBLayer("lib");
                    $statement = $dbl->executeWithoutParams("SELECT * FROM ams_querycache");
                    $rows = $statement->fetchAll();
                    foreach ($rows as $record) {

                        $db = new DBLayer($record['db']);
                        switch($record['type']) {
                            case 'createPermissions':
                                $decode = json_decode($record['query']);
                                $values = array('username' => $decode[0]);
                                //make connection with and put into shard db & delete from the lib
                                $sth=$db->selectWithParameter("UId", "user", $values, "Login= :username" );
                                $result = $sth->fetchAll();
                                /*foreach ($result as $UId) {
                                    $ins_values = array('UId' => $UId['UId']);
                                    $ins_values['ClientApplication'] = "r2";
                                    $ins_values['AccessPrivilege'] = "OPEN";
                                    $db->insert("permission", $ins_values);
                                    $ins_values['ClientApplication'] = 'ryzom_open';
                                    $db->insert("permission",$ins_values);
                                }*/ // FIXME: GARBAGE
                                break;
                            case 'change_pass':
                                $decode = json_decode($record['query']);
                                $values = array('Password' => $decode[1]);
                                //make connection with and put into shard db & delete from the lib
                                $db->update("user", $values, "Login = '$decode[0]'");
                                break;
                            case 'change_mail':
                                $decode = json_decode($record['query']);
                                $values = array('Email' => $decode[1]);
                                //make connection with and put into shard db & delete from the lib
                                $db->update("user", $values, "Login = '$decode[0]'");
                                break;
                            case 'createUser':
                                $decode = json_decode($record['query']);
                                $values = array('Login' => $decode[0], 'Password' => $decode[1], 'Email' => $decode[2] );
                                //make connection with and put into shard db & delete from the lib
                                $db->insert("user", $values);
                                break;
                        }
                        $dbl->delete("ams_querycache", array('SID' => $record['SID']), "SID=:SID");
                    }
                    if ($display == true) {
                        print('Syncing completed');
                    }
                }
                catch (PDOException $e) {
                    if ($display == true) {
                        print('Something went wrong! The shard is probably still offline!');
                        print_r($e);
                    }
                }
                unlink($pidfile);
            }

        }
    }

    public static function check_for_pid($pid){

        $OS = Sync::getOS();

        if ($OS == 2) {
            $processes = explode( "\n", shell_exec( "tasklist.exe" ));
            foreach( $processes as $key => $value )
            {
                 if( empty($value) != '1' && strpos( "Image Name", $value ) === 0
                   || empty($value) != '1' && strpos( "===", $value ) === 0 )
                      continue;
                 $matches = false;
                 preg_match( "/(.*?)\s+(\d+).*$/", $value, $matches );
                 if (isset($matches[ 2 ]) && $pid = $matches[ 2 ]) {
                    return true;
                 }
            }
        } else {
            return file_exists( "/proc/".$pid );
        }
    }
    static public function getOS() {
        switch (true) {
            case stristr(PHP_OS, 'DAR'): return self::OS_OSX;
            case stristr(PHP_OS, 'WIN'): return self::OS_WIN;
            case stristr(PHP_OS, 'LINUX'): return self::OS_LINUX;
            default : return self::OS_UNKNOWN;
        }
    }
}
