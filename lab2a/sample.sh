#!/bin/bash
rm -rf lab2_add.csv
rm -rf lab2_list.csv
threads=(1 2 4 8 12)
iterations=(1 10 100 1000 10000 10000)
syncoptsadd=(m s c)
for i in "${threads[@]}"; do
	for j in "${iterations[@]}"; do
		./lab2_add --threads=$i --iterations=$j >> lab2_add.csv
	done
done		
for i in "${threads[@]}"; do
	for j in "${iterations[@]}"; do
		./lab2_add --threads=$i --iterations=$j --yield >> lab2_add.csv
	done
done		
for i in "${threads[@]}"; do
	for j in "${iterations[@]}"; do
		for k in "${syncoptsadd[@]}"; do
			./lab2_add --threads=$i --iterations=$j --sync=$k >> lab2_add.csv
		done
	done
done
for i in "${threads[@]}"; do
	for j in "${iterations[@]}"; do
		for k in "${syncoptsadd[@]}"; do
			./lab2_add --threads=$i --iterations=$j --sync=$k --yield >> lab2_add.csv
		done
	done
done
list_its1=(10 100 1000 10000 20000)
list_t2=(2 4 8 12)
list_its2=(10 100 1000)
list_t4=(1 2 4 8 12)
list_its4=(2 4 8 16 32)
list_t5=(1 2 4 8 12 16 24)
yields=(i d l id il dl idl)
syncoptslist=(m s)
for i in "${list_its1[@]}"; do
	./lab2_list --threads=1 --iterations=$i >> lab2_list.csv
done	
for i in "${list_t2[@]}"; do
	for j in "${list_its2[@]}"; do
		timeout 0.8 ./lab2_list --threads=$i --iterations=$j >> lab2_list.csv
	done
done
for i in "${list_t2[@]}"; do
	for j in "${list_its2[@]}"; do
		for k in "${yields[@]}"; do
			timeout 0.4 ./lab2_list --threads=$i --iterations=$j --yield=$k >> lab2_list.csv
		done
	done
done
for i in "${list_t4[@]}"; do
	for j in "${list_its4[@]}"; do
		for k in "${yields[@]}"; do
				./lab2_list --threads=$i --iterations=$j --yield=$k --sync=m >> lab2_list.csv
		done
	done
done
for i in "${list_t4[@]}"; do
	for j in "${list_its4[@]}"; do
		for k in "${yields[@]}"; do
				./lab2_list --threads=$i --iterations=$j --yield=$k --sync=s >> lab2_list.csv
		done
	done
done
for i in "${list_t5[@]}"; do
	for j in "${syncoptslist[@]}"; do
		./lab2_list --threads=$i --sync=$j --iterations=1000 >> lab2_list.csv
	done
done
