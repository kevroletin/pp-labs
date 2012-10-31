
gnuplot --persist -e 'plot "merge_00.res" title "merge+insert" with lines,  "quick_00.res" title "quick+insert" with lines, "merge_01.res" title "merge+qsort" with lines, "quick_01.res" title "qsort+qsort" with lines, "merge_02.res" title "merge+merge" with lines, "quick_02.res" title "quick+merge" with lines'
