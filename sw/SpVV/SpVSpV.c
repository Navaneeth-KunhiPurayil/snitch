#include "SpVV.h"
#include "data-spv-spv.h"

int32_t *ptr_a, *ptr_b;
int32_t *ptr_nz_a, *ptr_nz_b, *ptr_coo_a, *ptr_coo_b;
int32_t *ptr_nz_rle_a,*ptr_nz_rle_b, *ptr_rle_a, *ptr_rle_b;
int32_t *ptr_bitmap_a, *ptr_bitmap_b;

size_t benchmark_get_cycle() { return read_csr(mcycle); } 

void assign_memory(int32_t *ptr, int32_t arr[], uint32_t len) {
	for(uint32_t i=0; i < len; i++) {
		*ptr = arr[i];
		ptr++;
	}
}

void setup_memory() {
	int32_t *ptr = snrt_cluster_memory().start;
	ptr_a = ptr; 
	assign_memory(ptr_a, a, len);
	ptr += len + 1;
	
	ptr_b = ptr; 
	assign_memory(ptr_b, b, len); 
	ptr += len + 1;

    // Non-zero values COO/Bitmap
	ptr_nz_a = ptr; 
	assign_memory(ptr_nz_a, nz_a, nz_len);
	ptr += nz_len + 1;

	ptr_nz_b = ptr; 
	assign_memory(ptr_nz_b, nz_b, nz_len);
	ptr += nz_len + 1;

    // Non-zero values RLE
	ptr_nz_rle_a = ptr; 
	assign_memory(ptr_nz_rle_a, nz_rle_a, rle_len_a);
	ptr += rle_len_a + 1;

	ptr_nz_rle_b = ptr; 
	assign_memory(ptr_nz_rle_b, nz_rle_b, rle_len_b);
	ptr += rle_len_b + 1;

    // Packed encoding
	ptr_coo_a = ptr; 
	assign_memory(ptr_coo_a, coo_a, len_coo_a);
	ptr += len_coo_a + 1;

	ptr_coo_b = ptr; 
	assign_memory(ptr_coo_b, coo_b, len_coo_b);
	ptr += len_coo_b + 1;

	ptr_rle_a = ptr; 
	assign_memory(ptr_rle_a, rle_a, len_rle_a);
	ptr += len_rle_a + 1;

	ptr_rle_b = ptr; 
	assign_memory(ptr_rle_b, rle_b, len_rle_b);
	ptr += len_rle_b + 1;

	ptr_bitmap_a = ptr; 
	assign_memory(ptr_bitmap_a, bitmap_a, len_bitmap_a);
	ptr += len_bitmap_a + 1;

	ptr_bitmap_b = ptr; 
	assign_memory(ptr_bitmap_b, bitmap_b, len_bitmap_b);
	ptr += len_bitmap_b + 1;
}

int32_t spVspV_baseline() {
    
    int32_t c = 0;
    for (uint32_t i=0; i<len; i+=1)
    {
    	c += ptr_a[i]*ptr_b[i];
    	//if (ptr_a[i] && ptr_b[i])
    	//	printf("i:%d a:%d b:%d c:%d\n", i, ptr_a[i], ptr_b[i], c);
    }
    return c;
}

int32_t spVspV_coordinate() {
    int32_t c=0, val1, val2;

    uint32_t idx_a=0,idx_b=1;
    uint32_t count_a=0, count_b=0, j_a = 0, j_b = 0;
    int32_t  coo_val_a = *ptr_coo_a;
    int32_t  coo_val_b = *ptr_coo_b;   
    uint32_t index_a = 0, index_b = 0;

    //printf("n_coo:%d\n",n_coo_a);

    idx_a = coo_val_a & mask_coo_a; 
    idx_b = coo_val_b & mask_coo_b;

    uint32_t * ptr_idx_a = ptr_coo_a, *ptr_idx_b = ptr_coo_b;

    while(1) {
        do {
            while(idx_a < idx_b) {
                
                coo_val_a = coo_val_a >> b_coo_a;
                index_a += 4; 
                if (coo_val_a == 0) {
                    coo_val_a = *(++ptr_idx_a);
                    if (index_a >= 4*nz_len)
                        return c;
                }
                idx_a = coo_val_a & mask_coo_a; 
            }
            while (idx_a > idx_b) {
                
                coo_val_b = coo_val_b >> b_coo_b;
                index_b += 4;
                if (coo_val_b == 0) {
                    coo_val_b = *(++ptr_idx_b);
                    if (index_b >= 4*nz_len)
                        return c;
                }
                idx_b = coo_val_b & mask_coo_b;
            } 
        } 
        while ((idx_a != idx_b));


                val1 = ptr_nz_a[index_a/4]; 
                index_a +=4;
                coo_val_a = coo_val_a >> b_coo_a;

                val2 = ptr_nz_b[index_b/4];
                index_b +=4;
                coo_val_b = coo_val_b >> b_coo_b;
                
                c += val1 * val2;

                if (coo_val_a == 0) {
                    coo_val_a = *(++ptr_idx_a);
                    if ((index_a >= nz_len*4) )
                        return c;
                }
                idx_a = coo_val_a & mask_coo_a;
                
                if (coo_val_b == 0) {
                    coo_val_b = *(++ptr_idx_b);
                    if ((index_b >= nz_len*4))
                        return c;
                }
                idx_b = coo_val_b & mask_coo_b;

                //printf("idx_a:%d idx_b:%d idx_nz_a:%d idx_nz_b:%d a:%d b:%d c:%d\n", idx_a, idx_b, index_a/4, index_b/4, ptr_nz_a[index_a/4], ptr_nz_b[index_b/4], c);

                
    }

    return c;

}

