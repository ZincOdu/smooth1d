#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX(a, b) ((a) > (b)? (a) : (b))
#define MIN(a, b) ((a) < (b)? (a) : (b))

// smooth子函数: 高斯函数
float gaussian(float x, float sigma) {
	return exp(-0.5f * (x / sigma) * (x / sigma));
}


// 双边滤波仅修正最后一个窗口的数据 空间域用左半个高斯滤波核
// 返回的filtered_data是一个长度为5的数组（半个窗口大小）
void bilateral_filter_1d_last(float* data, int length, float sigma_d, float sigma_r, float* filtered_data) {
	int half_window = 5; //半窗口大小取固定值5 与异常值剔除函数outlier_filter_last的窗口大小对应

	for (int i = 0; i < half_window; i++) {
		float sum_weights = 0.0f;
		float sum_weighted_values = 0.0f;
		int data_idx = length - half_window + i;

		for (int j = -half_window; j <= half_window; j++)
		{
			int index = data_idx + j;
			if (index >= 0 && index < length) {
				// 空间域权重
				float spatial_weight = gaussian(j, sigma_d);
				// 值域权重
				float range_weight = gaussian(data[index] - data[data_idx], sigma_r);
				// 总权重
				float weight = spatial_weight * range_weight;
			
				sum_weights += weight;
				sum_weighted_values += weight * data[index];
			}
		}

		filtered_data[i] = sum_weighted_values / sum_weights;
	}

}


// smooth子函数: 双边滤波
void bilateral_filter_1d(float* data, int length, float sigma_d, float sigma_r, float* filtered_data) {
	int half_window = (int)(3 * sigma_d);  // 取3倍标准差作为窗口大小
	int window_size = 2 * half_window + 1;

	for (int i = 0; i < length; i++) {
		float sum_weights = 0.0f;
		float sum_weighted_values = 0.0f;

		for (int j = -half_window; j <= half_window; j++) {
			int index = i + j;
			if (index >= 0 && index < length) {
				// 空间域权重
				float spatial_weight = gaussian(j, sigma_d);
				// 值域权重
				float range_weight = gaussian(data[index] - data[i], sigma_r);
				// 总权重
				float weight = spatial_weight * range_weight;

				sum_weights += weight;
				sum_weighted_values += weight * data[index];
			}
		}

		filtered_data[i] = sum_weighted_values / sum_weights;
	}
}

// smooth子函数: 交换两个数的值
void swap(float *a, float *b) {
	float temp = *a;
	*a = *b;
	*b = temp;
}

