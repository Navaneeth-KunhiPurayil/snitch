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
    int32_t c=0;

    uint32_t idx_a=0,idx_b=1;
    uint32_t count_a=0, count_b=0, j_a = 0, j_b = 0;
    int32_t  coo_val_a = *ptr_coo_a;
    int32_t  coo_val_b = *ptr_coo_b;   
    uint32_t index_a = 0, index_b = 0;

    
    /*while( (index_a < 4*nz_len) && (index_b < 4*nz_len)) {
    	idx_a = coo_val_a & mask_coo_a; 
    	idx_b = coo_val_b & mask_coo_b;
    	while ((idx_a != idx_b) && (index_a < 4*nz_len) && (index_b < 4*nz_len)) {
    	    idx_a = coo_val_a & mask_coo_a; 
    	    idx_b = coo_val_b & mask_coo_b;
    	    if (idx_a < idx_b) {
          	    index_a += 4;
        	    if (count_a == (n_coo_a - 1)) {
        		    j_a += 4;
        		    coo_val_a = ptr_coo_a[j_a/4];
        		    count_a = 0;
        	    } else {
        	        coo_val_a = coo_val_a >> b_coo_a;
        	        count_a++;
        	    }

            } else {
          	    index_b += 4;
        	    if (count_b == (n_coo_b - 1)) {
        		    j_b += 4;
        		    coo_val_b = ptr_coo_b[j_b/4];
        		    count_b = 0;
        	    } else {
        	        coo_val_b = coo_val_b >> b_coo_b;
        	        count_b++;
        	    }
            }
        }
        //printf("idx_a:%d idx_b:%d idx_nz_a:%d idx_nz_b:%d\n", idx_a, idx_b, index_a/4, index_b/4);
        printf("idx_a:%d idx_b:%d idx_nz_a:%d idx_nz_b:%d a:%d b:%d c:%d\n", idx_a, idx_b, index_a/4, index_b/4, ptr_nz_a[index_a/4], ptr_nz_b[index_b/4], c);
        c += ptr_nz_a[index_a/4] * ptr_nz_b[index_b/4];
        index_a += 4; 
        index_b += 4;
        idx_b--;
    }*/
    /*
    while( (index_a < 4*nz_len) && (index_b < 4*nz_len)) {
    	idx_a = coo_val_a & mask_coo_a; 
    	idx_b = coo_val_b & mask_coo_b;
    	printf("idx_a_new:%d idx_new_b:%d\n", idx_a, idx_b);
    	do {
    	while ((idx_a < idx_b) && (index_a < 4*nz_len) ) {
    	        idx_a = coo_val_a & mask_coo_a; 
          	    index_a += 4;
        	    if (count_a == (n_coo_a - 1)) {
        		    j_a += 4;
        		    coo_val_a = ptr_coo_a[j_a/4];
        		    count_a = 0;
        	    } else {
        	        // Go to next idx a
        	        coo_val_a = coo_val_a >> b_coo_a;
        	        count_a++;
        	    }
        	    
        }
        while ((idx_a > idx_b) && (index_b < 4*nz_len) ) {
        	    idx_b = coo_val_b & mask_coo_b;
          	    index_b += 4;
        	    if (count_b == (n_coo_b - 1)) {
        		    j_b += 4;
        		    coo_val_b = ptr_coo_b[j_b/4];
        		    count_b = 0;
        	    } else {
        	        // Go to next idx a
        	        coo_val_b = coo_val_b >> b_coo_b;
        	        count_b++;
        	    }
        	    
        }
    }
        while( (idx_a != idx_b) && (index_a < 4*nz_len) && (index_b < 4*nz_len));

        //printf("idx_a:%d idx_b:%d idx_nz_a:%d idx_nz_b:%d\n", idx_a, idx_b, index_a/4, index_b/4);
        printf("idx_a:%d idx_b:%d idx_nz_a:%d idx_nz_b:%d a:%d b:%d c:%d\n", idx_a, idx_b, index_a/4, index_b/4, ptr_nz_a[index_a/4], ptr_nz_b[index_b/4], c);
        c += ptr_nz_a[index_a/4] * ptr_nz_b[index_b/4];
        index_a += 4; 
        index_b += 4;
        idx_b--;
    }*/

    
    idx_a = coo_val_a & mask_coo_a; 
    idx_b = coo_val_b & mask_coo_b;

    while( (index_a < 4*nz_len) && (index_b < 4*nz_len)) {
    	    if (idx_a < idx_b) {
        	    index_a +=4; 
        	    if (index_a % (n_coo_a*4) == 0) {
        	    	coo_val_a = ptr_coo_a[index_a/(n_coo_a*4)];
        	    } else {
        	    	coo_val_a = coo_val_a >> b_coo_a;
        	    }
        	    idx_a = coo_val_a & mask_coo_a; 

            } else if (idx_a > idx_b) {
          	    index_b += 4;
        	    if (index_b % (n_coo_b*4) == 0) {
        	    	coo_val_b = ptr_coo_b[index_b/(n_coo_b*4)];
        	    } else {
        	    	coo_val_b = coo_val_b >> b_coo_b;
        	    }
        	    idx_b = coo_val_b & mask_coo_b;
            } else {
            	c += ptr_nz_a[index_a/4] * ptr_nz_b[index_b/4];
            	//printf("idx_a:%d idx_b:%d idx_nz_a:%d idx_nz_b:%d a:%d b:%d c:%d\n", idx_a, idx_b, index_a/4, index_b/4, ptr_nz_a[index_a/4], ptr_nz_b[index_b/4], c);
            	index_a += 4;
            	index_b += 4;
        	    if (index_a % (n_coo_a*4) == 0) {
        	    	coo_val_a = ptr_coo_a[index_a/(n_coo_a*4)];
        	    } else {
        	    	coo_val_a = coo_val_a >> b_coo_a;
        	    }
        	    if (index_b % (n_coo_b*4) == 0) {
        	    	coo_val_b = ptr_coo_b[index_b/(n_coo_b*4)];
        	    } else {
        	    	coo_val_b = coo_val_b >> b_coo_b;
        	    }
        	    idx_a = coo_val_a & mask_coo_a; 
        	    idx_b = coo_val_b & mask_coo_b;
            }     
    }

    return c;

}

