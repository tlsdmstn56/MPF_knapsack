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

////////////////////////////
// Data Structures
////////////////////////////
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

typedef struct CACHELINE_ALIGNED Quad
{
	int startA, endA, startB, endB;
} Quad;

typedef struct CACHELINE_ALIGNED Pair2
{
	int first, second;
	int arr;
} Pair2;


typedef struct CACHELINE_ALIGNED AlignedInt {
	int val;
} AlignedInt;
////////////////////////////
// Helper Functions
////////////////////////////
inline int ceiling_half(int n)
{
	return (n >> 1) + (n & 1);
}

inline int floor_half(int n)
{
	return (n >> 1);
}

//inline int min(int a, int b)
//{
//	return (a < b) ? a : b;
//}
//inline int max(int a, int b)
//{
//	return (a > b) ? a : b;
//}

int sort_search_gt(int low, int high, __global Triple* a, int key)
{
	int i = low, j = high;
	while (i < j)
	{
		int h = (i + j) >> 1;
		if (!(key > a[h].w))
		{
			i = h + 1;
		}
		else
		{
			j = h;
		}
	}
	return i - low;
}

int sort_search_ge(int low, int high, __global  Triple* a, int key)
{
	int i = low, j = high;
	while (i < j)
	{
		int h = (i + j) >> 1;
		if (!(key >= a[h].w))
		{
			i = h + 1;
		}
		else
		{
			j = h;
		}
	}
	return i - low;
}

int sort_search_lt(int low, int high, __global Triple* a, int key)
{
	int i = low, j = high;
	while (i < j)
	{
		int h = (i + j) >> 1;
		if (!(key < a[h].w))
		{
			i = h + 1;
		}
		else
		{
			j = h;
		}
	}
	return i - low;
}

int sort_search_le(int low, int high, __global  Triple* a, int key)
{
	int i = low, j = high;
	while (i < j)
	{
		int h = (i + j) >> 1;
		if (!(key <= a[h].w))
		{
			i = h + 1;
		}
		else
		{
			j = h;
		}
	}
	return i - low;
}

bool is_valid_pair(__global Triple* A, __global Triple* B,
	int x, int y,
	int lowA_, int highA_,
	int lowB_, int highB_, int* arr)
{
	const int r = highA_ - lowA_ + 1;
	const int s = highB_ - lowB_ + 1;
	int a = A[x].w, b = B[y].w;
	int medianIndex = ceiling_half(r + s);
	// check if a is median
	int bCount = sort_search_lt(lowB_, highB_ + 1, B, a) + 1;

	int lessCountA = x + bCount - 1;

	if (lessCountA == medianIndex - 1)
	{
		int index1 = sort_search_le(lowB_, highB_ + 1, B, a) + 1;
		int index2 = bCount - 1;
		// condition 2 for median pair
		if (B[index1].w == b || B[index2].w == b)
		{
			*arr = 'A';
			return true;
		}
	}

	// check if b is median
	int aCount = sort_search_lt(lowA_, highA_ + 1, A, b) + 1;
	int lessCountB = y + aCount - 1;
	if (lessCountB == medianIndex - 1)
	{
		int index1 = sort_search_le(lowA_, highA_ + 1, A, b) + 1;
		int index2 = aCount - 1;
		// condition 2 for median pair
		if (A[index1].w == a || A[index2].w == a)
		{
			*arr = 'B';
			return true;
		}
	}
	return false;
}

bool is_valid_pair_reverse(__global Triple* A, __global  Triple* B,
	int x, int y,
	int lowA_, int highA_,
	int lowB_, int highB_, int* arr)
{
	const int r = highA_ - lowA_ + 1;
	const int s = highB_ - lowB_ + 1;
	int a = A[x].w, b = B[y].w;
	int medianIndex = ceiling_half(r + s);
	// check if a is median
	int bCount = sort_search_gt(lowB_, highB_ + 1, B, a) + 1;

	int lessCountA = x + bCount - 1;

	if (lessCountA == medianIndex - 1)
	{
		int index1 = sort_search_ge(lowB_, highB_ + 1, B, a) + 1;
		int index2 = bCount - 1;

		// condition 2 for median pair
		if (B[index1].w == b || B[index2].w == b)
		{
			*arr = 'A';
			return true;
		}
	}

	// check if b is median
	int aCount = sort_search_gt(lowA_, highA_ + 1, A, b) + 1;
	int lessCountB = y + aCount - 1;
	if (lessCountB == medianIndex - 1)
	{
		int index1 = sort_search_ge(lowA_, highA_ + 1, A, b) + 1;
		int index2 = aCount - 1;
		// condition 2 for median pair
		if (A[index1].w == a || A[index2].w == a)
		{
			*arr = 'B';
			return true;
		}
	}
	return false;
}

