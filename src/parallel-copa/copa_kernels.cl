#if defined(_WIN32) || defined(WIN32)
#define PACKED __declspec(align(8))
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#define PACKED __attribute__((aligned(8)))
#else 
#error Unknown OS
#endif

#if defined(_WIN32) || defined(WIN32)
#define CACHELINE_ALIGNED __declspec(align(64))
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#define CACHELINE_ALIGNED __attribute__((aligned(64)))
#else 
#error Unknown OS
#endif

typedef struct PACKED Triple
{
	long set;
	int w;
	int p;
} Triple;

typedef struct PACKED Pair
{
	int a_idx;
	int b_idx;
} Pair;

typedef struct CACHELINE_ALIGNED AlignedInt {
	int val;
} AlignedInt;

__kernel void add_triple(
	__global Triple* tmp  /* A */,
	__global Triple* tmp2 /* A + A'*/,
	__global Triple* new_item)
{
	size_t id = get_global_id(0);
	tmp2[id].w = tmp[id].w + new_item->w;
	tmp2[id].p = tmp[id].p + new_item->p;
	tmp2[id].set = tmp[id].set + new_item->set;
}

__kernel void merge_array_sequential_a(
	__global Triple* out /* A + A'*/,
	__global Triple* in1, /* A */
	__global Triple* in2, /* A'*/
	const int size)
{
	int m = 0, n = 0, i = 0;
	while (m < size && n < size) {
		if (in1[m].w <= in2[n].w) {
			out[i++] = in1[m++];
		}
		else {
			out[i++] = in2[n++];
		}
	}
	while (m < size) {
		out[i++] = in1[m++];
	}
	while (n < size) {
		out[i++] = in2[n++];
	}
}

__kernel void merge_array_parallel_a(
	__global Triple* out,
	__global Triple* a,
	__global Triple* b,
	const int local_size)
{
	const int size = local_size << 1;
	const int group_id = get_global_id(0);
	out = &out[size * group_id];
	a = &a[local_size * group_id];
	b = &b[local_size * group_id];
	int m = 0, n = 0, i = 0;
	for (; i < size && m < local_size && n < local_size; ++i) {
		if (a[m].w <= b[n].w) {
			out[i] = a[m++];
		}
		else {
			out[i] = b[n++];
		}
	}
	for (; m < local_size; ++m, ++i) {
		out[i] = a[m];
	}

	for (; n < local_size; ++n, ++i) {
		out[i] = b[n];
	}
}

__kernel void merge_array_sequential_b(
	__global Triple* out /* A + A'*/,
	__global Triple* in1, /* A */
	__global Triple* in2, /* A'*/
	const int size)
{
	int m = 0, n = 0, i = 0;
	while (m < size && n < size) {
		if (in1[m].w >= in2[n].w) {
			out[i++] = in1[m++];
		}
		else {
			out[i++] = in2[n++];
		}
	}
	while (m < size) {
		out[i++] = in1[m++];
	}
	while (n < size) {
		out[i++] = in2[n++];
	}
}

__kernel void merge_array_parallel_b(
	__global Triple* out,
	__global Triple* a,
	__global Triple* b,
	const int local_size)
{
	const int size = local_size << 1;
	const int group_id = get_global_id(0);
	out = &out[size * group_id];
	a = &a[local_size * group_id];
	b = &b[local_size * group_id];
	int m = 0, n = 0, i = 0;
	for (; i < size && m < local_size && n < local_size; ++i) {
		if (a[m].w >= b[n].w) {
			out[i] = a[m++];
		}
		else {
			out[i] = b[n++];
		}
	}
	for (; m < local_size; ++m, ++i) {
		out[i] = a[m];
	}

	for (; n < local_size; ++n, ++i) {
		out[i] = b[n];
	}
}

__kernel void first_max_scan(
	__global Triple* a,
	__global AlignedInt* values,
	__global AlignedInt* values_idx)
{
	//const int local_size = get_local_size(0);
	const int group_id = get_group_id(0);
	const int global_id = get_global_id(0);
	/*for (int i = 1; i < local_size; ++i) {
		if (max_profit < a[i].p) {
			max_profit = a[i].p;
			max_idx = i;
		}
	}
	values[group_id] = max_profit;
	values_idx[group_id] = local_size * group_id+max_idx;*/
	atomic_max((volatile __global int*) & values[group_id].val, a[global_id].p);
}