int32_t spVspV_bitmap() {
    int32_t c = 0;
    

    uint32_t idx = 0, idx2 = 0; 
    uint32_t i=0, j=0, k=0, bit, bit_a, bit_b;
    int32_t nz_val_a, nz_val_b; 


    uint32_t bitmap_val_a = ptr_bitmap_a[0]; 
    uint32_t bitmap_val_b = ptr_bitmap_b[0]; 
    uint32_t bitmap_val = bitmap_val_a & bitmap_val_b;
    uint32_t bit_mask = 1;

    //while (i < nz_len*4 && j < nz_len*4) {
        do {

        	bit = bitmap_val & bit_mask;
            if (bit){
                c += ptr_nz_a[i/4] * ptr_nz_b[j/4];
                //printf("i:%u j:%u c:%d\n", i/4, j/4, c);
            }

        	if (bitmap_val_a & bit_mask) 
        		i+=4;
        	if (bitmap_val_b & bit_mask)
        		j+=4;

        	bit_mask = bit_mask << 1;

        	if (bit_mask==0) {
        		k+=4; 
        		bitmap_val_a = ptr_bitmap_a[k/4];
        		bitmap_val_b = ptr_bitmap_b[k/4];
        		bitmap_val = bitmap_val_a & bitmap_val_b;
        		bit_mask = 1;     	
                //printf("bita:%x bitb:%x and-bits:%x\n",bitmap_val_a,bitmap_val_b,bitmap_val);
        	}
            //printf("bit:%u bitmask:%u\n",bit, bit_mask);
        }
        while ((i<nz_len*4) && (j < nz_len*4)); 
        
    //}
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
    res_act = spVspV_bitmap();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("bitmap: %u res_act:%u\n",t2-t1,res_act);

    return 0;
}
