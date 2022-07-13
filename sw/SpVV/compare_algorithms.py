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
def coo(vec, round_to_power2 = False):
    nz=[]
    idx_list=[]
    for i in range(0, len(vec)):
        if(vec[i]!=0):
            idx_list.append(i)
            nz.append(vec[i])  
    b_coo = int(np.ceil(np.log2(len(vec))))   
    if (round_to_power2):
        b_coo = int(2**np.ceil(np.log2(b_coo)))
            # bits per index
    n_coo = int(np.floor(32/b_coo))             # indices per 32 bits
    len_coo = int(np.ceil(len(nz)/n_coo))       # Total no. of words for indices
    size = (len(nz) + len_coo)*4                # size in bytes
    return (size, b_coo, n_coo, len_coo, nz, idx_list)

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
    n_rle = int(np.floor(32/B))               # no. of run values in 32 bits
    len_rle = int(np.ceil(len(nz)/n_rle))     # Total no. of words for all run values
    size = (len(nz) + len_rle)*4              # size in bytes
    return (size, B, n_rle, len_rle, nz, runlist)

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

# Kernel execution time calculation
def get_t_coo(N_nz, n_coo):
        return int(N_nz*(16*(n_coo-1) + 17)/n_coo)

def get_t_rle(N_nz, n_rle):
    return int(N_nz*(18*(n_rle-1) + 22)/n_rle)

def get_t_dense(N):
    return N * 14

def get_t_bitmap(N, sp):
    return int( N*6 + N*(1-sp)*13 )

def get_data_energy(s):
    return s * 8 * 30 # s in Bytes , 30pJ/bit

if __name__=='__main__':
    energy_per_bit = 30 # pJ/bit
    energy_per_inst = 100 # pJ/inst
    
    x_list =[]
    y_list =[]

    coo_size = []
    rle_size = []
    bitmap_size = []
    dense_size = []

    energy_coo = []
    energy_rle = []
    energy_bitmap = []
    energy_dense = []

    vec_len_list = [64, 256, 512, 1024, 2048]
    #vec_len_list = np.arange(64,1024,)
    #vec_len_list = [512]

    #sp_list = np.arange(0.01,1.0,0.01)
    sp_list = [0.1,0.3,0.5,0.7,0.8,0.9,0.99]
    #sp_list = [0.8]
    
    for sp in sp_list:
      for vec_len in vec_len_list:
        ds,_, vec_sparse = gen_sparse_vec(vec_len, sp)
        dense_size.append(ds)
        
        # coo
        cs,b_coo,n_coo,len_coo, nz_list_coo, idx_list_coo= coo(vec_sparse)
        coo_size.append(cs)
        
        # rle
        B = np.floor(np.log2(sp/(1-sp)))+1
        if B<1:
            B=1
        rs, b_rle, n_rle, len_rle, nz_list_rle, run_list_rle= rle(vec_sparse,B)
        rle_size.append(rs)
        
        # bitmap
        bs,_,_ = bitmap(vec_sparse)
        bitmap_size.append(bs)

        # Dense Data movement energy
        b_energy = ds * 8 * energy_per_bit
        a_energy = ds * 8 * energy_per_bit

        # Compressed data movement energy
        a_energy_coo = cs * 8 * energy_per_bit
        a_energy_rle = rs * 8 * energy_per_bit
        a_energy_bitmap = bs * 8 * energy_per_bit

        # total data movement energy
        data_energy_dense = a_energy + b_energy 
        data_energy_coo = a_energy_coo + b_energy
        data_energy_rle = a_energy_rle + b_energy
        data_energy_bitmap = a_energy_bitmap + b_energy

        # Cycles for each algorithm
        t_dense = get_t_dense(vec_len)
        t_coo = get_t_coo(len(nz_list_coo), n_coo)
        t_rle = get_t_rle(len(nz_list_rle), n_rle)
        t_bitmap = get_t_bitmap(vec_len, sp)
        
        # kernel runtime energy
        kernel_energy_dense = t_dense * energy_per_inst
        kernel_energy_coo = t_coo * energy_per_inst
        kernel_energy_rle = t_rle * energy_per_inst
        kernel_energy_bitmap = t_dense * energy_per_inst

        # total energy 
        tot_energy_dense = data_energy_dense + kernel_energy_dense
        tot_energy_coo = kernel_energy_coo + data_energy_coo
        tot_energy_rle = kernel_energy_rle + data_energy_rle
        tot_energy_bitmap = kernel_energy_bitmap + data_energy_bitmap
 
        #print("N:",vec_len," sp:",sp," coolen:",len(nz_list_coo)," rlelen:",len(nz_list_rle), " ncoo:",n_coo," n_rle:",n_rle,
        #    " len_coo:", len_coo, " len_rle:", len_rle)
        print(vec_len, sp, 
            ds, cs, rs, bs, 
            data_energy_dense, data_energy_coo, data_energy_rle, data_energy_bitmap, 
            t_dense, t_coo, t_rle, t_bitmap,  
            kernel_energy_dense, kernel_energy_coo, kernel_energy_rle, kernel_energy_bitmap,
            tot_energy_dense, tot_energy_coo, tot_energy_rle, tot_energy_bitmap)

        energy_dense.append(tot_energy_dense)
        energy_coo.append(tot_energy_coo)
        energy_rle.append(tot_energy_rle)
        energy_bitmap.append(tot_energy_bitmap)
        x_list.append(vec_len)
        y_list.append(sp)
    
    #X, Y = np.meshgrid(x_list, y_list)
    fig = plt.figure()
    ax = plt.axes(projection='3d')
    #ax.bar3d(x_list, y_list, energy_coo, 1,1,1)
    plot1 = ax.plot_trisurf(x_list, y_list, energy_dense, label='Dense-Baseline')
    plot2 = ax.plot_trisurf(x_list, y_list, energy_coo,label='COO')
    plot3 = ax.plot_trisurf(x_list, y_list, energy_rle, label='RLE')
    plot4 = ax.plot_trisurf(x_list, y_list, energy_bitmap, label='BITMAP')
    #plt.legend(('dense','coo'))
    #ax.plot3D(x_list, y_list, energy_coo)
    #ax.legend([plot1,plot2,plot3,plot4],['Dense','COO','RLE','BITMAP'])
    plt.show()
    #plt.plot(vec_len_list, energy_dense[])


    
