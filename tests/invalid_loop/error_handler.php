<?php
set_error_handler(function($errno, $errstr, $errfile, $errline) use($class){         
    True("Argument 1 passed to $class::__construct must be an instance of UVLoop, stdClass given" == $errstr, $class);
    die;
});