// smooth子函数: 快速排序的分割函数
int partition(float arr[], int low, int high) {
	float pivot = arr[high];
	int i = low - 1;
	for (int j = low; j <= high - 1; j++) {
		if (arr[j] <= pivot) {
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return i + 1;
}

// smooth子函数: 快速排序函数
void quick_sort(float arr[], int low, int high) {
	if (low < high) {
		int pi = partition(arr, low, high);
		quick_sort(arr, low, pi - 1);
		quick_sort(arr, pi + 1, high);
	}
}

// smooth子函数: 计算数组的中位数
float median(float arr[], int n) {
	quick_sort(arr, 0, n - 1);
	if (n % 2 == 0) {
		return (arr[n / 2 - 1] + arr[n / 2]) / 2;
	}
	else {
		return arr[n / 2];
	}
}

// smooth子函数: 中值滤波做异常值剔除
void outlier_filter(float data[], int n) {
	int windowSize = 5;
	int halfWindow = 2;

	float window[5];

	for (int i = 0; i < n; i++) {
		int start = i - halfWindow;
		int end = i + halfWindow;
		if (start < 0) start = 0;
		if (end >= n) end = n - 1;

		int count = 0;
		for (int j = start; j <= end; j++) {
			window[count++] = data[j];
		}

		float med = median(window, count);
		data[i] = med;
	}

}

// smooth_last子函数 iqr方法判断最优一个数是否为异常值
// 若不是则返回原始值 若是则用最邻近值替换
// n需大于等于5
float outlier_filter_last(float data[], int n) {
	float window[5];
	
	for (int i = 0; i < 5; i++){
		window[i] = data[n-1-i];
	}
	
	float res = window[0];
	float near = window[1];
	
	// iqr方法判断是否为异常值
	quick_sort(window, 0, 4);
	float th_iqr = (window[3] - window[1]) * 1.5;
	float th = MAX(window[4] * 0.5, th_iqr);
	if (res < window[1] - th || res > window[3] + th){
		res = near;
	}

	return res;
}

// 数据平滑函数
// data: 待平滑数据地址
// length: 待平滑数据长度（数据长度最好大于等于50）
// smoothed_data: 平滑后数据存放的地址
void smooth(float* data, int length, float* smoothed_data) {
	
	bilateral_filter_1d(data, length, 3.0, 30.0, smoothed_data);
	outlier_filter(smoothed_data, length);
}

//数据平滑 仅输出最后一个数据
// data: 待平滑数据地址
// length: 待平滑数据长度 需大于等于5 取值为11则效果最佳
float smooth_last(float* data, int length) {

	float filtered_data[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	bilateral_filter_1d_last(data, length, 3.0, 30.0, filtered_data);
	return outlier_filter_last(filtered_data, 5);

}

// 使用示例1
# if 0
int main() {
	
	// 原始数据
	float data[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};
	int length = sizeof(data) / sizeof(data[0]);
	
	// 待平滑数据申请内存
	float* smoothed_data = (float*)malloc(length * sizeof(float));

	// 平滑
	smooth(data, length, smoothed_data);

	// 结果打印
	printf("原始数据: ");
	for (int i = 0; i < length; i++) {
		printf("%f,", data[i]);
	}
	printf("\n");

	printf("平滑数据: ");
	for (int i = 0; i < length; i++) {
		printf("%f,", smoothed_data[i]);
	}
	printf("\n");

	// 内存释放
	free(smoothed_data);
	return 0;
}
#endif

// 使用示例2: 读一个文件中的数据做平滑操作 平滑后的数据写到另一个文件里
#if 0

#define MAX_LINES 4000  // 文件最大行数

int read_file_to_array(const char *filename, float *array, int max_lines) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	int count = 0;
	while (count < max_lines && fscanf(file, "%f", &array[count]) == 1) {
		count++;
	}

	fclose(file);
	return count;
}

int main() {

	// 输入数据文件
	const char *filename = "./dat.txt";

	// 输出文件
	const char *filename_out = "./dat_smoothed.txt";
	FILE *file = fopen(filename_out, "w");
	fclose(file);

	// 读取数据
	float data[MAX_LINES];
	int num_values = read_file_to_array(filename, data, MAX_LINES);

	// 待平滑数据的长度 每sm_data_length个数据处理一次
	int batch_sz = 50;

	// 待平滑数据申请内存
	float* smoothed_data = (float*)malloc(batch_sz * sizeof(float));

	// 开始处理
	int start = 0;
	while (start < num_values) {
		
		int end = start + batch_sz;
		
		if (end > num_values) {
			end = num_values;
		}
		// 平滑
		int sm_data_length = end - start;
		smooth(data + start, sm_data_length, smoothed_data);
		
		//// 平滑结果追加写到文件
		//file = fopen(filename_out, "a");
		//for (int i = 0; i < sm_data_length; i++) {
		//	fprintf(file, "%f\n", smoothed_data[i]);
		//}
		//fclose(file);

		start += batch_sz;
	}

	// 内存释放
	free(smoothed_data);
	return 0;
}

#endif


// 使用示例3: ic、tc数据
#if 0

#define MAX_LINES 500  // 文件最大行数

int read_file_to_array(const char *filename, float *array, int max_lines) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	int count = 0;
	while (count < max_lines && fscanf(file, "%f", &array[count]) == 1) {
		count++;
	}

	fclose(file);
	return count;
}

int main() {

	// 输入数据文件
	const char *filename = "./tc.txt";

	// 输出文件
	const char *filename_out = "./tc_smoothed.txt";
	FILE *file = fopen(filename_out, "w");
	fclose(file);

	// 读取数据
	float data[MAX_LINES];
	int num_values = read_file_to_array(filename, data, MAX_LINES);

	// 待平滑数据的长度 每sm_data_length个数据处理一次
	int batch_sz = 15;
	int step = 1;

	// 待平滑数据申请内存
	float* smoothed_data = (float*)malloc(batch_sz * sizeof(float));

	// 开始处理
	int start = 0;
	while (start < num_values) {

		int end = start + batch_sz;

		if (end > num_values) {
			end = num_values;
		}
		// 平滑
		int sm_data_length = end - start;
		smooth(data + start, sm_data_length, smoothed_data);

		// 平滑结果追加写到文件
		file = fopen(filename_out, "a");
		fprintf(file, "%f\n", smoothed_data[0]);
		fclose(file);

		start += step;
	}

	// 内存释放
	free(smoothed_data);
	return 0;
}

#endif

// 使用示例4
# if 0
int main() {

	// 原始数据
	float data[] = { 571.9,	567.6,	570.9,	574.9,	565.4,	571.4,	567.7,	563.7,	566.5,	564.9,	566.6,	566.3,	565.9,	565.8,	566.7,	567.5,	567,	566.1,	564.4,	562.2,	561,	560.6,	560,	559.2,	558.8,	557.2,	534.1,	291.3,	70.5,	70.1,	70.1,	69.7,	69.3,	69.2,	69.1,	69.1,	68.7,	68.1,	67.6,	67.2,	67.1,	67.2,	67.6,	67.9,	67.5,	67,	66.5,	66.2,	66,	65.6,	65.3,	65.1,	65.1,	65.3,	65.7,	66.4,	67.2,	68.2,	69.1,	69.4,	69.7,	68.8,	67.8,	66.7,	65.9,	65.3,	65.2,	64.8,	64.4,	64.2,	64.1,	64.2,	64.5,	65,	65.8,	66.8,	67.8,	69.6,	68.9,	66.8,	63.6,	61.2,	59.4,	58.1,	57.8,	57.6,	57.7,	58,	58.6,	59.1,	59,	59,	59,	58.9,	59.1,	59,	59,	59.1,	59.1,	58.8,	58.7,	58.7,	58.7,	59,	59.3,	59.2,	59,	59.1,	59.6,	59.5,	59.2,	59.1,	59.2,	59.2,	58.9,	59.1,	59.2,	59.3,	59,	58.8,	59,	59.2,	59.2,	59,	58.8,	59,	59.3,	59.3,	59.1,	59.1,	59.3,	59.4,	59.2,	59,	58.9,	58.8,	58.7,	58.7,	58.5,	58.5,	58.6,	59.1,	58.9,	58.7,	58.8,	58.9, };
	int length = sizeof(data) / sizeof(data[0]);

	// 待平滑数据的长度 每sm_data_length个数据处理一次
	int batch_sz = 12;
	int step = 1;

	// 待平滑数据申请内存
	float* smoothed_data = (float*)malloc(batch_sz * sizeof(float));

	// 开始处理
	int start = 0;
	while (start < length) {

		int end = start + batch_sz;

		if (end > length) {
			end = length;
		}
		// 平滑
		int sm_data_length = end - start;
		smooth(data + start, sm_data_length, smoothed_data);

		printf("%f,", smoothed_data[5]);

		start += step;
	}

	// 内存释放
	free(smoothed_data);
	return 0;
}
#endif

// 测试
# if 0
int main() {

	float data[11] = {567, 566, 564, 562, 561, 560, 558, 557, 534, 291, 70};

	printf("%f", smooth_last(data, 11));
	return 0;
}
#endif

// 测试 读文件
#if 1

#define MAX_LINES 4000  // 文件最大行数

int read_file_to_array(const char *filename, float *array, int max_lines) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	int count = 0;
	while (count < max_lines && fscanf(file, "%f", &array[count]) == 1) {
		count++;
	}

	fclose(file);
	return count;
}

int main() {

	// 输入数据文件
	const char *filename = "./dat3.txt";

	// 读取数据
	float data[MAX_LINES];
	int num_values = read_file_to_array(filename, data, MAX_LINES);

	float min_outlier_th = 300.f;

	// 开始处理
	int start = 0;
	while (start < num_values) {

		int end = start + 11;

		if (end > num_values) {
			return 0;
		}

		float smoothed = smooth_last(data + start, 11, min_outlier_th);
		float raw = data[start + 10];
		printf("%f,%f\n", raw, smoothed);
		
		start += 1;
	}

	return 0;
}

#endif