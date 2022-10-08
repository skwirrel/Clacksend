set datafile separator ','
set term qt size 1800, 800
plot "/tmp/gnuplotData.csv" using 0:1 with lines title "Raw samples", '' using 0:2 with lines title "Rolling average samples", '' using 0:3 with lines title "Rolling maximum", '' using 0:4 with lines title "Rolling minimum", '' using 0:5 with lines title "Rolling median"
