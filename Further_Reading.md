# Further Reading
*For the book "Linux Kernel Development Cookbook", Kaiwan N Billimoria, Packt (2019).*

*Book GitHub repo*: https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook

Here, we present, in a chapter-wise format, additional online as well as book references. We feel that these will help, you, the reader, in digging deeper into topics, or gaining additional views/information, or both. We definitely recommend you browse through the provided references. Also, you will find that some resources are repeated across chapters; we leave it this way so that the reader can gain all required resources in one place (and chapter-wise).
(Obviously, these reference articles/blogs/etc are provided "as-is"; we cannot vouch for their correctness or validity).

For the reader's convenience, below are chapter-wise links to the *Further Reading* notes:

 - [Ch 1, Building the Kernel From Source](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#chapter-1-building-the-linux-kernel-from-source--further-reading) 
 - [Ch 2. Writing your first Kernel Module - the LKM Framework](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#chapter-2-writing-your-first-kernel-module-the-lkm-framework-further-reading)
 - Ch 3
 - Ch 4. Kernel Memory Allocation for Module Authors
 - Ch 5

- *Online Chapters*
	 - [A: File IO Essentials](https://github.com/PacktPublishing/Hands-on-System-Programming-with-Linux/blob/master/Further_reading.md#a-file-io-essentials--further-reading)


## Chapter 1, Building the Linux Kernel From Source : Further Reading

- Official Kernel Documentation : the modern Linux kernel documentation is very well written and [available online here](https://www.kernel.org/doc/html/latest/index.html)
	- ["2. How the development process works"](https://www.kernel.org/doc/html/latest/process/2.Process.html#how-the-development-process-works)
	- [The Linux kernel build system - kbuild - infrastructure](https://www.kernel.org/doc/Documentation/kbuild/)
- [Linux Kernel Release Model, Greg Kroah-Hartman, Feb 2018](http://kroah.com/log/blog/2018/02/05/linux-kernel-release-model/)
- [Linux kernel versions with detailed notes on new features, kernelnewbies](https://kernelnewbies.org/LinuxVersions)
- Licensing
	-   [The differences between the GPL, LGPL and the BSD](https://fosswire.com/post/2007/04/the-differences-between-the-gpl-lgpl-and-the-bsd/)
	- [Proprietary loadable kernel modules, LWN](https://lwn.net/Articles/434491/)
	- [Choose an open source license](https://choosealicense.com/)
	- [Legal Risks of Open Source – GPL/Linux Loadable Kernel Modules](http://sourceauditor.com/blog/legal-risks-of-open-source-gpllinux-loadable-kernel-modules/)
	- [Linux kernel licensing rules](https://www.kernel.org/doc/html/latest/process/license-rules.html#linux-kernel-licensing-rules)
- [Configuring the kernel](https://www.kernel.org/doc/html/latest/admin-guide/README.html#configuring-the-kernel)
- Detailed article: [Exploring the Linux kernel: The secrets of Kconfig/kbuild](https://opensource.com/article/18/10/kbuild-and-kconfig)
- Initrd / initramfs article: [Opening and modifying the initrd](http://www.alexonlinux.com/opening-and-modifying-the-initrd)
- [GRUB: How do I change the default boot kernel](https://askubuntu.com/questions/216398/set-older-kernel-as-default-grub-entry)
- Article: [Linux Kernel Cross Compilation](https://gts3.org/2017/cross-kernel.html)
- [SEALS - Simple Embedded ARM Linux System - project](https://github.com/kaiwan/seals)
	- - [Wiki pages](https://github.com/kaiwan/seals/wiki)
- [Raspberry Pi kernel build: official doc](https://www.raspberrypi.org/documentation/linux/kernel/building.md)

## Chapter 2, Writing your first Kernel Module-the LKM framework: Further Reading

- [Official Kernel Documentation : kernel parameters](https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html)
- ['What are the __init* and __exit* macros ?', kernelnewbies](https://kernelnewbies.org/FAQ/InitExitMacros)
- Systemd
	-   Article: ['Systemd Essentials: Working with Services, Units, and the Journal', DigitalOcean](https://www.digitalocean.com/community/tutorials/systemd-essentials-working-with-services-units-and-the-journal)
	- Article: ['Understanding Systemd Units and Unit Files', DigitalOcean](https://www.digitalocean.com/community/tutorials/understanding-systemd-units-and-unit-files) 
	- [Systemd Documentation](http://0pointer.de/blog/projects/systemd-docs.html)
	- [Systemd FAQs](https://www.freedesktop.org/wiki/Software/systemd/FrequentlyAskedQuestions/)
- [Linux console (on Wikipedia)](https://en.wikipedia.org/wiki/Linux_console)
- [Debugging by printing, eLinux](https://elinux.org/Debugging_by_printing)
- Blog article: ["MAKEFILES TUTORIAL", Liran B.H.](http://devarea.com/makefiles-tutorial/)
- Article: [WORKING ON THE CONSOLE WITH THE RASPBERRY PI, kaiwanTECH](https://kaiwantech.wordpress.com/2018/12/16/working-on-the-console-with-the-raspberry-pi/)
- Article: ['Improving EXPORT_SYMBOL()', Jon Corbet, Feb 2016, LWN](https://lwn.net/Articles/674303/)
- StackOverflow Q&A: ["How to prevent “error: 'symbol' undeclared here” despite EXPORT_SYMBOL in a Linux kernel module?"](https://stackoverflow.com/questions/6670589/how-to-prevent-error-symbol-undeclared-here-despite-export-symbol-in-a-linu?noredirect=1&lq=1)
- Linux kernel static analysis
	- Blog article: ["Static analysis on the Linux kernel"](http://smackerelofopinion.blogspot.com/2017/09/static-analysis-on-linux-kernel.html)
	- PDF: ["Static code checking In the Linux kernel", Arnd Bergmann, Linaro](https://elinux.org/images/d/d3/Bargmann.pdf)
	- Thesis paper: ["Automatic Bug-finding Techniques for Large Software Projects", Jiri Slaby](https://www.fi.muni.cz/~xslaby/sklad/dis.pdf)
- Blog article: ["LINUX KERNEL DEVELOPMENT – KERNEL MODULE PARAMETERS", Liran B.H.](http://devarea.com/linux-kernel-development-kernel-module-parameters/)
- Useful Arch Linux Wiki on kernel module - gathering info, auto-loading, aliasing, blacklisting, etc: ["Kernel module"](https://wiki.archlinux.org/index.php/Kernel_module)
- StackOverflow Q&A: ["Where does modprobe load a driver that udev requests?"](https://unix.stackexchange.com/questions/330186/where-does-modprobe-load-a-driver-that-udev-requests)
- DKMS: Dynamic Kernel Module Support
	- AskUbuntu: ["What does DKMS do? How do I use it?"](https://askubuntu.com/questions/408605/what-does-dkms-do-how-do-i-use-it)
	- dkms(8) man page](https://linux.die.net/man/8/dkms)
- Linux Kernel Security
	- The [Kernel Self Protection Project](https://kernsec.org/wiki/index.php/Kernel_Self_Protection_Project)  (KSPP, Kees Cook)
	- Arch Linux wiki: ["Security / Kernel hardening"](https://wiki.archlinux.org/index.php/security#Kernel_hardening)
	- ["The status of kernel hardening", LWN, Nov 2016](https://lwn.net/Articles/705262/)
	- Blog article: ["Effectively bypassing kptr_restrict on Android"](http://bits-please.blogspot.com/2015/08/effectively-bypassing-kptrrestrict-on.html)
	- Quora: ["What are some of the best resources for Kernel exploitation on Linux?"](https://www.quora.com/What-are-some-of-the-best-resources-for-Kernel-exploitation-on-Linux/answer/Kaiwan-N-Billimoria)
	- [Linux Kernel Exploitation GitHub blog - a long list of kernel exploitation resources](https://github.com/xairy/linux-kernel-exploitation)
	- ["Hardened GNU/Linux - Linux kernel mitigation checklist"](https://hardenedlinux.github.io/system-security/2016/12/13/kernel_mitigation_checklist.html)
	- ["Loading signed kernel modules", LWN, Dec 2011](https://lwn.net/Articles/470906/)
- [The Eudyptula Challenge](http://eudyptula-challenge.org/): "The Eudyptula Challenge was a series of programming exercises for the Linux kernel, that started from a very basic "Hello world" kernel module, moving on up in complexity to getting patches accepted into the main Linux kernel source tree." It's really cool but NOT active as of this writing though.

## Chapter 4, Kernel Memory Allocation for Module Authors
- Book: [Hands-On System Programming with Linux", Kaiwan N Billimoria, Packt](https://www.packtpub.com/networking-and-servers/hands-system-programming-linux)
- Linux kernel memory management Internals - Books
	- ['Professional Linux Kernel Architecture', Mauerer, Wrox Press](https://www.amazon.in/Professional-Linux-Kernel-Architecture-Programmer/dp/0470343435/ref=sr_1_1?ie=UTF8&qid=1547525512&sr=8-1&keywords=Professional%20Linux%20Kernel%20Architecture%27)
	- ['Understanding the Linux Virtual Memory Manager', Mel Gorman (Bruce Perens Open Source)](https://www.amazon.in/Understanding-Virtual-Memory-Manager-Perens/dp/0131453483/ref=sr_1_1?ie=UTF8&qid=1547529951&sr=8-1&keywords=mel%20gorman)
	- 
- [un]likely() macros:
	- [likely()/unlikely() macros in the Linux kernel - how do they work? What's their benefit?](https://stackoverflow.com/questions/109710/likely-unlikely-macros-in-the-linux-kernel-how-do-they-work-whats-their)
	- [Why do we use __builtin_expect when a straightforward way is to use if-else](https://stackoverflow.com/questions/7346929/why-do-we-use-builtin-expect-when-a-straightforward-way-is-to-use-if-else)
- b

	 


[\[Top\]](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#further-reading)

[End of Doc]