Pair2 two_sequence_median(__global Triple* A, __global  Triple* B,
	int lowA_, int highA_,
	int lowB_, int highB_)
{
	int lowA = lowA_, lowB = lowB_, highA = highA_, highB = highB_, nA, nB;
	int u, v, w;
	int r = nA = highA - lowA + 1;
	int s = nB = highB - lowB + 1;

	while (nA > 1 && nB > 1)
	{
		u = lowA + ceiling_half(highA - lowA - 1);
		v = lowB + ceiling_half(highB - lowB - 1);
		w = min(floor_half(nA), floor_half(nB));
		nA -= w;
		nB -= w;
		if (A[u].w >= B[v].w)
		{
			highA -= w;
			lowB += w;
		}
		else
		{
			lowA += w;
			highB -= w;
		}
	}

	int i, j;
	Pair2 ans;
	int arr;
	for (i = max(0, u - 1); i <= u + 1; ++i)
	{
		for (j = max(0, v - 1); j <= v + 1; ++j)
		{
			if (is_valid_pair(A, B, i, j, lowA_, highA_, lowB_, highB_, &arr))
			{
				ans.first = i;
				ans.second = j;
				ans.arr = arr;
			}
		}
	}
	return ans;
}

// function to find the median of two non-decreasing sequences
Pair2  two_sequence_median_reverse(__global Triple* A, __global  Triple* B,
	int lowA_, int highA_,
	int lowB_, int highB_)
{
	int lowA = lowA_, lowB = lowB_, highA = highA_, highB = highB_, nA, nB;
	int u, v, w;
	int r = nA = highA - lowA + 1;
	int s = nB = highB - lowB + 1;

	while (nA > 1 && nB > 1)
	{
		u = lowA + ceiling_half(highA - lowA - 1);
		v = lowB + ceiling_half(highB - lowB - 1);
		w = min(floor_half(nA), floor_half(nB));
		nA -= w;
		nB -= w;
		if (A[u].w <= B[v].w)
		{
			highA -= w;
			lowB += w;
		}
		else
		{
			lowA += w;
			highB -= w;
		}
	}

	Pair2 ans;
	int arr;
	for (int i = max(0, u - 1), end = min(r - 1, u + 1); i <= end; ++i)
	{
		for (int j = max(0, v - 1), end = min(s - 1, v + 1); j <= end; ++j)
		{
			if (is_valid_pair_reverse(A, B, i, j, lowA_, highA_, lowB_, highB_, &arr))
			{
				ans.first = i;
				ans.second = j;
				ans.arr = arr;
			}
		}
	}
	return ans;
}

void sequential_merge(__global Triple* A, __global Triple* B, __global Triple* C, int lowA, int highA, int lowB, int highB, int offsetC)
{
	int i = lowA, j = lowB;
	int k = offsetC;
	while (i <= highA && j <= highB)
	{
		if (A[i].w < B[j].w)
		{
			C[k++] = A[i++];
		}
		else
		{
			C[k++] = B[j++];
		}
	}
	while (i <= highA)
	{
		C[k++] = A[i++];
	}
	while (j <= highB)
	{
		C[k++] = B[j++];
	}
}

void sequential_merge_reverse(__global Triple* A, __global Triple* B, __global Triple* C, int lowA, int highA, int lowB, int highB, int offsetC)
{
	int i = lowA, j = lowB;
	int k = offsetC;
	while (i <= highA && j <= highB)
	{
		if (A[i].w > B[j].w)
		{
			C[k++] = A[i++];
		}
		else
		{
			C[k++] = B[j++];
		}
	}
	while (i <= highA)
	{
		C[k++] = A[i++];
	}
	while (j <= highB)
	{
		C[k++] = B[j++];
	}
}

