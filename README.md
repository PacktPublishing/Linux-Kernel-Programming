


# Linux Kernel Programming

<a href="https://www.packtpub.com/product/linux-kernel-programming/9781789953435?utm_source=github&utm_medium=repository&utm_campaign=9781789953435"><img src="https://static.packt-cdn.com/products/9781789953435/cover/smaller" alt="Linux Kernel Programming" height="256px" align="right"></a>

This is the code repository for [Linux Kernel Programming](https://www.packtpub.com/product/linux-kernel-programming/9781789953435?utm_source=github&utm_medium=repository&utm_campaign=9781789953435), published by Packt.

**Linux Kernel Programming, published by Packt**

## What is this book about?
Linux Kernel Programming is a comprehensive introduction for those new to Linux kernel and module development. This easy-to-follow guide will have you up and running with writing kernel code in next-to-no time. This book uses the latest 5.4 Long-Term Support (LTS) Linux kernel, which will be maintained from November 2019 through to December 2025. By working with the 5.4 LTS kernel throughout the book, you can be confident that your knowledge will continue to be valid for years to come. 

This book covers the following exciting features:
* Write high-quality modular kernel code (LKM framework) for 5.x kernels
* Configure and build a kernel from source
* Explore the Linux kernel architecture
* Get to grips with key internals regarding memory management within the kernel
* Understand and work with various dynamic kernel memory alloc/dealloc APIs
Discover key internals aspects regarding CPU scheduling within the kernel
Gain an understanding of kernel concurrency issues
Learn how to work with key kernel synchronization primitives

If you feel this book is for you, get your [copy](https://www.amazon.com/dp/178995343X) today!

<a href="https://www.packtpub.com/?utm_source=github&utm_medium=banner&utm_campaign=GitHubBanner"><img src="https://raw.githubusercontent.com/PacktPublishing/GitHub/master/GitHub.png" 
alt="https://www.packtpub.com/" border="5" /></a>

## Instructions and Navigations
All of the code is organized into folders. For example, ch2.

The code will look like the following:
```
static int __init miscdrv_init(void)
{
	int ret;
	struct device *dev;
```

**Following is what you need for this book:**
This book is for Linux programmers beginning to find their way with Linux kernel development. Linux kernel and driver developers looking to overcome frequent and common kernel development issues, as well as understand kernel internals, will benefit from this book. A basic understanding of Linux CLI and C programming is required.

With the following software and hardware list you can run all code files present in the book (Chapter 1-13).
### Software and Hardware List
| Chapter | Software required | OS required |
| -------- | ------------------------------------ | ----------------------------------- |
| 1-13 | Oracle VirtualBox 6.1 | Windows and Linux (Any) |

We also provide a PDF file that has color images of the screenshots/diagrams used in this book. [Click here to download it](https://static.packt-cdn.com/downloads/9781789953435_ColorImages.pdf).

### Known Errata
Wrt the PDF doc:

- pg 26:
    - 'sudo apt install git fakeroot ...' ; corrections:
        - change pstree to psmisc
        - to install 'tuna' refer https://tuna.readthedocs.io/en/stable/installation.html 
        - change hexdump to bsdmainutils (the package name)
        - for 'openjdk-14-jre' installation refer https://java.tutorials24x7.com/blog/how-to-install-openjdk-14-on-ubuntu
- pg 99:
    - 'Chapter 9' should be 'Chapter 10'
    - 'Chapter 10' should be 'Chapter 11'
- pg 155:
    - the line '// ch4/helloworld_lkm/hellowworld_lkm.c' has the letter 'w' twice; it should be:
     '// ch4/helloworld_lkm/helloworld_lkm.c'   (thanks to @xuhw21)
- pg 246:
    - 'via the module_parm_cb() macro' should be 'via the module_param_cb() macro'
- pg 291:
    - '(The kernel-mode stack for ' - incomplete sentence; it should be deleted/ignored.
- pg 307:
    - the process view after the sentence '...  and a total of *nine threads*:'
        - the first two columns are shown as 'PID  TGID'; the order is reversed, it should be 'TGID  PID'
- pg 324: in *Figure 7.4*, the third column 'Addr Bits', last 3 rows have errors; the corrections are shown here:

                          `AB    VM-split`

`x86_64:  5 : 56 --> 57 :  64PB:64PB  : corrected (allows for total of 128 PB)`

`Aarch64: 3 : 39 --> 40 : 512G:512G : corrected (allows for total of 1024 GB = 1 TB)`

`Aarch64: 4 : 48 --> 49 : 256T:256T  : corrected (allows for total of 512 T)`

- pg 385:
   - 'On high-end enterprise server class systems running the Itanium (IA-64) processor, MAX_ORDER can be as high as 17 (implying a
largest chunk size on order (17-1), thus of 216 = 65,536 pages = 512 MB chunks of physically contiguous RAM on order 16 of the freelists, for
a 4 KB page size).'
should be:
'On high-end enterprise server class systems running the Itanium (IA-64) processor, MAX_ORDER can be as high as 17 (implying a
largest chunk size on order (17-1), thus of 216 = 65,536 pages = *256 MB* chunks of physically contiguous RAM on order 16 of the freelists, for
a 4 KB page size).'

- pg 388:
    - '... the next available memory chunk is on order 7, of size 256 KB.' should be: '... the next available memory chunk is on order 6, of size 256 KB.

- pg 656: the line
  'In place of atomic64_dec_if_positive(), use atomic64_dec_if_positive()'
  should be
  'In place of atomic_dec_if_positive(), use atomic64_dec_if_positive()'
(thanks to @xuhw21)

- pg 661:
    - '... there is a incorrect reference regarding a folder chp15/kthread_simple/kthread. The correct reference should be ch5/kthread_simple/kthread in part 2 of the book's GitHub [[Repository]](https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2)

- pg 665:
    - '...In info/Tip: 
```
"The material in this section assumes you have at least a base understanding of accessing 
peripheral device (chip) memory and registers; we have covered this in detail in 
Chapter 13, Working with Hardware I/O Memory. Please ensure that you understand it before moving forward. 
```
should be
```
"The material in this section assumes you have at least a base understanding of accessing 
peripheral device (chip) memory and registers; we have covered this in detail in 
Linux Kernel Programming Part 2 - Chapter 3, Working with Hardware I/O Memory. 
Please ensure that you understand it before moving forward."
```
* pg 183 : **Wiring to the console** _should be_ **Writing to the console**

### Related products
* Mastering Linux Device Driver Development [[Packt]](https://www.packtpub.com/product/Mastering-Linux-Device-Driver-Development/9781789342048) [[Amazon]](https://www.amazon.com/Mastering-Linux-Device-Driver-Development/dp/178934204X)

* Hands-On System Programming with Linux [[Packt]](https://www.packtpub.com/in/networking-and-servers/hands-system-programming-linux?utm_source=github&utm_medium=repository&utm_campaign=9781788998475) [[Amazon]](https://www.amazon.com/Hands-System-Programming-Linux-programming-ebook/dp/B079RKKKJ7/ref=sr_1_1?dchild=1&keywords=Hands-On+System+Programming+with+Linux&qid=1614057025&s=books&sr=1-1)

## Get to Know the Author
**Kaiwan N Billimoria**
taught himself BASIC programming on his dad's IBM PC back in 1983. He was programming in C and Assembly on DOS until he discovered the joys of Unix, and by around 1997, Linux!

Kaiwan has worked on many aspects of the Linux system programming stack, including Bash scripting, system programming in C, kernel internals, device drivers, and embedded Linux work. He has actively worked on several commercial/FOSS projects. His contributions include drivers to the mainline Linux OS and many smaller projects hosted on GitHub. His Linux passion feeds well into his passion for teaching these topics to engineers, which he has done for well over two decades now. He's also the author of Hands-On System Programming with Linux. It doesn't hurt that he is a recreational ultrarunner too.

## Other books by the authors
* [Hands-On System Programming with Linux](https://www.packtpub.com/in/networking-and-servers/hands-system-programming-linux?utm_source=github&utm_medium=repository&utm_campaign=9781788998475)
  
* * * * 
### Download a free PDF

 <i>If you have already purchased a print or Kindle version of this book, you can get a DRM-free PDF version at no cost.<br>Simply click on the link to claim your free PDF.</i>
<p align="center"> <a href="https://packt.link/free-ebook/9781789953435">https://packt.link/free-ebook/9781789953435 </a> </p>
