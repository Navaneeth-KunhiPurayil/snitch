from compare_algorithms import *

def write_to_file(f,name,arr):
	#global f
	f.write('int32_t volatile ')
	f.write(name)
	f.write('[]={')
	for i in range(0,len(arr)):
		f.write(str(arr[i]))
		f.write(',\n')	
	f.write('}; \n')

def packed_write_to_file(f, name, arr, b):
	#global f
	
	factor = int(2**b)
	n = int(np.floor(32/b))
	
	val = 0 
	f.write('int32_t volatile ')
	f.write(name)
	f.write('[]={')
	count = 0
	for i in range(0,len(arr)):
		#print("i:",i," val:", arr[i])
		val = val + factor**(i%n) * arr[i];
		if (i%n==(n-1) and i!=0):
			f.write(str(val))
			f.write(',\n')
			val = 0
			count = count + 1
	if val!=0:
		f.write(str(val))
		f.write(',\n')
		count = count + 1
	f.write('};\n')

	f.write('\nuint32_t len_') 
	f.write(name) 
	f.write(' =')
	f.write(str(count))
	f.write(';\n')

	f.write('\nuint32_t mask_') 
	f.write(name) 
	f.write(' =')
	f.write(str(factor-1))
	f.write(';\n')

	f.write('\nuint32_t b_') 
	f.write(name) 
	f.write(' =')
	f.write(str(b))
	f.write(';\n')

	f.write('\nuint32_t n_') 
	f.write(name) 
	f.write(' =')
	f.write(str(n))
	f.write(';\n')

if __name__=='__main__':
	f = open('data-spvv.h','w')
	max_val = 16
	vec_len = 2048
	sp = 0.99
	B = int(np.floor(np.log2(sp/(1-sp)))+1)
	if B<1:
		B=1
	#print(B)
	ds, a, vec_sparse = gen_sparse_vec(vec_len, sp, max_val)

	(size, _, _, _, nz, idx_list) = coo(vec_sparse)
	(_, _, _, _, nz_rle, rle) = rle(vec_sparse, B)
	(_, _, bitmap) = bitmap(vec_sparse)
	'''print(rle)
	rle = [x + 1 for x in rle]
	if(rle[0]!=0):
		rle[0] = rle[0]-1

	print(rle)'''

	write_to_file(f,"nz",nz)
	packed_write_to_file(f,"coo",idx_list, int(np.ceil(np.log2(len(vec_sparse)))) )
	write_to_file(f,"nz_rle",nz_rle)
	packed_write_to_file(f,"rle", rle, B)
	packed_write_to_file(f,"bitmap",bitmap, 1)
	write_to_file(f,"a",a)

	b = np.random.randint(1, max_val,  size=vec_len, dtype=np.int32)
	write_to_file(f,"b",b)

	f.write('\n uint32_t len =')
	f.write(str(vec_len))
	f.write(';')

	f.write('\n uint32_t nz_len =')
	f.write(str(len(nz)))
	f.write(';')

	f.write('\n uint32_t rle_len =')
	f.write(str(len(nz_rle)))
	f.write(';')
