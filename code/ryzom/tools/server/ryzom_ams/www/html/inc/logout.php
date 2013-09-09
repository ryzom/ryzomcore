<?php

function logout(){
    session_unset();
    session_destroy();
}
