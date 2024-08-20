#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// smooth子函数: 高斯函数
float gaussian(float x, float sigma) {
	return exp(-0.5f * (x / sigma) * (x / sigma));
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


// 数据平滑函数
// data: 待平滑数据地址
// length: 待平滑数据长度（数据长度最好大于等于50）
// smoothed_data: 平滑后数据存放的地址
void smooth(float* data, int length, float* smoothed_data) {
	
	bilateral_filter_1d(data, length, 10.0, 1200.0, smoothed_data);
	outlier_filter(smoothed_data, length);
}

// 使用示例1
# if 0
int main() {
	
	// 原始数据
	float data[] = {1100, 966, 476, 1132, 1270, 853, 464, 993, 1276, 1223,
		891, 499, 1273, 1245, 841, 485, 1214, 1214, 1000, 372, 1112};
	int length = sizeof(data) / sizeof(data[0]);
	
	// 待平滑数据申请内存
	float* smoothed_data = (float*)malloc(length * sizeof(float));

	// 平滑
	smooth(data, length, smoothed_data);

	// 结果打印
	printf("原始数据: ");
	for (int i = 0; i < length; i++) {
		printf("%f ", data[i]);
	}
	printf("\n");

	printf("平滑数据: ");
	for (int i = 0; i < length; i++) {
		printf("%f ", smoothed_data[i]);
	}
	printf("\n");


	// 内存释放
	free(smoothed_data);
	return 0;
}
#endif

// 使用示例2: 读一个文件中的数据做平滑操作 平滑后的数据写到另一个文件里
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
		
		// 平滑结果追加写到文件
		file = fopen(filename_out, "a");
		for (int i = 0; i < sm_data_length; i++) {
			fprintf(file, "%f\n", smoothed_data[i]);
		}
		fclose(file);

		start += batch_sz;
	}

	// 内存释放
	free(smoothed_data);
	return 0;
}

#endif




