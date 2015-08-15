/*
 * Main.cpp
 *
 * Created on: 14.08.2015
 * Author: Anton Larionov
 */

// директивы include сгенерированы автоматически
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>

// прототипы функций
void write_numbers(const char* const, const size_t&,
		const std::function<int()>&);
size_t more_effective(const char* const, const size_t&, const size_t&);

bool test();

int main(int argc, char **argv) {

	// тестирование
	if (!test()) {
		std::cerr << "Program is broken" << std::endl;
		return (-1);
	}

	// генератор псевдослучайных чисел в диапазоне [a, b] (по умолчанию [1,2])
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine dre { seed };
	auto rnd = [&dre](int&& a = 1, int&& b = 2) {return a + dre() % b;};

	// количество чисел для записи в файл от 1 до миллиарда
	size_t N { rnd(1, 1E9) };

	// запись в файл N псевдослучайных чисел Pi, получаемых по заданной формуле
	write_numbers("_file_.bin", N, rnd);

	// два произвольных числа
	size_t A { rnd(0, N) };
	size_t B { rnd(N, 2E9) };

	std::cout << "Total numbers in the file: " << N << std::endl;

	// вывести количество чисел Pi, удовлетворяющих условию A < Pi < B
	size_t result = more_effective("_file_.bin", A, B);
	std::cout << "Count of numbers in the range (" << A << ", " << B << "): "
			<< result << std::endl;

	return (0);
}

/**
 * Программа записи в файл псевдослучайных чисел Pi,
 * получаемых по формуле: P(i) = P(i-1) + rand(1,2); P(0) = 0.
 *
 * @param file_name имя файла
 * @param N количество чисел для записи (N > 0)
 * @param random функция генерации чисел rand(1,2)
 */
void write_numbers(const char* const file_name, const size_t& N,
		const std::function<int()>& random) {

	// файл для записи в бинарном режиме
	FILE* const pFile = fopen(file_name, "wb");
	if (!pFile) {
		std::cerr << "Can't open input file" << std::endl;
	}

	// инициализаци и запись в файл первого числа
	size_t value { };
	fwrite(&value, 1, sizeof(size_t), pFile);

	for (size_t i = 1; i < N; ++i) {
		// вычисление и запись Pi
		value += random();
		fwrite(&value, 1, sizeof(size_t), pFile);
	}
	fclose(pFile);
}

/**
 * Программа определения эффективным способом количества чисел Pi,
 * удовлетворяющих условию a < Pi < b
 *
 * @param file_name имя файла
 * @param a произвольное число
 * @param b произвольное число
 *
 * @return количество чисел в файле удовлетворяющих условию a < Pi < b
 */
size_t more_effective(const char* const file_name, const size_t& a,
		const size_t& b) {

	// если границы совпадают
	if (a == b) {
		return 0;
	}

	// файл для чтения в бинарном режиме
	FILE* const pFile = fopen(file_name, "rb");
	if (!pFile) {
		std::cerr << "Can't open input file" << std::endl;
		return 0;
	}

	// нижняя и верхняя границы
	const size_t min = std::min(a, b);
	const size_t max = std::max(a, b);

	// определение количества всех чисел в файле
	fseek(pFile, 0, SEEK_END);
	const size_t length = ftell(pFile) / sizeof(size_t);

	// минимально возможная позиция нижней границы
	size_t pos_min { min >> 1 };

	// если выход нижней границы за пределы файла
	if (pos_min > (length - 1)) {
		fclose(pFile);
		return 0;
	}

	size_t value { };

	// переход и считывание значение на предполагаемой позиции
	fseek(pFile, sizeof(size_t) * pos_min, SEEK_SET);
	fread(&value, sizeof(size_t), 1, pFile);

	size_t tmp { };
	while (value <= min) {
		// если разница между нижней границей и считаным значением велика,
		// то вычисление следующей минимально возможной позиции
		if ((min - value) > 2) {
			tmp = (min - value) >> 1;
			pos_min += tmp;
		} else {
			tmp = 0;
		}

		// контроль выхода за пределы файла
		++pos_min;
		if (pos_min > (length - 1)) {
			fclose(pFile);
			return 0;
		}

		// переход и считывание значение на предполагаемой позиции
		fseek(pFile, sizeof(size_t) * tmp, SEEK_CUR);
		fread(&value, sizeof(size_t), 1, pFile);
	}

	// минимально возможная позиция верхней границы
	size_t pos_max = max >> 1;

	// если выход верхней границы за пределы файла
	if (pos_max > (length - 1)) {
		fclose(pFile);
		return (length - pos_min);
	}

	fseek(pFile, sizeof(size_t) * pos_max, SEEK_SET);
	fread(&value, sizeof(size_t), 1, pFile);

	tmp = 0;
	while (value < max) {
		// если разница между верхней границей и считаным значением велика,
		// то вычисление следующей минимально возможной позиции
		if ((max - value) > 2) {
			tmp = (max - value) >> 1;
			pos_max += tmp;
		} else {
			tmp = 0;
		}

		// контроль выхода за пределы файла
		++pos_max;
		if (pos_max > (length - 1)) {
			fclose(pFile);
			return (length - pos_min);
		}

		// переход и считывание значение на предполагаемой позиции
		fseek(pFile, sizeof(size_t) * tmp, SEEK_CUR);
		fread(&value, sizeof(size_t), 1, pFile);
	}

	fclose(pFile);
	return (pos_max - pos_min);
}

