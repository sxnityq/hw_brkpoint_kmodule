# hw_brkpont_module
test assignment for INANGO
### Introductiory
This is simple hardware breakpoint kernel module for kernel 6.6.58-gentoo-dist that sets hardware breakpoint and triggers "triggered" function when someone tries to read and write for that specific address.

### TODO
1) yocto recipe for qemux86_64
2) ability to load module with params
3) some sort of addr checking

### HOW TO USE

I set hardcoded address. That was the worst decision, but it will be fixed later.
So in order to test sysfs modification and module work in general lets begin with answering which address to pick and from where

/proc/kallsyms - this file contains all information about different variables, dwelled in kernel space. lets get address that is related to /dev/random.

UNDER THE ROOT!
```cat /proc/kallsyms | grep " D " | grep random_fops```
remember this addr

Load the module
```
make
insmod mmodule.ko
```

change address via sysfs
```/sys/kernel/mmodule - kobject
/sys/kernerl/mmodule/addr - is file that stores addr
echo <addr from /proc/kallsyms> > /sys/kernel/mmodule/addr
```

in new window 
```
dmesg -w
```
in another window 
```
cat /dev/random
and CTRL^C
```

now it should work