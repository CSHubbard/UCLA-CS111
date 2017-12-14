#Cody Hubbard
#004843389
#CodySpHubbard@gmail.com

import sys

def CheckInvalid(block_num, inode_num, ind_lvl, block_offset, last_valid):
	if block_num < 0 or block_num > last_valid - 1:
		sys.stdout.write('INVALID ' + ind_lvl + ' ' + str(block_num) + ' IN INODE ' + str(inode_num) + ' AT OFFSET ' + str(block_offset) + '\n')
		return False
	else :
		return True
		
def CheckReserved(block_num, inode_num, ind_lvl, block_offset, first_valid):
	if block_num > 0 and block_num < first_valid:
		sys.stdout.write('RESERVED ' + ind_lvl + ' ' + str(block_num) + ' IN INODE ' + str(inode_num) + ' AT OFFSET ' + str(block_offset) + '\n')
		return False
	else :
		return True
		
def WriteDups(block_num, inode_num, ind_lvl):
	block_offset = 0
	if ind_lvl == 'INDIRECT BLOCK':
		block_offset = 12
	if ind_lvl == 'DOUBLE INDIRECT BLOCK':
		block_offset = 268
	if ind_lvl == 'TRIPLE INDIRECT BLOCK':
		block_offset = 65804

	sys.stdout.write('DUPLICATE ' + ind_lvl + ' ' + str(block_num) + ' IN INODE ' + str(inode_num) + ' AT OFFSET ' + str(block_offset) + '\n')
		
