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

- **[Official Kernel Documentation](https://www.kernel.org/doc/html/latest/index.html)** : the modern Linux kernel documentation is very well written and presented here
	- ["2. How the development process works"](https://www.kernel.org/doc/html/latest/process/2.Process.html#how-the-development-process-works)
	- [The Linux kernel build system - kbuild - infrastructure](https://www.kernel.org/doc/Documentation/kbuild/)
- Git
    - [Firstly, A Gentle Introduction to Version Control](http://chronicle.com/blogs/profhacker/a-gentle-introduction-to-version-control/23064)
    - [Resources to Learn Git](http://try.github.io/)
    - [The “official” git website](http://git-scm.com/)
    - [Try Git in your browser – interactive tutorial!](https://try.github.io/levels/1/challenges/1)
    - [Most commonly used git tips and tricks on github](https://github.com/git-tips/tips)
    - [The "Git Magic" book, by Ben Lynn (PDF)](http://www-cs-students.stanford.edu/~blynn/gitmagic/book.pdf)
    - [Aha! Moments when learning Git](http://betterexplained.com/articles/aha-moments-when-learning-git/)
    - [How not to be afraid of Git anymore](https://medium.freecodecamp.org/how-not-to-be-afraid-of-git-anymore-fe1da7415286)
    - [Getting Git Right, Atlassian.com](https://www.atlassian.com/git/) 
    - [... and many more](https://www.google.com/search?q=learning+to+use+git)

- [Linux Kernel Release Model, Greg Kroah-Hartman, Feb 2018](http://kroah.com/log/blog/2018/02/05/linux-kernel-release-model/)
- ["LTSI	Project	update - Long Term Support Initiative"](https://events.linuxfoundation.org/wp-content/uploads/2017/11/Using-Linux-for-Long-Term-Community-Status-and-the-Way-We-Go-OSS-Tsugikazu-Shibata.pdf) - slide deck, Open Source Summit Japan, June 2018
- Linux kernel upstream development
    - [Working with the kernel development community](https://www.kernel.org/doc/html/latest/process/index.html#working-with-the-kernel-development-community)
    - [Working with linux-next](https://www.kernel.org/doc/man-pages/linux-next.html)

- [Linux kernel versions with detailed notes on new features, kernelnewbies](https://kernelnewbies.org/LinuxVersions)
- Licensing
	-   [The differences between the GPL, LGPL and the BSD](https://fosswire.com/post/2007/04/the-differences-between-the-gpl-lgpl-and-the-bsd/)
	- [Proprietary loadable kernel modules, LWN](https://lwn.net/Articles/434491/)
	- [Choose an open source license](https://choosealicense.com/)
	- [Legal Risks of Open Source – GPL/Linux Loadable Kernel Modules](http://sourceauditor.com/blog/legal-risks-of-open-source-gpllinux-loadable-kernel-modules/)
	- [Linux kernel licensing rules](https://www.kernel.org/doc/html/latest/process/license-rules.html#linux-kernel-licensing-rules)
	- [MIT vs. BSD vs. Dual License](https://softwareengineering.stackexchange.com/questions/121998/mit-vs-bsd-vs-dual-license)
	
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
- ['How to get printk format specifiers right', Linux kernel doc](https://www.kernel.org/doc/html/latest/core-api/printk-formats.html?highlight=dma)
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

## Chapter 3, Kernel and Memory Management Internals -Essentials
- Book: [Hands-On System Programming with Linux", Kaiwan N Billimoria, Packt](https://www.packtpub.com/networking-and-servers/hands-system-programming-linux). Prerequisites to this chapter (essential reading, really):
	- Ch 1 : Linux System Architecture
	- Ch 2 : Virtual Memory
- Books: Linux kernel memory management Internals
	- ['Professional Linux Kernel Architecture', Mauerer, Wrox Press](https://www.amazon.in/Professional-Linux-Kernel-Architecture-Programmer/dp/0470343435/ref=sr_1_1?ie=UTF8&qid=1547525512&sr=8-1&keywords=Professional%20Linux%20Kernel%20Architecture%27)
	- ['Understanding the Linux Virtual Memory Manager', Mel Gorman (Bruce Perens Open Source)](https://www.amazon.in/Understanding-Virtual-Memory-Manager-Perens/dp/0131453483/ref=sr_1_1?ie=UTF8&qid=1547529951&sr=8-1&keywords=mel%20gorman)

-   CPU ABI (Application Binary Interface) documentation, blog article : 'APPLICATION BINARY INTERFACE (ABI) DOCS AND THEIR MEANING' :  [https://kaiwantech.wordpress.com/2018/05/07/application-binary-interface-abi-docs-and-their-meaning/](https://kaiwantech.wordpress.com/2018/05/07/application-binary-interface-abi-docs-and-their-meaning/)
- Deep detail on MM and paging on the Intel processors can be found in their excellent manuals here (Ch 4, Paging): [Intel® 64 and IA-32 Architectures
Software Developer’s Manual. Volume 3 (3A, 3B & 3C): System Programming Guide](https://software.intel.com/en-us/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-3a-3b-3c-and-3d-system-programming-guide)
	- [All Intel 64 and IA-32 architecture and programming environment manuals](https://software.intel.com/en-us/articles/intel-sdm)
- Blog article: ['Some Tricks used by the Linux kernel'](https://geeksww.com/tutorials/operating_systems/linux/tips_and_tricks/some_tricks_used_by_the_linux_kernel.php)
- [un]likely() macros:
	- [likely()/unlikely() macros in the Linux kernel - how do they work? What's their benefit?](https://stackoverflow.com/questions/109710/likely-unlikely-macros-in-the-linux-kernel-how-do-they-work-whats-their)
	- [Why do we use __builtin_expect when a straightforward way is to use if-else](https://stackoverflow.com/questions/7346929/why-do-we-use-builtin-expect-when-a-straightforward-way-is-to-use-if-else)

- vsyscall, vdso and VVAR
    - ["On vsyscalls and the vDSO"](https://lwn.net/Articles/446528/), LWN, Jon Corbet, June 2011
    - ["Are system calls the only way to interact with the Linux kernel from user land?", StackExchange, Apr 2014](https://unix.stackexchange.com/questions/124928/are-system-calls-the-only-way-to-interact-with-the-linux-kernel-from-user-land)

- Kernel memory optimization techniques
	- [THP (Transparent Huge Pages)](https://www.kernel.org/doc/html/latest/admin-guide/mm/transhuge.html#admin-guide-transhuge)
	- [KSM (Kernel Samepage Merging)](https://www.kernel.org/doc/html/latest/admin-guide/mm/ksm.html)
- ['Virtual Memory and Linux', PDF, Matt Porter, Alan Ott](https://elinux.org/images/b/b0/Introduction_to_Memory_Management_in_Linux.pdf)


## Chapter 4, Kernel Memory Allocation for Module Authors

- Book: ['Hands-On System Programming with Linux', Kaiwan N Billimoria, Packt](https://www.packtpub.com/networking-and-servers/hands-system-programming-linux). Prerequisites to this chapter (essential reading, really):
	- Ch 1 : Linux System Architecture
	- Ch 2 : Virtual Memory

- Information regarding GFP flags internal usage, etc: ['Memory management when failure is not an option', LWN, Mar 2015](https://lwn.net/Articles/635354/)

- CPU Caches
    - ["How L1 and L2 CPU Caches Work, and Why They’re an Essential Part of Modern Chips", Joel Hruska, Aug 2018](http://www.extremetech.com/extreme/188776-how-l1-and-l2-cpu-caches-work-and-why-theyre-an-essential-part-of-modern-chips)
    - An example diagram of CPU (cache) hierarchy: ["Memory hierarchy of an AMD Bulldozer server"](https://en.wikipedia.org/wiki/CPU_cache#/media/File:Hwloc.png)

- Slab layer - performance, etc
    -   ['Toward a more efficient slab allocator', LWN, Jon Corbet, Jan 2015](https://lwn.net/Articles/629152/)
    -   Blog article ['INTERESTING NUMBERS'](https://kaiwantech.wordpress.com/2015/05/01/interesting-numbers/); see the section on *Networking* for some information on how the network subsystem has time critical code paths (and the resource links that follow)
- ["GNUPLOT 4.2 - A Brief Manual and Tutorial", Duke University](https://people.duke.edu/~hpgavin/gnuplot.html)
- ['Object-oriented design patterns in the kernel, part 1', Neil Brown, LWN, June 2011](https://lwn.net/Articles/444910/)
- ['kvmalloc()', Jon Corbet, LWN, Jan 2017](https://lwn.net/Articles/711653/)
- ['The "too small to fail" memory-allocation rule', Jon Corbet, LWN, Dec 2014](https://lwn.net/Articles/627419/)
- Blog article: ['linux slab poisoning 101', Mar 2009](http://www.chizang.net/alex/2009/03/11/linux-slab-poisoning-101/)
- ['The slab and protected-memory allocators', LWN, May 2018](https://lwn.net/Articles/753154/)
- DMA - Direct Memory Access
    - Books: 'Essential Linux Device Drivers', S Venkateswaran, and 'Linux Device Drivers', Rubini, Corbet & Hartman
    - ['DMAEngine documentation', Linux kernel](https://www.kernel.org/doc/html/latest/driver-api/dmaengine/index.html#dmaengine-documentation)
    - ['A deep dive into CMA', LWN, Mar 2012](https://lwn.net/Articles/486301/)
    - A quite simple and interesting example of a “DMA test application” device driver is from Xilinx (for it's Zynq PL330 DMA controller); [wiki page here](http://www.wiki.xilinx.com/Zynq+Linux+pl330+DMA)
    - [Q&A on SO: Linux DMA: Using the DMAengine for scatter-gather transactions, May 2016](http://stackoverflow.com/questions/37119332/linux-dma-using-the-dmaengine-for-scatter-gather-transactions)
- VM Overcommit
    - [Quora: What are the disadvantages of disabling memory overcommit in Linux?](https://www.quora.com/What-are-the-disadvantages-of-disabling-memory-overcommit-in-Linux)
    - [Linux kernel documentation](https://www.kernel.org/doc/Documentation/vm/overcommit-accounting)
    - Blog article: ['Virtual memory settings in Linux - The Problem with Overcommit'](http://engineering.pivotal.io/post/virtual_memory_settings_in_linux_-_the_problem_with_overcommit/)
    - RHEL specific but useful: ['Interpreting /proc/meminfo and free output for Red Hat Enterprise Linux 5, 6 and 7'](https://access.redhat.com/solutions/406773)
- b


## Ch 11 : Kernel Debug

- Blog article (medium): ['A Short Guide to Kernel Debugging
A story about finding a kernel bug on a production system', Wester, Oct 2015](https://medium.com/square-corner-blog/a-short-guide-to-kernel-debugging-e6fdbe7bfcdf)
- a



[\[Top\]](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#further-reading)

[End of Doc]
