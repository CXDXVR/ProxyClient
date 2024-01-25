## ProxyClient
It is a CLI proxy client for Windows that uses WinAPI hooks to redirect the target application traffic to the proxy server.<br>
Currently supports only TCP socks4 and socks5 connections. Does not support subprocess contamination and injection into processes other than the client process bitness.

### Project structure:
`/client` - The main console application of the client.<br>
`/common` - Static library that implements methods used in different parts of the project.<br>
`/libs` - Third-party libraries.<br>
`/redirector` - DLL library acting as a redirector. Used for injection into target applications.<br>
`/winpipe` - Static library providing simple classes for working with named pipes.<br>

### Third-party libraries:
- [argparse](https://github.com/p-ranav/argparse/)
- [minhook](https://github.com/TsudaKageyu/minhook/)
- [spdlog](https://github.com/gabime/spdlog/)

## Installation:
```
git clone https://github.com/WxyToHeaven/ProxyClient.git
git submodule init
git submodule update
cd ProxyClient
cmake build . -DCMAKE_BUILD_TYPE=Release -A x64 -B ./build
```

## Usage:
```
$ ./client.exe -h
Usage: client.exe [--help] [--version] [--pid VAR...] [--name VAR...] [--enable-log] [--proxy-type VAR] [--proxy-v4 VAR] [--proxy-v6 VAR]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  --pid          pid of a process to inject. [nargs=0..6] [default: {}]
  --name         short filename of a process with wildcard matching to inject. [nargs=0..6] [default: {}]
  --enable-log   enable logging for network connections.
  --proxy-type   proxy type (socks4 or socks5). [nargs=0..1] [default: "socks4"]
  --proxy-v4     set a proxy IPv4 address for network connections. [nargs=0..1] [default: ""]
  --proxy-v6     set a proxy IPv6 address for network connections. [nargs=0..1] [default: ""]
```
