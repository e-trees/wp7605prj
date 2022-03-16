# Log with Location for WP7605

Software that sends location information and UNIX time along with text logs.

## Build
```bash
$ . /opt/swi/SWI9X07Y_02.28.03.05/environment-setup-armv7a-neon-poky-linux-gnueabi
$ make
```

## Usage
```bash
$ ./demo <Endpoint URL to send the log>
# ./demo https://example.com/api
# ->https://example.com/api?msg=hello&lat=1.234567&lon=%9.876543&time=1646060400
```


# Referenced WebSite
https://blog.lvgl.io/2018-01-03/linux_fb


# LVGL for frame buffer device

LVGL configured to work with /dev/fb0 on Linux.

When cloning this repository, also make sure to download submodules (`git submodule update --init --recursive`) otherwise you will be missing key components.

Check out this blog post for a step by step tutorial:
https://blog.lvgl.io/2018-01-03/linux_fb