/**
 * Программа определения простым способом количества чисел Pi,
 * удовлетворяющих условию a < Pi < b
 *
 * @param file_name имя файла
 * @param a произвольное число
 * @param b произвольное число
 *
 * @return количество чисел в файле удовлетворяющих условию a < Pi < b
 */
size_t simple(const char* const file_name, const size_t& a, const size_t& b) {
	// файл для чтения в бинарном режиме
	FILE* const pFile = fopen(file_name, "rb");
	if (!pFile) {
		std::cerr << "Can't open input file" << std::endl;
		return 0;
	}
	const size_t min = std::min(a, b);
	const size_t max = std::max(a, b);

	// определение количества всех чисел в файле
	fseek(pFile, 0, SEEK_END);
	size_t size = ftell(pFile) / sizeof(size_t);
	fseek(pFile, 0, SEEK_SET);

	size_t n { };
	size_t value { };

	// простой перебор
	for (size_t i = 0; i < size; ++i) {
		fread(&value, sizeof(size_t), 1, pFile);
		if ((value > min) && (value < max))
			n++;
	}
	fclose(pFile);
	return n;
}

bool test() {
	// файл для чтения и записи в бинарном режиме
	const char* file_name = "test_file.bin";
	FILE* const pFile = fopen(file_name, "w+b");
	if (!pFile) {
		return false;
	}

	// тестовая последовательность
	size_t value[] { 0, 1, 2, 4, 5, 7, 8, 9, 10, 12 };
	fwrite(&value, 1, 10 * sizeof(size_t), pFile);
	fclose(pFile);

	bool tests[] = {
		more_effective(file_name, 0, 3) == simple(file_name, 0, 3),
		more_effective(file_name, 1, 5) == simple(file_name, 1, 5),
		more_effective(file_name, 6, 2) == simple(file_name, 6, 2),
		more_effective(file_name, 9, 12) == simple(file_name, 9, 12),
		more_effective(file_name, 3, 20) == simple(file_name, 3, 20),
		more_effective(file_name, 13, 23) == simple(file_name, 13, 23),
		more_effective(file_name, 0, 13) == simple(file_name, 0, 13),
		more_effective(file_name, 1, 2) == simple(file_name, 1, 2),
		more_effective(file_name, 3, 7) == simple(file_name, 3, 7),
		more_effective(file_name, 8, 10) == simple(file_name, 8, 10),
		more_effective(file_name, 15, 19) == simple(file_name, 15, 19),
		more_effective(file_name, 1, 9) == simple(file_name, 1, 9),
		more_effective(file_name, 2, 14) == simple(file_name, 2, 14),
		more_effective(file_name, 11, 13) == simple(file_name, 11, 13),
		more_effective(file_name, 101, 230) == simple(file_name, 101, 230),
		more_effective(file_name, 5, 5) == simple(file_name, 5, 5)
	};

	bool result { true };
	for (auto& test : tests) {
		result = result && test;
	}

	remove(file_name);

	return result;
}

