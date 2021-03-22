#include <iostream>

int main() {
  float x = 0;
  float y = 0;

  std::string str;

  float i;

  for (i = 1.5; i > -1.5; i -= 0.1) {
    str = "";

    for (x = -1.5; x < 1.5; x += 0.05) {
      y = x * x + i * i - 1;

      if (y * y * y - x * x * i * i * i < 0.0)
        str += "*";
      else
        str += " ";
    }
    std::cout << str << std::endl;
  }
  return 0;
}