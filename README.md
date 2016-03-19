# Open SMPP

SMPP Library for C and C++ based on [`libsmpp34`](http://c-open-smpp-34.sourceforge.net/)

A `ESME` simulator, which is implemented in C# using P/Invoke, is included , it should work with any `SMSC`. Should compile with Mono Develop and Visual Studio 2008, easily upgradable to other Visual Studio version. The fun stuff is in [`smpplib.cs`](tests/esmesim/smpplib.cs) anyway.

### Requirements:

* boost >= 1.44

### Installation

```sh
git clone git@github.com:ichramm/opensmpp.git
cd opensmpp
make Configuration=Release # or Configuration=Debug (default: Release)
make Configuration=Release install # Uses env var PREFIX (default: /usr/local)
```
