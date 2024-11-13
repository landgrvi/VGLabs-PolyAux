#include <iostream>
#include <simd/Vector.hpp>
#include <simd/functions.hpp>
#include <chrono>
using namespace std::chrono;

using namespace std;
using namespace rack;

int main() {
	float f;
	simd::float_4 fs = { };
	simd::float_4 f4s[4] = { };
	int repeats = 100000;

	cout<<"Hello, World!\n";

	simd::float_4 foo[4] = {1,2,3,4};
	simd::float_4 pows = {2,3,4,5};
	simd::float_4 gains[2] = {{1,2,3,4},{5,6,7,8}};
	
	float ones[8] = {1,2,3,4,5,6,7,8};
	simd::float_4 twos[2] = {{2,2,2,2},{2,2,2,2}};
	simd::float_4 threes[2] = { };
	
	auto start = high_resolution_clock::now();
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	start = high_resolution_clock::now();
	for (int c = 0; c < repeats; c++) {
		for (int i = 0; i < 4; i++) {
			f4s[i] = simd::pow(foo[i],2);
		}
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop - start);
	cout << duration.count() << endl;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << f4s[i][j];
			cout << " ";
		}
	}
	cout << endl;
	
	start = high_resolution_clock::now();
	for (int c = 0; c < repeats; c++) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				f4s[i][j] = simd::pow(foo[i][j],2);
			}
		}
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop - start);
	cout << duration.count() << endl;
	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << f4s[i][j];
			cout << " ";
		}
	}
	cout << endl;
	f4s[0] -= f4s[3];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << f4s[i][j];
			cout << " ";
		}
	}
	cout << endl;
	
	threes[0] = *(reinterpret_cast<simd::float_4*>(&ones[0])) + twos[0];
	threes[1] = *(reinterpret_cast<simd::float_4*>(&ones[4])) + twos[1];
	
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			cout << threes[i][j];
			cout << " ";
		}
	}
	cout << endl;

}
