#include "SpVV.h"
#include "data-spvv.h"

int32_t *ptr_a, *ptr_b;
int32_t *ptr_nz, *ptr_coo;
int32_t *ptr_nz_rle, *ptr_rle;
int32_t *ptr_bitmap;

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

	ptr_nz = ptr; 
	assign_memory(ptr_nz, nz, nz_len);
	ptr += nz_len + 1;

	ptr_nz_rle = ptr; 
	assign_memory(ptr_nz_rle, nz_rle, rle_len);
	ptr += rle_len + 1;
    
    // Packed encoding 
	ptr_coo = ptr; 
	assign_memory(ptr_coo, coo, len_coo);
	ptr += len_coo + 1;

	ptr_rle = ptr; 
	assign_memory(ptr_rle, rle, len_rle);
	ptr += len_rle + 1;

	ptr_bitmap = ptr; 
	assign_memory(ptr_bitmap, bitmap, len_bitmap);
	ptr += len_bitmap + 1;
}

int32_t spVV_baseline() {
    
    int32_t c = 0;
    for (uint32_t i=0; i<len; i+=1)
    {
    	c += ptr_a[i]*ptr_b[i];
    }
    return c;
}

int32_t spVV_coordinate() {
    
    int32_t c=0;
    uint32_t idx,idx2;
    uint32_t count=0,i=0, j=0;
    int32_t  coo_val = *ptr_coo;
    int32_t nz_val, dense_val; 

    while (i < nz_len*4) {
    	nz_val = ptr_nz[i/4];
        idx = coo_val & mask_coo;
        if (count == (n_coo-1)){
        	j+=4;
        	coo_val = ptr_coo[j/4];
        	count=0;
        } else {
        	count++;
        	coo_val = coo_val >> b_coo;
        }
        //printf("i:%d idx:%d\n",i,idx);
        c += nz_val*ptr_b[idx];
        i+=4; 
    }
    return c;
}

int32_t spVV_rle() {
    int32_t c = 0, rle_val = ptr_rle[0];
    uint32_t idx = -1;
    uint32_t count=0, i=0, j=0, run;
    int32_t nz_val; 
    
    while (i < rle_len*4) {
    	nz_val = ptr_nz_rle[i/4];
        run = rle_val & mask_rle;
        idx += run + 1;
        if (count == (n_rle-1)){
        	j+=4;
        	rle_val = ptr_rle[j/4];
        	count=0;
        } else {
        	count++;
        	rle_val = rle_val >> b_rle;
        }
        
        //printf("i:%d idx:%d\n", i, idx);
        c += nz_val * ptr_b[idx];
        i+=4;
        
    }
    return c;
}

int32_t spVV_bitmap() {
    int32_t c = 0, bitmap_val = ptr_bitmap[0];
    uint32_t idx = 0, idx2 = 0; 
    uint32_t i=0, j=0, bit;
    int32_t nz_val; 

    while (i < nz_len*4) {
    	nz_val = ptr_nz[i/4];
        do {
        	bit = bitmap_val & 1;
        	idx += 4;
        	if ((idx & 124) == 0) {
        		j+=4; 
        		bitmap_val = ptr_bitmap[j/4];
        	} else {
        		bitmap_val = bitmap_val >> 1;
        	}  	 
        	//printf("bit:%d bitmap_val:%d idx:%u count:%u\n",bit, bitmap_val, idx, count);
        } 
        while (!bit);
        
        idx2 = idx/4-1; 
        c += nz_val * ptr_b[idx2];
        i+=4;
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
    res_act = spVV_baseline();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("baseline: %u res_act:%u\n",t2-t1,res_act);
        
    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVV_coordinate();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("coordinate: %u res_act:%u\n",t2-t1,res_act);

    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVV_rle();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("run length encoding: %u res_act:%u\n",t2-t1,res_act);
       
    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVV_bitmap();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("bitmap:%u res_act:%u\n",t2-t1,res_act);

    return 0;
}