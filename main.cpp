#include <iostream>
#include <pthread.h>
#include <random>

struct Args
{
  double r;
  size_t tests;
  size_t seed;
};

double area(double r, size_t threads, size_t tests);
size_t calc(double r, size_t tests, size_t seed);
bool isInside(double x, double y, double r);
void* sample(void* data);

double area(double r, size_t threads, size_t tests)
{
  if (r <= 0 || threads == 0 || tests == 0)
  {
    throw std::invalid_argument(
      "Radius, threads and tests must be greater than 0"
    );
  }

  std::vector< pthread_t > pthreads(threads);
  std::vector< Args > arguments(threads);

  const std::size_t testsPerThread = tests / threads;
  const std::size_t remainder = tests % threads;

  std::size_t createdThreads = 0;

  for (std::size_t i = 0; i < threads; ++i)
  {
    const std::size_t currentTests =
      testsPerThread + (i < remainder ? 1 : 0);

    arguments[i] = {
      r,
      currentTests,
      i + 1
    };

    const int error = pthread_create(
      &pthreads[i],
      nullptr,
      sample,
      &arguments[i]
    );

    if (error != 0)
    {
      for (std::size_t j = 0; j < createdThreads; ++j)
      {
        pthread_join(pthreads[j], nullptr);
      }

      throw std::runtime_error(
        std::string("pthread_create: ") + std::strerror(error)
      );
    }

    ++createdThreads;
  }

  std::size_t totalPassed = 0;

  for (std::size_t i = 0; i < threads; ++i)
  {
    void* threadResult = nullptr;

    const int error = pthread_join(
      pthreads[i],
      &threadResult
    );

    if (error != 0)
    {
      throw std::runtime_error(
        std::string("pthread_join: ") + std::strerror(error)
      );
    }

    totalPassed += static_cast< std::size_t >(
      reinterpret_cast< std::uintptr_t >(threadResult)
    );
  }

  const double squareArea = 4.0 * r * r;

  return squareArea *
    static_cast< double >(totalPassed) /
    static_cast< double >(tests);
}

size_t calc(double r, size_t tests, size_t seed)
{
  size_t passed = 0;
  std::mt19937 gen(seed);
  std::uniform_real_distribution< double > dist(-r, r);
  
  for (size_t i = 0; i < tests; ++i) {
    double x = dist(gen);
    double y = dist(gen);
    if (isInside(x, y, r)) {
      passed++;
    }
  }
  return passed;
}

bool isInside(double x, double y, double r)
{
  return (x * x + y * y) <= (r * r);
}

void* sample(void* data) {
  auto* args = static_cast<Args*>(data);

  const size_t hits = calc(
    args->r,
    args->tests,
    args->seed
  );

  return reinterpret_cast<void*>(
    static_cast<std::uintptr_t>(hits)
  );
}

int main()
{
  double r;
  size_t threads, tests;

  std::cout << "write radius: ";
  if (!(std::cin >> r))
  {
    std::cerr << "radius error\n";
    return 1;
  }

  std::cout << "write amount of threads: ";
  if (!(std::cin >> threads))
  {
    std::cerr << "count of threads error\n";
    return 1;
  }


  std::cout << "write count of tests: ";
  if (!(std::cin >> tests))
  {
    std::cerr << "count of tests error\n";
    return 1;
  }

  double result = area(r, threads, tests);

  std::cout << "area: " << result << '\n';
}