int32_t spVspV_rle() {
    int32_t c=0, val1, val2;

    uint32_t run_a = 0, run_b = 0, acc_a = 0, acc_b = 0; 
    //uint32_t count_a=0, count_b=0, j_a = 0, j_b = 0;
    int32_t  rle_val_a = *ptr_rle_a;
    int32_t  rle_val_b = *ptr_rle_b;   
    uint32_t index_a = 0, index_b = 0;

    run_a = rle_val_a & mask_rle_a; 
    run_b = rle_val_b & mask_rle_b;
    acc_a = run_a;
    acc_b = run_b;

    uint32_t * ptr_idx_a = ptr_rle_a, *ptr_idx_b = ptr_rle_b;
    
    while(1) {

        do {
            while (acc_a < acc_b) {
                index_a += 4;
                if (index_a >= 4*rle_len_a)
                    return c;
                if ((index_a % (4*n_rle_a)) == 0) {                   
                    rle_val_a = *(++ptr_idx_a);
                } else {
                    rle_val_a = rle_val_a >> b_rle_a;
                }
                 
                run_a = rle_val_a & mask_rle_a; 
                acc_a += (run_a + 1); 
                //printf("run_a:%u   idx_a:%u idx_b:%u acc_a:%u acc_b:%u\n ", run_a, index_a, index_b, acc_a, acc_b);
            }

            while (acc_a > acc_b) {
                index_b += 4;
                if (index_b >= 4*rle_len_b)
                        return c;
                if ((index_b % (4*n_rle_b)) == 0) {
                    rle_val_b = *(++ptr_idx_b);
                } else {
                    rle_val_b = rle_val_b >> b_rle_b;
                }
                
                run_b = rle_val_b & mask_rle_b; 
                acc_b += (run_b + 1); 
                //printf("run_b:%u idx_a:%u idx_b:%u acc_a:%u acc_b:%u\n ", run_b, index_a, index_b, acc_a, acc_b);

            }
        } while (acc_a != acc_b);

        //printf("idx_a:%u idx_b:%u acc_a:%u acc_b:%u A:%d B:%d c:%d\n ", index_a, index_b, acc_a, acc_b, ptr_nz_rle_a[index_a], ptr_nz_rle_b[index_b], c);

        val1 = ptr_nz_rle_a[index_a/4];
        index_a +=4;       

        val2 = ptr_nz_rle_b[index_b/4];
        index_b +=4;        
        
        c += val1 * val2;

        if (index_a >= 4*rle_len_a)
                return c;
        if (index_b >= 4*rle_len_b)
                return c;

        if ((index_a % (4*n_rle_a)) == 0) {
            rle_val_a = *(++ptr_idx_a);           
        } else {
            rle_val_a = rle_val_a >> b_rle_a;
        }

        run_a = rle_val_a & mask_rle_a; 
        acc_a += (run_a + 1); 

        //printf("run_a:%u   idx_a:%u idx_b:%u acc_a:%u acc_b:%u\n ", run_a, index_a, index_b, acc_a, acc_b);

        if ((index_b % (4*n_rle_b)) == 0) {
            rle_val_b = *(++ptr_idx_b);
        } else {
            rle_val_b = rle_val_b >> b_rle_b;
        }

        run_b = rle_val_b & mask_rle_b; 
        acc_b += (run_b + 1);
        //printf("run_b:%u idx_a:%u idx_b:%u acc_a:%u acc_b:%u\n ", run_b, index_a, index_b, acc_a, acc_b);
    }
    return c;
}


int32_t spVspV_bitmap() {
    int32_t c = 0;
    

    uint32_t idx = 0, idx2 = 0; 
    uint32_t i=0, j=0, k=4, bit, bit_a, bit_b;
    int32_t nz_val_a, nz_val_b; 

    uint32_t bitmap_val_a = ptr_bitmap_a[0]; 
    uint32_t bitmap_val_b = ptr_bitmap_b[0];

    uint32_t bit_mask = 1;

        while(1) {
            if (bitmap_val_a & bit_mask) {
                if (bitmap_val_b & bit_mask) {

                    nz_val_a = ptr_nz_a[i/4];
                    nz_val_b = ptr_nz_b[j/4];
                    c += nz_val_a * nz_val_b;
                    i+=4;
                    j+=4;
                    
                    if (j==nz_len*4) 
                        return c; 
                    if (i==nz_len*4)
                        return c; 

                } else {

                    i+=4;
                    if (i==nz_len*4)
                        return c; 
                }
                
            } else if (bitmap_val_b & bit_mask) {
                j+=4;
                if (j==nz_len*4)
                    return c; 
            }
     		 
        	bit_mask = bit_mask << 1;
            if (bit_mask==0) {
            	bitmap_val_a = ptr_bitmap_a[k/4];
        		bitmap_val_b = ptr_bitmap_b[k/4];
                k+=4;
        		bit_mask = 1;
        	} 
        }
            
    return c;
}

int main()
{
    int32_t res_act;
    size_t t1,t2,d;
    setup_memory();

    t1 = benchmark_get_cycle(); // dummy statement
    
    // Baseline
    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVspV_baseline();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("baseline: %u res_act:%u\n",t2-t1,res_act);
    
    
    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVspV_coordinate();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("coordinate: %u res_act:%u\n",t2-t1,res_act);

    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVspV_rle();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("rle: %u res_act:%u\n",t2-t1,res_act);


    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVspV_bitmap();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("bitmap: %u res_act:%u\n",t2-t1,res_act);

    return 0;
}