__kernel void Step1Point2(__global Quad* quads, __global Triple* A, __global Triple* B)
{
	int p1, q1, p2, q2;
	int i = get_global_id(0);
	// Step 1.2.1
	// vector<int> A_, B_;
	// copy(A+quads[i].startA, A+(quads[i].endA+1), back_inserter(A_));
	// copy(B+quads[i].startB, B+(quads[i].endB+1), back_inserter(B_));
	Pair2 medianPair = two_sequence_median(A, B, quads[i].startA, quads[i].endA,
		quads[i].startB, quads[i].endB);
	medianPair.first += quads[i].startA + 1;
	medianPair.second += quads[i].startB + 1;

	// Step 1.2.2
	if (medianPair.arr == 'A')
	{
		p1 = medianPair.first;
		q1 = medianPair.first + 1;

		if (B[medianPair.second].w <= A[medianPair.first].w)
		{
			p2 = medianPair.second;
			q2 = medianPair.second + 1;
		}
		else
		{
			p2 = medianPair.second - 1;
			q2 = medianPair.second;
		}
	}
	else
	{
		p2 = medianPair.second;
		q2 = medianPair.second + 1;

		if (A[medianPair.first].w <= B[medianPair.second].w)
		{
			p1 = medianPair.first;
			q1 = medianPair.first + 1;
		}
		else
		{
			p1 = medianPair.first - 1;
			q1 = medianPair.first;
		}
	}

	// Step 1.2.3
	quads[2 * i + 1].startA = quads[i].startA;
	quads[2 * i + 1].endA = p1;
	quads[2 * i + 1].startB = quads[i].startB;
	quads[2 * i + 1].endB = p2;

	// Step 1.2.4
	quads[2 * i + 2].startA = q1;
	quads[2 * i + 2].endA = quads[i].endA;
	quads[2 * i + 2].startB = q2;
	quads[2 * i + 2].endB = quads[i].endB;
}

__kernel void Step1Point2_reverse(__global Quad* quads, __global Triple* A, __global Triple* B)
{
	const int i = get_global_id(0);
	int p1, q1, p2, q2;

	// Step 1.2.1
	// vector<int> A_, B_;
	// copy(A+quads[i].startA, A+(quads[i].endA+1), back_inserter(A_));
	// copy(B+quads[i].startB, B+(quads[i].endB+1), back_inserter(B_));
	Pair2 medianPair = two_sequence_median_reverse(A, B, quads[i].startA, quads[i].endA,
		quads[i].startB, quads[i].endB);
	medianPair.first += quads[i].startA + 1;
	medianPair.second += quads[i].startB + 1;

	// Step 1.2.2
	if (medianPair.arr == 'A')
	{
		p2 = medianPair.second;
		q2 = medianPair.second + 1;

		if (A[medianPair.first].w <= B[medianPair.second].w)
		{
			p1 = medianPair.first;
			q1 = medianPair.first + 1;
		}
		else
		{
			p1 = medianPair.first - 1;
			q1 = medianPair.first;
		}
	}
	else
	{

		p1 = medianPair.first;
		q1 = medianPair.first + 1;

		if (B[medianPair.second].w <= A[medianPair.first].w)
		{
			p2 = medianPair.second;
			q2 = medianPair.second + 1;
		}
		else
		{
			p2 = medianPair.second - 1;
			q2 = medianPair.second;
		}
	}

	// Step 1.2.3
	quads[2 * i + 1].startA = q1;
	quads[2 * i + 1].endA = quads[i].endA;
	quads[2 * i + 1].startB = q2;
	quads[2 * i + 1].endB = quads[i].endB;
	
	// Step 1.2.4
	quads[2 * i + 2].startA = quads[i].startA;
	quads[2 * i + 2].endA = p1;
	quads[2 * i + 2].startB = quads[i].startB;
	quads[2 * i + 2].endB = p2;
}

