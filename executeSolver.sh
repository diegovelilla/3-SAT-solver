#!/bin/bash

# execute with: bash executeSolver.sh
# or execute with: ./executeSolver.sh

start_measuring_time() {
  read start < <(date +'%s')
}

stop_measuring_time() {
  read end < <(date +'%s')
}

show_elapsed_time() {
  echo "$((end-start)) s"
}

for f in random3SAT/vars*.cnf

do
    echo
    echo "------------------"
    echo $f
    echo "Solver:"
    start_measuring_time
    ./SAT-solver < $f
    stop_measuring_time
    show_elapsed_time
done
