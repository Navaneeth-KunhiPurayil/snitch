from compare_algorithms import *

f = open('data-spv-spv.h','w')

def write_to_file(name,arr):
	global f
	f.write('int32_t volatile ')
	f.write(name)
	f.write('[]={')
	for i in range(0,len(arr)):
		f.write(str(arr[i]))
		f.write(',\n')	
	f.write('}; \n')

if __name__=='__main__':
	max_val = 4
	vec_len = 500
	sp = 0.8
	B = np.floor(np.log2(sp/(1-sp)))+1
	if B<1:
		B=1
	ds, a, vec_sparse_a = gen_sparse_vec(vec_len, sp, max_val)
	(size, nz_a, idx_list_a) = coo(vec_sparse_a)
	(_, nz_rle_a, rle_a) = rle(vec_sparse_a, B)
	(_, _, bitmap_a) = bitmap(vec_sparse_a)

	ds, b, vec_sparse_b = gen_sparse_vec(vec_len, sp, max_val)
	(size, nz_b, idx_list_b) = coo(vec_sparse_b)
	(_, nz_rle_b, rle_b) = rle(vec_sparse_b, B)
	(_, _, bitmap_b) = bitmap(vec_sparse_b)

	write_to_file("nz_a",nz_a)
	write_to_file("coo_a",idx_list_a)
	write_to_file("nz_rle_a",nz_rle_a)
	write_to_file("rle_a",rle_a)
	write_to_file("bitmap_a",bitmap_a)
	write_to_file("a",a)

	write_to_file("nz_b",nz_b)
	write_to_file("coo_b",idx_list_b)
	write_to_file("nz_rle_b",nz_rle_b)
	write_to_file("rle_b",rle_b)
	write_to_file("bitmap_b",bitmap_b)
	write_to_file("b",b)

	f.write('\n uint32_t len =')
	f.write(str(vec_len))
	f.write(';')

	f.write('\n uint32_t nz_len =')
	f.write(str(len(nz_a)))
	f.write(';')

	f.write('\n uint32_t rle_len_a =')
	f.write(str(len(nz_rle_a)))
	f.write(';')

	f.write('\n uint32_t rle_len_b =')
	f.write(str(len(nz_rle_b)))
	f.write(';')
