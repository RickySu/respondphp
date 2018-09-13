<?php
$autoloadFile=__DIR__.'/../vendor/autoload.php';

if (!($loader=@include $autoloadFile)) {
    die("please use auto loader!\n");
}
