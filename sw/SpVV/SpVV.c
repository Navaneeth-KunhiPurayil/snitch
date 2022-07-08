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

	ptr_coo = ptr; 
	assign_memory(ptr_coo, coo, nz_len);
	ptr += nz_len + 1;

	ptr_nz_rle = ptr; 
	assign_memory(ptr_nz_rle, nz_rle, rle_len);
	ptr += rle_len + 1;

	ptr_rle = ptr; 
	assign_memory(ptr_rle, rle, rle_len);
	ptr += rle_len + 1;

	ptr_bitmap = ptr; 
	assign_memory(ptr_bitmap, bitmap, len);
	ptr += len + 1;
}

int32_t spVV_baseline() {
    
    int32_t c = 0;
    for (uint32_t i=0; i<len; i++)
    {
    	c += ptr_a[i]*ptr_b[i];
    }
    return c;
}

int32_t spVV_coordinate() {
    
    int32_t c=0;
    
    //int32_t *idx_ptr = ptr_coo;
    //int32_t *nz_ptr = ptr_nz;
    //int32_t *b_ptr = ptr_b; 
    for (uint32_t i=0; i< nz_len; i++) {
    	//c += *nz_ptr * *(ptr_b + *idx_ptr);
    	//idx_ptr ++;
        //nz_ptr ++;
        
        //c += ptr_nz[i] * ptr_b[*idx_ptr]; 
        //idx_ptr++;
        
        c += ptr_nz[i]*ptr_b[ptr_coo[i]];
    }
    return c;
}

int32_t spVV_rle() {
    int32_t c = 0;
    uint32_t idx = -1;
    for (uint32_t i=0; i< rle_len; i++) {
        idx += ptr_rle[i] + 1;
        c += ptr_nz_rle[i] * ptr_b[idx];
    }
    return c;
}

int32_t spVV_bitmap() {
    int32_t c = 0;
    uint32_t k = 0; 
    for (uint32_t i=0; i<len; i++) {
        if (ptr_bitmap[i]) {
            c += ptr_nz[k] * ptr_b[i];
            k++;
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
