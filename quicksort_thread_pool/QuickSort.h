#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include "ThreadUtils.h" 
int multi_size = 1000;

void fill_random(std::vector<int>& v, int size, int seed) {
	srand(seed);
	v.clear();
	v.resize(size);
	for (int i = 0; i < size; ++i) {
		v[i] = rand() % 10000;
	}
}
void print_v(std::vector<int>& v, int left, int right) {
	for (int i = left; i <= right; i++) {
		std::cout << v[i] << " ";
	}
	std::cout << "\n";
}
bool check_v(std::vector<int>& v, int left, int right) {
	for (int i = left + 1; i <= right; ++i) { if (v[i - 1] > v[i]) return false; }
	return true;
}
int partition(std::vector<int>& array, int left, int right) {
	int i = left;
	int j = right - 1;
	int p = right;
	while ((i < right - 1) && (j > left)) {
		
		/*std::cout << "\n\n";
		std::cout << "i: " << i << "   j: " << j << "   p: " << p << "  arr[p]: " << array[p] << "\n";
		print_v(array,left,right);
		std::cout << "\n\n";*/	
		while (array[i] <= array[p]) {
			++i;
			if (i >= j) break;
		}
		//std::cout << i << " ";
		//std::cout << "\n";
		while (array[j] >= array[p]) {
			--j;
			if (i >= j) break;
		}
		//std::cout << j << " ";
		//std::cout << "\n";
		if (i >= j) break;
		std::swap(array[i], array[j]);
		++i;
		--j;
	}
	/*std::cout << "\n\n";
	std::cout << "LAST:\n";
	std::cout << "i: " << i << "   j: " << j << "   p: " << p << "  arr[p]: " << arr[p] << "\n";
	print_v(arr);
	std::cout << "\n\n";	//*/
	if (array[i] > array[p]) std::swap(array[i], array[p]);
	return i;
}
void quicksort_single(std::vector<int>& array, int left, int right) {
	if (left >= right) return;
	/*std::cout << "\nbefore\n";
	print_v(array, left, right);
	std::cout << "\n\n";*/	

	int m = partition(array, left, right);
	/*std::cout << "m: " << m << "\n";
	std::cout << "\nafter\n";
	print_v(array, left, right);
	std::cout << "\n\n";*/	

	quicksort_single(array, left, m);
	quicksort_single(array, m + 1, right);
}
void quicksort_multithread_nopool(std::vector<int>& array, int left, int right, bool enable) {
	if (left >= right) return;
	int size = right - left;
	int m = partition(array, left, right);
	if ((size >= 1000) && enable) {
		auto f = std::async(std::launch::async, [&]() {
			quicksort_multithread_nopool(array, left, m, true);
			});
		quicksort_multithread_nopool(array, m + 1, right, true);
	}
	else {
		quicksort_multithread_nopool(array, left, m, false);
		quicksort_multithread_nopool(array, m + 1, right, false);
	}
}

bool make_thread = false;
void quicksort_threadpool(std::vector<int>& array, int left, int right, bool enable, ThreadPool& tp, int multi_size) {
	if (left >= right) return;
	int left_bound = left;
	int right_bound = right;
	int middle = array[(left_bound + right_bound) / 2];
	//Меняем элементы местами
	while (left_bound <= right_bound) {
		while (array[left_bound] < middle) {
			left_bound++;
		}
		while (array[right_bound] > middle) {
			right_bound--;
		}
		if (left_bound <= right_bound) {
			std::swap(array[left_bound], array[right_bound]);
			left_bound++;
			right_bound--;
		}
	}
	if (make_thread && (right_bound - left > 10000)) {
		std::promise<void> promise;
		std::future<void> future = promise.get_future();
		// если элементов в левой части больше чем 10000
		// вызываем асинхронно рекурсию для правой части
		auto f = std::async(std::launch::async, [&](std::shared_ptr<std::vector<int>> arr_ptr) {
			quicksort_threadpool(*arr_ptr, left, right_bound, false, tp, multi_size);
			promise.set_value();
			},
			std::make_shared<std::vector<int>>(array));
		quicksort_threadpool(array, left_bound, right, true, tp, multi_size);
		future.wait();
	}
	else {
		quicksort_threadpool(array, left, right_bound, false, tp, multi_size);
		quicksort_threadpool(array, left_bound, right, false, tp, multi_size);
	}
}