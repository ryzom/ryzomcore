<?php
class Users {
    
/**
 *
 * Function checkUser 
 *
 * @takes        $username
 * @return      string
 *
 * Info: Returns a string based on if the username is valid, if valid then "success" is returned
 *
 */
public function checkUser($username)
{
    if (isset($username)) {
        if (strlen($username) > 12) {
            return "Username must be no more than 12 characters.";
        } elseif (strlen($username) < 5) {
            return "Username must be 5 or more characters.";
        } elseif (!preg_match('/^[a-z0-9\.]*$/', $username)) {
            return "Username can only contain numbers and letters.";
        } elseif (db_query("SELECT COUNT(*) FROM {users} WHERE name = :name", array(
            ':name' => $username
        ))->fetchField()) {
            return "Username " . $username . " is in use.";
        } else {
            return "success";
        }
    } else {
        return "success";
    }
    return "fail";
}
/**
 *
 * Function checkPassword 
 *
 * @takes        $pass
 * @return      string
 *
 * Info: Returns a string based on if the password is valid, if valid then "success" is returned
 *
 */
public function checkPassword($pass)
{
    if (isset($pass)) {
        if (strlen($pass) > 20) {
            return "Password must be no more than 20 characters.";
        } elseif (strlen($pass) < 5) {
            return "Password must be more than 5 characters.";
        } else {
            return "success";
        }
    }
    return "fail";
}
/**
 *
 * Function confirmPassword 
 *
 * @takes        $pass
 * @return      string
 *
 * Info: Verify's $_POST["Password"] is the same as $_POST["ConfirmPass"]
 *
 */
public function confirmPassword()
{
    if (($_POST["Password"]) != ($_POST["ConfirmPass"])) {
        return "Passwords do not match.";
    } else {
        return "success";
    }
    return "fail";
}
/**
 *
 * Function checkEmail 
 *
 * @takes        $email
 * @return      
 *
 * 
 *
 */
public function checkEmail($email)
{
    if (isset($email)) {
        if (!validEmail($email)) {
            return "Email address is not valid.";
        } elseif (db_query("SELECT COUNT(*) FROM {users} WHERE mail = :mail", array(
            ':mail' => $email
        ))->fetchField()) {
            return "Email is in use.";
        } else {
            return "success";
        }
    } else {
        return "success";
    }
    return "fail";
}
public function validEmail($email)
{
    $isValid = true;
    $atIndex = strrpos($email, "@");
    if (is_bool($atIndex) && !$atIndex) {
        $isValid = false;
    } else {
        $domain    = substr($email, $atIndex + 1);
        $local     = substr($email, 0, $atIndex);
        $localLen  = strlen($local);
        $domainLen = strlen($domain);
        if ($localLen < 1 || $localLen > 64) {
            // local part length exceeded
            $isValid = false;
        } else if ($domainLen < 1 || $domainLen > 255) {
            // domain part length exceeded
            $isValid = false;
        } else if ($local[0] == '.' || $local[$localLen - 1] == '.') {
            // local part starts or ends with '.'
            $isValid = false;
        } else if (preg_match('/\\.\\./', $local)) {
            // local part has two consecutive dots
            $isValid = false;
        } else if (!preg_match('/^[A-Za-z0-9\\-\\.]+$/', $domain)) {
            // character not valid in domain part
            $isValid = false;
        } else if (preg_match('/\\.\\./', $domain)) {
            // domain part has two consecutive dots
            $isValid = false;
        } else if (!preg_match('/^(\\\\.|[A-Za-z0-9!#%&`_=\\/$\'*+?^{}|~.-])+$/', str_replace("\\\\", "", $local))) {
            // character not valid in local part unless 
            // local part is quoted
            if (!preg_match('/^"(\\\\"|[^"])+"$/', str_replace("\\\\", "", $local))) {
                $isValid = false;
            }
        }
        if ($isValid && !(checkdnsrr($domain, "MX") || checkdnsrr($domain, "A"))) {
            // domain not found in DNS
            $isValid = false;
        }
    }
    return $isValid;
}
public function generateSALT($length = 2)
{
    // start with a blank salt
    $salt      = "";
    // define possible characters - any character in this string can be
    // picked for use in the salt, so if you want to put vowels back in
    // or add special characters such as exclamation marks, this is where
    // you should do it
    $possible  = "2346789bcdfghjkmnpqrtvwxyzBCDFGHJKLMNPQRTVWXYZ";
    // we refer to the length of $possible a few times, so let's grab it now
    $maxlength = strlen($possible);
    // check for length overflow and truncate if necessary
    if ($length > $maxlength) {
        $length = $maxlength;
    }
    // set up a counter for how many characters are in the salt so far
    $i = 0;
    // add random characters to $salt until $length is reached
    while ($i < $length) {
        // pick a random character from the possible ones
        $char = substr($possible, mt_rand(0, $maxlength - 1), 1);
        // have we already used this character in $salt?
        if (!strstr($salt, $char)) {
            // no, so it's OK to add it onto the end of whatever we've already got...
            $salt .= $char;
            // ... and increase the counter by one
            $i++;
        }
    }
    // done!
    return $salt;
}
}

