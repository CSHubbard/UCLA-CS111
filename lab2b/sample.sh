#!/bin/bash
rm -rf lab2_list.csv
list_t1=(1 2 4 8 12 16 24)
for i in "${list_t1[@]}"; do
	timeout 0.8 ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
done
for i in "${list_t1[@]}"; do
	timeout 0.8 ./lab2_list --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
done
list_t2=(1 2 4 8 16 24)
for i in "${list_t2[@]}"; do
	timeout 0.8 ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
done
list_t3=(1 4 8 12 16)
list_i1=(1 2 4 8 16)
list_i2=(10 20 40 80)
list_sync=(s m)
for i in "${list_t3[@]}"; do
	for j in "${list_i1[@]}"; do
		timeout 0.8 ./lab2_list --lists=4 --threads=$i --iterations=$j --yield=id >> lab2b_list.csv
	done
done
for k in "${list_sync[@]}"; do
	for i in "${list_t3[@]}"; do
		for j in "${list_i2[@]}"; do
			timeout 0.8 ./lab2_list --lists=4 --threads=$i --iterations=$j --sync=$k --yield=id >> lab2b_list.csv
		done
	done
done
list_t4=(1 4 8 12 16)
list_l1=(1 2 4 8 16)
for k in "${list_sync[@]}"; do
	for i in "${list_t4[@]}"; do
		for j in "${list_l1[@]}"; do
			timeout 0.8 ./lab2_list --lists=$j --threads=$i --iterations=1000 --sync=$k --yield=id >> lab2b_list.csv
		done
	done
done