import numpy as np
import random
#from scipy import sparse
import matplotlib.pyplot as plt

# To generate sparse vectors
def gen_sparse_vec(vec_size, sparsity, max_val = 127):
    vec = np.random.randint(1, max_val, size=vec_size, dtype=np.int32) # each vector is 32 bits = 4 B
    dense_size = vec.size * vec.itemsize
    indices = random.sample(range(vec_size), int(vec_size*sparsity)) # To get unique indices to be zero
    vec_sparse = vec
    vec_sparse[indices] = 0
    return (dense_size, vec, vec_sparse)

# Sparse encoding algorithms

# Coordinate encoding
def coo(vec):
    nz=[]
    idx_list=[]
    for i in range(0, len(vec)):
        if(vec[i]!=0):
            idx_list.append(i)
            nz.append(vec[i])    
    size = len(nz)*(32 + np.ceil(np.log2(len(vec))))/8
    return (size, nz, idx_list)

# run length encoding
def rle(vec, B):
    nz=[]
    runlist=[]
    run = 0
    for i in range(0,len(vec)):
        if vec[i]==0:
            run=run+1
            if (run==2**B):
                runlist.append(int(2**B-1))
                nz.append(0)
                run=0
        else :
            runlist.append(run)
            nz.append(vec[i])
            run=0
    #print(nz, runlist)
    size = (len(nz)*(32 + B))/8.0
    return (size, nz, runlist)

# Bitmap encoding
def bitmap(vec):
    nz=[]
    bitmap=[]
    for i in range(0,len(vec)):
        if vec[i]==0:
            bitmap.append(0)
        else:
            bitmap.append(1)
            nz.append(vec[i])
    #print(bitmap, nz, len(bitmap))
    size = (len(bitmap) + 32*len(nz))/8.0
    return (size, nz, bitmap)

if __name__=='__main__':
    vec_len = 10000
    coo_size = []
    rle_size = []
    bitmap_size = []
    dense_size = []

    sp_list = np.arange(0.01,1.0,0.01)
    #sp_list = [0.1,0.3,0.5,0.7,0.9,0.99]

    for sp in sp_list:
        ds,_, vec_sparse = gen_sparse_vec(vec_len, sp)
        dense_size.append(ds)
        
        # coo
        ss,_,_ = coo(vec_sparse)
        coo_size.append(ss)
        
        # rle
        B = np.floor(np.log2(sp/(1-sp)))+1
        if B<1:
            B=1
        rs,_,_= rle(vec_sparse,B)
        rle_size.append(rs)
        
        # bitmap
        bs,_,_ = bitmap(vec_sparse)
        bitmap_size.append(bs)

    dense_size = np.array(dense_size)
    coo_size = np.array(coo_size)
    rle_size = np.array(rle_size)
    bitmap_size = np.array(bitmap_size)

    plt.plot(sp_list, coo_size/dense_size)
    plt.plot(sp_list, rle_size/dense_size)
    plt.plot(sp_list, bitmap_size/dense_size)
    plt.legend(('coo','rle','bitmap'))
    plt.show()