// function to execute Step 2 of EREW MERGE
__kernel void Step2(__global Triple* A, __global Triple* B, __global Triple* out, __global Quad* quads)
{
	const int i = get_global_id(0);
	int w = quads[i].startA + quads[i].startB;
	sequential_merge(A, B, out,
		quads[i].startA, quads[i].endA,
		quads[i].startB, quads[i].endB, w);
}

// function to execute Step 2 of EREW MERGE
__kernel void Step2_reverse(__global Triple* A, __global Triple* B, __global Triple* out, __global Quad* quads)
{
	const int i = get_global_id(0);
	int w = quads[i].startA + quads[i].startB;
	sequential_merge_reverse(A, B, out,
		quads[i].startA, quads[i].endA,
		quads[i].startB, quads[i].endB, w);
}

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
	__global AlignedInt* values_idx,
	__local int* local_group_max)
{
	//const int local_size = get_local_size(0);
	const int group_id = get_group_id(0);
	const int global_id = get_global_id(0);
	const int local_group_id = get_local_id(0);
	/*for (int i = 1; i < local_size; ++i) {
		if (max_profit < a[i].p) {
			max_profit = a[i].p;
			max_idx = i;
		}
	}
	values[group_id] = max_profit;
	values_idx[group_id] = local_size * group_id+max_idx;*/
	//atomic_max((volatile __global int*) & values[group_id].val, a[global_id].p);
	atomic_max((volatile __local int*) local_group_max, a[global_id].p);
	barrier(CLK_LOCAL_MEM_FENCE);
	if (local_group_id == 0) {
		values[group_id].val = *local_group_max;
	}

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
	const int k,
	__local int* max_value_i,
	__local int* out_idx)
{
	//int max_value_i = 0, max_value_b_idx = -1;
	//const int i = get_group_id(0);
	//A = &A[A_local_size * i];
	//out = &out[2 * i];
	//int out_idx = 0;
	//for (int j = i, end = k + i; j < end; ++j)
	//{
	//	const int jmodk = (j % k);
	//	__global Triple* thisB = &B[jmodk * B_local_size];
	//	const int Z = A[0].w + thisB[B_local_size - 1].w;
	//	const int Y = A[A_local_size - 1].w + thisB[0].w;
	//	if (Y <= c)
	//	{
	//		if ((AMax[i].val + BMax[jmodk].val) > max_value_i)
	//		{
	//			max_value_i = AMax[i].val + BMax[jmodk].val;
	//			max_value_b_idx = jmodk;
	//		}
	//	}
	//	else if ((Z <= c) && (Y > c))
	//	{
	//		// choose
	//		out[out_idx].a_idx = i * A_local_size;
	//		out[out_idx++].b_idx = jmodk * B_local_size;
	//	}
	//}
	//max_val_out[i].val = max_value_i;
	///*if (i == 1) print_log(i, "max_value_b_idx: ", max_value_b_idx);
	//if (i == 1) print_log(i, "BMax_idx[max_value_b_idx]: ", BMax_idx[max_value_b_idx]);
	//if (i == 1) print_log(i, "BMax_idx[max_value_b_idx].set: ", B[BMax_idx[max_value_b_idx]].set);*/
	//if (max_value_b_idx >= 0)
	//{
	//	max_val_set_out[i].val =
	//		A[AMax_idx[i].val].set +
	//		B[BMax_idx[max_value_b_idx].val].set;
	//}
	

	const int i = get_group_id(0);
	const int j = get_local_id(0) + get_local_size(0);
	A = &A[A_local_size * i];
	out = &out[2 * i];
	const int jmodk = (j % k);
	B = &B[jmodk * B_local_size];
	const int Z = A[0].w + B[B_local_size - 1].w;
	const int Y = A[A_local_size - 1].w + B[0].w;
	if (Y <= c)
	{
		atomic_max((volatile __local int*)max_value_i, (AMax[i].val + BMax[jmodk].val));
	}
	else if ((Z <= c) && (Y > c))
	{
		// choose
		int out_idx2 = atomic_inc((volatile __local int*)out_idx);
		out[out_idx2].a_idx = i * A_local_size;
		out[out_idx2].b_idx = jmodk * B_local_size;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if (jmodk == 0) {
		max_val_out[i].val = *max_value_i;
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