set datafile separator ','
set term qt size 1800, 800
plot "/tmp/gnuplotData.csv" using 0:1 with lines title "Frequency distribution histogram" at end, '' using 0:2 with lines title "FDH (after processing" at end, '' using 0:3 with lines title "Thresholds" at end
