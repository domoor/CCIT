**netfilter_test**
-------------

### Usage.

```
ex)
# iptables -A INPUT -j NFQUEUE
# iptables -A OUTPUT -j NFQUEUE
# ./netfilter_test
```


### Install the library.

```
ex)
# apt install libnetfilter-queue-dev
```


### Download the library.

> - **Latest Stable Version : 1.1.2.1 [ [Download](http://packetfactory.openwall.net/libnet/dist/libnet.tar.gz) ]**
>
> 		# wget http://packetfactory.openwall.net/libnet/dist/libnet.tar.gz
> - **tar -xvf file.**
>
> 		# tar -zxvf libnet.tar.gz
> - **Create a Directory.**
>
> 		# mkdir /usr/include/libnet
> 		# mkdir /usr/include/win32
> - **Move the file into the directory.**
>
> 		# cd libnet/include/libnet
> 		# mv libnet-macros.h libnet-headers.h /usr/include/libnet
> 		# mv ../win32/config.h /usr/include/win32


### Download the Glog.

> - **Cloning a Git repository.**
>
> 		# git clone https://github.com/google/glog.git
> - **Change from the directory.**
>
> 		# cd glog
> - **Install.**  
> ※ See INSTALL for (generic) installation instructions for C++: basically.
>
> 		# ./autogen.sh && ./configure && make && make install
> 		# ldconfig
> - **Error during installation.**
>   * **autoreconf: not found.**
>
>		  # apt install autoconf
>   * **libtoolize is needed because this package uses Libtool.**
>
>		  # apt install libtool

