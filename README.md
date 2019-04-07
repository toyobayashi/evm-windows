# EVM for Windows

Electron Version Manager.

``` txt
Usage:

  evm version:                          Displays the current running version of evm for Windows. Aliased as v.
  evm arch [ia32 | x64]:                Show or set electron arch.
  evm mirror [github | taobao | <url>]: Show or set mirror.
  evm path [<dir>]:                     Show or set electron directory.
  evm cache [<dir>]:                    Show or set electron download cache directory.
  evm root [<dir>]:                     Show or set the directory where evm should store different versions of electron.
  evm install <version> [options]
  evm list:                             List the electron installations.
  evm use <version> [options]:          Switch to use the specified version.
  evm uninstall <version>:              The version must be a specific version.

Options:

  --arch=<ia32 | x64>
  --mirror=<github | taobao | <url>>
  --path=<dir>
  --cache=<dir>
  --root=<dir>

```

## Quick Start

1. Add evm directory to `%PATH%` manually.
2. Set evm electron path

    ``` cmd
    > evm path D:\Example\electron
    ```
    and add it to `%PATH%` manually.

3. Use evm like nvm.

    ``` cmd
    > evm use 4.1.4
    > electron -v
    ```

