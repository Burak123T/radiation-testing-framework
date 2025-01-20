# *rad_logger* library

This library exposes multiple functions, which are traced using u(ret)probes in the *radiation testing framework*.

These functions include:

## **TO-DO**

- [ ] Add more interesting logging functions and accompanying aggregations in eBPF (e.g. throughput, latency histograms)

## Usage

Users can use the *rad_logger* library by simply importing the library and calling any of the above-mentioned functions, no initialization required.

C/C++:
```C
#include <rad_logger.h>

while (1)
{
    log_start(1, "custom_event_name", "custom_event_type", "custom_event_data");
    sleep(1);
}
```

or in Python:

```python
from rad_logger import log_start
from time import sleep

while True:
    log_start(1, "custom_event_name", "custom_event_type", "custom_event_data")
    sleep(1)
```

## Python bindings

We also provide Python bindings, which allow for tracing of Python applications.

### Build instructions

The Python bindings require the shared object library to be compiled and installed on your system.
Afterwards, the Python bindings can be simply built and install using a single command:

```sh
cd python
pip install .
```
