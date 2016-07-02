# Example of external usage

### Quick start:

```bash
$ vim make_gen.cpp  # modify `PARACEL_INSTALL_PREFIX` etc.
$ g++ make_gen.cpp -std=c++11 -lmsgpack -lzmq -lboost_regex -I<PARACEL_INSTALL_PREFIX>/include
$ make
$ <PARACEL_INSTALL_PREFIX>/prun.py -w 1 -p 1 -c demo_cfg.json ./demo
```
