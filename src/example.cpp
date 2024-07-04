#include "../include/art.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>

using std::string;

void art_bench() {
  std::ifstream file("dataset.txt");
  std::unordered_map<uint32_t, string> dataset;
  uint32_t n = 0;
  string line;
  while (std::getline(file, line)) {
    dataset[n++] = line;
  }
  file.close();

  // fast_zipf rng(n);
  art::Art<int *> m;
  /* std::map<art::key_type, int*> m; */
  /* std::unordered_map<art::key_type, int*> m; */
  int v = 1;
  std::mt19937_64 g(0);
  for (uint32_t i = 0; i < 1000000; ++i) {
    auto k = dataset[rng()];
    m.set(k.c_str(), &v);
    /* m[dataset[rng()]] = &v; */
  }
}

int main() {
  /* std::cout << "sparse insert" << std::endl; */
  /* art_compressions_sparse_insert(); */
  /* std::cout << "sparse delete" << std::endl; */
  /* art_compressions_sparse_delete(); */
  /* std::cout << "paths insert" << std::endl; */
  /* art_compressions_paths_insert(); */
  /* std::cout << "paths delete" << std::endl; */
  /* art_compressions_paths_delete(); */
  /* std::cout << "dense insert" << std::endl; */
  /* art_compressions_dense_insert(); */
  /* std::cout << "dense delete" << std::endl; */
  /* art_compressions_dense_delete(); */

  /* casual_stress_test(16 * 1000 * 1000); */

  //
  // simple example
  art::Art<int> m;

  m.set("aa", 0);
  m.set("aaaa", 1);
  m.set("aaaaaaa", 2);
  m.set("aaaaaaaaaa", 3);
  m.set("aaaaaaaba", 4);
  m.set("aaaabaa", 5);
  m.set("aaaabaaaaa", 6);

  /* The above statements construct the following tree:
   *
   *          (aa)
   *   $_____/ |a
   *   /       |
   *  ()->0   (a)
   *   $_____/ |a\____________b
   *   /       |              \
   *  ()->1   (aa)            (aa)
   *   $_____/ |a\___b         |$\____a
   *   /       |     \         |      \
   *  ()->2 (aa$)->3 (a$)->4 ()->5 (aa$)->6
   *
   */

  auto it = m.begin();
  auto it_end = m.end();
  for (int i = 0; it != it_end; ++i, ++it) {
    std::cout << it.key() << " -> " << *it << std::endl;
  }

  return 0;
}
