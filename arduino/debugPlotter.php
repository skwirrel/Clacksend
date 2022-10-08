<?php

$fh = fopen('php://stdin', 'r');
$output = false;
$filename = '/tmp/gnuplotData.csv';

while( $line = fgets($fh) ) {
    if(!strlen(trim($line))) continue;
    if (strpos($line,'=========')===0) {
        if (!$output) {
            echo "hoo";
            $output = fopen($filename,'wa+');
        } else {
            fclose($output);
            `gnuplot -p debugThresholds.gnuplot`;
            exit();
        }
    } else {
        if ($output) fputs($output,$line);
    }
}
