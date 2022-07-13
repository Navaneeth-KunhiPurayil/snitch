from generate_data import *

if __name__=='__main__':
	f = open('data-spv-spv.h','w')
	max_val = 4
	vec_len = 64
	sp = 0.9
	B = int(np.floor(np.log2(sp/(1-sp)))+1)
	if B<1:
		B=1

	ds, a, vec_sparse_a = gen_sparse_vec(vec_len, sp, max_val)
	(size, b_coo_a, _, _, nz_a, idx_list_a) = coo(vec_sparse_a, True) #(size, nz_a, idx_list_a) = coo(vec_sparse_a)
	(_, _, _, _, nz_rle_a, rle_a) = rle(vec_sparse_a, B)  #(_, nz_rle_a, rle_a) = rle(vec_sparse_a, B)
	(_, _, bitmap_a) = bitmap(vec_sparse_a)               #(_, _, bitmap_a) = bitmap(vec_sparse_a)

	ds, b, vec_sparse_b = gen_sparse_vec(vec_len, sp, max_val)
	(size, b_coo_b, _, _, nz_b, idx_list_b) = coo(vec_sparse_b, True) #(size, nz_b, idx_list_b) = coo(vec_sparse_b)
	(_, _, _, _, nz_rle_b, rle_b) = rle(vec_sparse_b, B)  #(_, nz_rle_b, rle_b) = rle(vec_sparse_b, B)
	(_, _, bitmap_b) = bitmap(vec_sparse_b)               #(_, _, bitmap_b) = bitmap(vec_sparse_b)

	write_to_file(f,"nz_a",nz_a)
	packed_write_to_file(f,"coo_a",idx_list_a, b_coo_a )
	write_to_file(f,"nz_rle_a",nz_rle_a)
	packed_write_to_file(f,"rle_a",rle_a, B)
	packed_write_to_file(f,"bitmap_a",bitmap_a, 1)
	write_to_file(f,"a",a)

	write_to_file(f,"nz_b",nz_b)
	packed_write_to_file(f,"coo_b",idx_list_b, b_coo_b)
	write_to_file(f,"nz_rle_b",nz_rle_b)
	packed_write_to_file(f,"rle_b",rle_b, B)
	packed_write_to_file(f,"bitmap_b",bitmap_b, 1)
	write_to_file(f,"b",b)

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