void print_log(int group, __constant char* str, int data) {
	printf("[%d] %s: %ld\n", group, str, data);
}

__kernel void prune(
	__global Triple* A,
	__global Triple* B,
	__global Pair* out,
	__global AlignedInt* AMax,
	__global AlignedInt* BMax,
	__global AlignedInt* AMax_idx,
	__global AlignedInt* BMax_idx,
	__global AlignedInt* max_val_out,
	__global AlignedInt* max_val_set_out,
	const int A_local_size,
	const int B_local_size,
	const int c,
	const int k)
{
	int max_value_i = 0, max_value_b_idx = -1;
	const int i = get_group_id(0);
	A = &A[A_local_size * i];
	out = &out[2 * i];
	int out_idx = 0;
	for (int j = i, end = k + i; j < end; ++j)
	{
		const int jmodk = (j % k);
		__global Triple* thisB = &B[jmodk * B_local_size];
		const int Z = A[0].w + thisB[B_local_size - 1].w;
		const int Y = A[A_local_size - 1].w + thisB[0].w;
		if (Y <= c)
		{
			if ((AMax[i].val + BMax[jmodk].val) > max_value_i)
			{
				max_value_i = AMax[i].val + BMax[jmodk].val;
				max_value_b_idx = jmodk;
			}
		}
		else if ((Z <= c) && (Y > c))
		{
			// choose
			out[out_idx].a_idx = i * A_local_size;
			out[out_idx++].b_idx = jmodk * B_local_size;
		}
	}
	max_val_out[i].val = max_value_i;
	/*if (i == 1) print_log(i, "max_value_b_idx: ", max_value_b_idx);
	if (i == 1) print_log(i, "BMax_idx[max_value_b_idx]: ", BMax_idx[max_value_b_idx]);
	if (i == 1) print_log(i, "BMax_idx[max_value_b_idx].set: ", B[BMax_idx[max_value_b_idx]].set);*/
	if (max_value_b_idx >= 0)
	{
		max_val_set_out[i].val =
			A[AMax_idx[i].val].set +
			B[BMax_idx[max_value_b_idx].val].set;
	}

}

__kernel void second_max_scan(
	__global Triple* A,
	__global Triple* B,
	__global Pair* remained_pairs,
	__global int* max_out,
	const int A_local_size,
	const int B_local_size)
{
	const int i = get_group_id(0);
	__global Pair* this_remained_pairs = &remained_pairs[i * 2];

	for (int t = 0; (t < 2) && (this_remained_pairs[t].a_idx >= 0l); ++t) {
		__global Triple* this_B = &B[this_remained_pairs[t].b_idx];
		__global int* this_max_out = &max_out[2 * B_local_size * i + t * B_local_size];
		this_max_out[B_local_size - 1] = this_B[B_local_size - 1].p;
		for (int j = B_local_size - 2; j >= 0l; --j)
		{
			this_max_out[j] = (this_B[j].p > this_max_out[j + 1]) ?
				this_B[j].p : this_max_out[j + 1];
		}
	}
}

__kernel void final_search(
	__global Triple* A,
	__global Triple* B,
	__global Pair* remained_pairs,
	__global int* max,
	__global int* max_val_out,
	__global Pair* max_val_pair_out,
	const int A_local_size,
	const int B_local_size,
	const int c,
	const int k)
{
	int t = 0, maxvalue = 0;
	Pair X; X.a_idx = 0; X.b_idx = 0;
	const int i = get_group_id(0);
	__global Pair* this_remained_pairs = &remained_pairs[i * 2];

	for (int t = 0; (t < 2) && (this_remained_pairs[t].a_idx >= 0l); ++t) {
		__global Triple* this_A = &A[this_remained_pairs[t].a_idx];
		__global Triple* this_B = &B[this_remained_pairs[t].b_idx];
		__global int* this_max = &max[2 * B_local_size * i + t * B_local_size];
		int x = 0, y = 0;
		while (x < A_local_size && y < B_local_size)
		{
			if ((this_A[x].w + this_B[y].w) > c) {
				y++;
				continue;
			}
			if ((this_A[x].p + this_max[y]) > maxvalue)
			{
				maxvalue = this_A[x].p + this_max[y];
				X.a_idx = this_A[x].set;
				X.b_idx = this_B[x].set;
			}
			x++;
		}
	}
	max_val_out[i] = maxvalue;
	max_val_pair_out[i] = X;
}