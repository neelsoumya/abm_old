#include <iostream>
using namespace std;

inline void error(const string msg)
{
  cerr << msg << endl;
  exit(1);
}

inline void error(const string msg, const string name)
{
  cerr << msg << " " << name << endl;
  exit(1);
}

inline void error(const string msg, int value)
{
  cerr << msg << " " << value << endl;
  exit(1);
}

inline void error(const string msg, double value)
{
  cerr << msg << " " << value << endl;
  exit(1);
}

