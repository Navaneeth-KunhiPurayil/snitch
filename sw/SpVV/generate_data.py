from compare_algorithms import *

f = open('data-spvv.h','w')

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
	max_val = 16
	vec_len = 10
	sp = 0.1
	B = np.floor(np.log2(sp/(1-sp)))+1
	if B<1:
		B=1
	ds, a, vec_sparse = gen_sparse_vec(vec_len, sp, max_val)

	(size, nz, idx_list) = coo(vec_sparse)
	(_, nz_rle, rle) = rle(vec_sparse, B)
	(_, _, bitmap) = bitmap(vec_sparse)
	#print(rle)
	#rle = [x + 1 for x in rle]
	#if(rle[0]==1):
	#	rle[0] = rle[0]-1

	#print(rle)

	write_to_file("nz",nz)
	write_to_file("coo",idx_list)
	write_to_file("nz_rle",nz_rle)
	write_to_file("rle",rle)
	write_to_file("bitmap",bitmap)
	write_to_file("a",a)

	b = np.random.randint(1, max_val,  size=vec_len, dtype=np.int32)
	write_to_file("b",b)

	f.write('\n uint32_t len =')
	f.write(str(vec_len))
	f.write(';')

	f.write('\n uint32_t nz_len =')
	f.write(str(len(nz)))
	f.write(';')

	f.write('\n uint32_t rle_len =')
	f.write(str(len(nz_rle)))
	f.write(';')
