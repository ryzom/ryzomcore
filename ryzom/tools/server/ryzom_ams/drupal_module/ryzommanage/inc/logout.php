<?php

function logout(){
    session_unset();
    session_destroy();
    global $INGAME_WEBPATH;
    header("Location: ". $INGAME_WEBPATH);
    exit;
}
