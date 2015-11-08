# Open SMPP
SMPP Library for C and C++ based on [`libsmpp34`](http://c-open-smpp-34.sourceforge.net/)

### Requirements:

* boost >= 1.44

### Installation

```sh
git clone git@github.com:ichramm/opensmpp.git
cd opensmpp
make Configuration=Release # or Configuration=Debug (default: Release)
make Configuration=Release install # Uses env var PREFIX (default: /usr/local)
```