def FileSystem_Audits(sys_sum):

	#Block Consistency Audits
	blockArr = {}

	for line in sys_sum:
		sep_string = line.split(',')

		if sep_string[0] == 'SUPERBLOCK':
			inode_size = int(sep_string[4])
			block_size = int(sep_string[3])

		if sep_string[0] == 'GROUP':
			total_blocks = int(sep_string[2])
			total_inodes = int(sep_string[3])
			fisrt_inode_block = int(sep_string[8].split('\n')[0])
			num_inode_blocks = (inode_size * total_inodes / block_size)
			nonres_block_num = int(fisrt_inode_block + num_inode_blocks)

		if sep_string[0] == 'BFREE':
			block_num = int(sep_string[1].split('\n')[0])
			blockArr[block_num] = 'free'

		if sep_string[0] == 'INODE':
			inode_num = sep_string[1]
			for n in range(12, 27):
				block_num = int(sep_string[n].split('\n')[0])
				if block_num != 0 and n < 24 and  block_num not in blockArr:
					if CheckInvalid(block_num, inode_num, 'BLOCK', 0, total_blocks):
						if CheckReserved(block_num, inode_num, 'BLOCK', 0, nonres_block_num):
							blockArr[block_num] = (inode_num, 'BLOCK')
				elif block_num != 0 and n == 24 and block_num not in blockArr:
					if CheckInvalid(block_num, inode_num, 'INDIRECT BLOCK', 12, total_blocks):
						if CheckReserved(block_num, inode_num, 'INDIRECT BLOCK', 12, nonres_block_num):
							blockArr[block_num] = (inode_num, 'INDIRECT BLOCK')
				elif block_num != 0 and n == 25 and block_num not in blockArr:
					if CheckInvalid(block_num, inode_num, 'DOUBLE INDIRECT BLOCK', 268, total_blocks):
						if CheckReserved(block_num, inode_num, 'DOUBLE INDIRECT BLOCK', 268, nonres_block_num):
							blockArr[block_num] = (inode_num, 'DOUBLE INDIRECT BLOCK')
				elif block_num != 0 and n == 26 and block_num not in blockArr:
					if CheckInvalid(block_num, inode_num, 'TRIPLE INDIRECT BLOCK', 65804, total_blocks):
						if CheckReserved(block_num, inode_num, 'TRIPLE INDIRECT BLOCK', 65804, nonres_block_num):
							blockArr[block_num] = (inode_num, 'TRIPLE INDIRECT BLOCK')
				elif block_num != 0 and blockArr[block_num] == 'free':
					sys.stdout.write('ALLOCATED BLOCK ' + str(block_num) + ' ON FREELIST\n')
				elif block_num != 0 and blockArr[block_num] != 'free':
					ind_lvl = 'BLOCK' 
					if n == 24: 
						ind_lvl = 'INDIRECT BLOCK'
					if n == 25: 
						ind_lvl = 'DOUBLE INDIRECT BLOCK'
					if n == 26:
						ind_lvl = 'TRIPLE INDIRECT BLOCK'
					WriteDups(block_num, inode_num, ind_lvl)
					WriteDups(block_num, blockArr[block_num][0], blockArr[block_num][1])
					
		if sep_string[0] == 'INDIRECT':
			block_num = int(sep_string[5].split('\n')[0])
			inode_num = int(sep_string[1])
			block_offset = int(sep_string[3])
			ind_lvl = 'BLOCK'; 
			if int(sep_string[2]) == 1: 
				ind_lvl = 'INDIRECT BLOCK'
			if int(sep_string[2]) == 2:
				ind_lvl = 'DOUBLE INDIRECT BLOCK'
			if int(sep_string[2]) == 3:
				ind_lvl = 'TRIPLE INDIRECT BLOCK'

			if block_num in blockArr:
				if blockArr[block_num] == 'free':
					sys.stdout.write('ALLOCATED BLOCK ' + str(block_num) + ' ON FREELIST\n')
					continue
			if block_num in blockArr:
				if blockArr[block_num] != 'free':
					WriteDups(block_num, inode_num, ind_lvl)
					WriteDups(block_num, blockArr[block_num][0], blockArr[block_num][1])
					continue
			if CheckInvalid(block_num, inode_num, ind_lvl, block_offset, total_blocks):
				if CheckReserved(block_num, inode_num, ind_lvl, block_offset, nonres_block_num):
					blockArr[block_num] = (inode_num, ind_lvl)

	for n in range(nonres_block_num, total_blocks):
		if n not in blockArr:
			sys.stdout.write('UNREFERENCED BLOCK ' + str(n) + '\n')
			
	sys_sum.seek(0)
	
	#I-node Allocation Audits
	inodeArr = {}

	for line in sys_sum:
		sep_string = line.split(',')
		inode_num = int(sep_string[1])
		if sep_string[0] == 'SUPERBLOCK':
			total_inodes = int(sep_string[2])
			non_res_inode = int(sep_string[7].split('\n')[0])
		elif sep_string[0] == 'IFREE':
			inodeArr[inode_num] = 'free'
		elif sep_string[0] == 'INODE' and inode_num not in inodeArr:
			inodeArr[inode_num] = 'allocated'
		elif sep_string[0] == 'INODE':
			sys.stdout.write('ALLOCATED INODE ' + str(inode_num) + ' ON FREELIST\n')
			inodeArr[inode_num] = 'allocated'

	for n in range(non_res_inode, total_inodes + 1):
		if n not in inodeArr:
			sys.stdout.write('UNALLOCATED INODE ' + str(n) + ' NOT ON FREELIST\n')
			
	sys_sum.seek(0)
	
	#Directory Consistency Audits
	inode_reported_link_count = {}
	inode_actual_link_count = {}
	allocInodeArr = []
	for inode in inodeArr:
		if inodeArr[inode] != 'free':
			allocInodeArr.append(inode)
	for inode in allocInodeArr:
		del inodeArr[inode]

	for line in sys_sum:
		sep_string = line.split(',')
		if sep_string[0] == 'INODE':
			inode_num = int(sep_string[1])
			inode_reported_link_count[inode_num] = int(sep_string[6])
			inode_actual_link_count[inode_num] = 0

	sys_sum.seek(0)

	parentArr = {}
	for line in sys_sum:
		sep_string = line.split(',')
		if sep_string[0] == 'DIRENT':
			ref_file_inode = int(sep_string[3])
			parent_inode_num = int(sep_string[1])
			dir_name = sep_string[6].split('\n')[0]

			if ref_file_inode in inodeArr:
				sys.stdout.write('DIRECTORY INODE ' + str(parent_inode_num)+ ' NAME ' + dir_name + ' UNALLOCATED INODE ' + str(ref_file_inode) + '\n')
			elif ref_file_inode not in inodeArr and ref_file_inode not in inode_reported_link_count:
				sys.stdout.write('DIRECTORY INODE ' + str(parent_inode_num) + ' NAME ' + dir_name + ' INVALID INODE ' + str(ref_file_inode) + '\n')
			else:
				inode_actual_link_count[ref_file_inode] += 1
				if ref_file_inode not in parentArr:
					parentArr[ref_file_inode] = parent_inode_num

		if sep_string[0] == 'DIRENT':
			ref_file_inode = int(sep_string[3])
			parent_inode_num = int(sep_string[1])
			dir_name = sep_string[6].split('\n')[0]
			if (dir_name == "'.'" and (ref_file_inode != parent_inode_num)):
				sys.stdout.write('DIRECTORY INODE ' + str(parent_inode_num) + ' NAME ' + dir_name + ' LINK TO INODE ' + str(ref_file_inode) + ' SHOULD BE ' + str(parent_inode_num) + '\n')
			elif (dir_name == "'..'"):
				grandparentNumber = parentArr[parent_inode_num]
				if(ref_file_inode != grandparentNumber):
					sys.stdout.write('DIRECTORY INODE ' + str(parent_inode_num) + ' NAME ' + dir_name + ' LINK TO INODE ' + str(ref_file_inode) + ' SHOULD BE ' + str(grandparentNumber) + '\n')

	for inode_num in inode_reported_link_count:
		if inode_actual_link_count[inode_num] != inode_reported_link_count[inode_num]:
			sys.stdout.write('INODE ' + str(inode_num) + ' HAS ' + str(inode_actual_link_count[inode_num]) + ' LINKS BUT LINKCOUNT IS ' + str(inode_reported_link_count[inode_num]) + '\n')

	sys_sum.seek(0)

def main():
	if len(sys.argv) != 2:
		sys.stderr.write('Error! Need a single file system summary argument!\n')
		exit(1)
		
	try:
		sys_sum = open(sys.argv[1])
		FileSystem_Audits(sys_sum)
		sys_sum.close()
		
	except IOError as e:
		sys.stderr.write('Error! File open failure.\n')
		exit(1)  
		
if __name__ == '__main__': main()