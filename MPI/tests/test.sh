
for i in input*.txt
do
    read -r x y < $i
    task_num=$(($y*$y+1)) 
    echo -n "$i: "
    mpirun -n $task_num ../labyrinth.out $i
done





