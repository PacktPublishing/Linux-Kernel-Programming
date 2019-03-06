# gnuplot 'script'
# plot_actualsz_vs_waste.p
# Run it like this:
# gnuplot> plot 'plot_actualsz_vs_waste.p'
set autoscale                          # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title "Slab/Page Allocator: Requested vs Actually allocated size Wastage in Percent"
set xlabel "Required size"
set ylabel "%age Waste"
plot "2plotdata.txt" using 1:10 title 'Required size' with points, "2plotdata.txt" using 1:1 title 'Wastage' with linespoints
