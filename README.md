# uvchan
Channels of GO! Resurrected in C using libuv

```bash
pushd /usr/local/src
sudo git clone https://github.com/libuv/libuv.git
cd libuv
sudo git fetch --tags
sudo git checkout tags/v1.19.2
sudo ./autogen.sh
sudo ./configure
sudo make
sudo make install
popd
```
