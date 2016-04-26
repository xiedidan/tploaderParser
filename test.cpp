#include <dlfcn.h>

class Printer
{
  public:
  print();
}

int main(void)
{
  void *dl_handle;
  void (*invoke_parser)(Printer* printer);


  return 0;
}
