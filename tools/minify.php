<?php
$output = "<?hh\n";
$stdin = fopen('php://stdin', 'r');
while(!feof($stdin)){
    $line = fgets($stdin);
    preg_match("/^\s*(.*)/i", $line, $match);
    $match[1] = preg_replace("/\/\/.*\$/i", '', $match[1]);
    $output.=$match[1];
}
fclose($stdin);
$output = preg_replace("/\/\*(.*?)\*\//i", '', $output);
echo $output;
