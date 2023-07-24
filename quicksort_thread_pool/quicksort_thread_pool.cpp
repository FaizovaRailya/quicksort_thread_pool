#include <iostream>
#include <chrono>
#include "QuickSort.h"

int main(){
    
	srand(0);
	ThreadPool tp;
	
	std::vector<int> v;
	const int experiment_num = 10;
	const int experiment_size = 100000;

	//тестовые примеры для одного потока
	double summ_time1 = 0;
	for (int i = 0; i < experiment_num; ++i) {
		fill_random(v, experiment_size, i);

		auto start1 = std::chrono::high_resolution_clock::now();
		quicksort_single(v, 0, v.size() - 1);
		auto stop1 = std::chrono::high_resolution_clock::now();
		std::cout << check_v(v, 0, v.size() - 1) << "\n";
		std::chrono::duration<double> elapsed_time1 = stop1 - start1;
		summ_time1 = summ_time1 + elapsed_time1.count();
		if (!check_v(v, 0, v.size() - 1)) std::cout << "FAIL!!!" << std::endl;
	}
	std::cout << "SINGLE THREAD DONE. Elapsed time : " << summ_time1 / experiment_num << std::endl;

	//тестовые примеры для нескольких потоков
	double summ_time2 = 0;
	for (int i = 0; i < experiment_num; ++i) {
		fill_random(v, experiment_size, i);

		auto start2 = std::chrono::high_resolution_clock::now();
		quicksort_multithread_nopool(v, 0, v.size() - 1, true);
		auto stop2 = std::chrono::high_resolution_clock::now();

		std::cout << check_v(v, 0, v.size() - 1) << "\n";

		std::chrono::duration<double> elapsed_time2 = stop2 - start2;
		summ_time2 = summ_time2 + elapsed_time2.count();
		if (!check_v(v, 0, v.size() - 1)) std::cout << "FAIL!!!" << std::endl;
	}
	std::cout << "MULTIPLE THREAD WITH NO THREADPOOL DONE. Elapsed time : " << summ_time2 / experiment_num << std::endl;

	
	
	//тестовые примеры для пула потоков
	double summ_time3 = 0;
	for (int i = 0; i < experiment_num; ++i) {
		fill_random(v, experiment_size, i);

		auto start3 = std::chrono::high_resolution_clock::now();
		quicksort_threadpool(v, 0, v.size() - 1, true, tp, 1000);
		auto stop3 = std::chrono::high_resolution_clock::now();

		std::cout << check_v(v, 0, v.size() - 1) << "\n";
		std::chrono::duration<double> elapsed_time3 = stop3 - start3;
		summ_time3 = summ_time3 + elapsed_time3.count();
		if (!check_v(v, 0, v.size() - 1)) std::cout << "FAIL!!!" << std::endl;
	}
	std::cout << "MULTIPLE THREAD WITH THREADPOOL DONE. Elapsed time : " << summ_time3 / experiment_num << std::endl;
	
	return 0;
}
