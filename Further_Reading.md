# Further Reading
*For the book "Learn Linux Kernel Development", Kaiwan N Billimoria, Packt (2019).*

*Book GitHub repo*: https://github.com/PacktPublishing/Learn-Linux-Kernel-Development

Here, we present, in a chapter-wise format, additional online as well as book references. We feel that these will help, you, the reader, in digging deeper into topics, or gaining additional views/information, or both. We definitely recommend you browse through the provided references. Also, you will find that some resources are repeated across chapters; we leave it this way so that the reader can gain all required resources in one place (and chapter-wise).
(Obviously, these reference articles/blogs/etc are provided "as-is"; we cannot vouch for their correctness or validity).

For the reader's convenience, below are chapter-wise links to the *Further Reading* notes:

 - [Ch 1, Kernel workspace setup](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#chapter-1-kernel-workspace-setup--further-reading)
 - [Ch 2, Building the Kernel From Source](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#chapter-1-building-the-linux-kernel-from-source--further-reading) 
 - [Ch 4. Writing your first Kernel Module - the LKM Framework](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#chapter-2-writing-your-first-kernel-module-the-lkm-framework-further-reading)
 - Ch 3
 - Ch 4. Kernel Memory Allocation for Module Authors
 - Ch 5

- *Online Chapters*
	 - [A: File IO Essentials](https://github.com/PacktPublishing/Hands-on-System-Programming-with-Linux/blob/master/Further_reading.md#a-file-io-essentials--further-reading)


## Chapter 1, Kernel Development Workspace Setup - Further Reading

- [Emulating Raspberry Pi on Linux](http://embedonix.com/articles/linux/emulating-raspberry-pi-on-linux/)

- [qemu-rpi-kernel, GitHub](https://github.com/dhruvvyas90/qemu-rpi-kernel/wiki)
- Code browsers
    - [Ctags Tutorial](https://courses.cs.washington.edu/courses/cse451/10au/tutorials/tutorial_ctags.html)
    - [The Vim/Cscope tutorial](http://cscope.sourceforge.net/cscope_vim_tutorial.html)



## Chapter 2 and Ch 3, Building the Linux Kernel From Source - Further Reading

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
    - Blog artcile: ['Kernel Patch Submission tutorial', Saurabh Singh Sengar](https://saurabhsengarblog.wordpress.com/2015/11/26/kernel-patch-submission-tutorial/)
    - [Working with linux-next](https://www.kernel.org/doc/man-pages/linux-next.html)

- [Linux kernel versions with detailed notes on new features, kernelnewbies](https://kernelnewbies.org/LinuxVersions)
- Fun: [Occurences of words in the Linux kernel source code over time](https://www.vidarholen.net/contents/wordcount/#hack*,workaround*,todo,fixme)

- Licensing
	-   [The differences between the GPL, LGPL and the BSD](https://fosswire.com/post/2007/04/the-differences-between-the-gpl-lgpl-and-the-bsd/)
	- [Proprietary loadable kernel modules, LWN](https://lwn.net/Articles/434491/)
	- [Choose an open source license](https://choosealicense.com/)
	- [Legal Risks of Open Source – GPL/Linux Loadable Kernel Modules](http://sourceauditor.com/blog/legal-risks-of-open-source-gpllinux-loadable-kernel-modules/)
	- [Linux kernel licensing rules](https://www.kernel.org/doc/html/latest/process/license-rules.html#linux-kernel-licensing-rules)
	- [MIT vs. BSD vs. Dual License](https://softwareengineering.stackexchange.com/questions/121998/mit-vs-bsd-vs-dual-license)
	
- [Configuring the kernel](https://www.kernel.org/doc/html/latest/admin-guide/README.html#configuring-the-kernel)
- Detailed article: [Exploring the Linux kernel: The secrets of Kconfig/kbuild](https://opensource.com/article/18/10/kbuild-and-kconfig)
- Initramfs (initrd):
    -  [Opening and modifying the initrd](http://www.alexonlinux.com/opening-and-modifying-the-initrd)
    - [Initramfs (on Ubuntu wiki)](https://wiki.ubuntu.com/Initramfs)
    - informative: [debian bug report: intel-microcode: breaks initrd for newer kernels, Sept 2014](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=760765)
- [GRUB: How do I change the default boot kernel](https://askubuntu.com/questions/216398/set-older-kernel-as-default-grub-entry)
- Article: [Linux Kernel Cross Compilation](https://gts3.org/2017/cross-kernel.html)
- [SEALS - Simple Embedded ARM Linux System - project](https://github.com/kaiwan/seals)
	- - [Wiki pages](https://github.com/kaiwan/seals/wiki)
- Building the (usual 32-bit) kernel for the Raspberry Pi device
    - [Raspberry Pi kernel build: official doc](https://www.raspberrypi.org/documentation/linux/kernel/building.md)
    - Building a **64-bit** kernel for the Raspberry Pi
        - blog article: ['BUILD A 64-BIT KERNEL FOR YOUR RASPBERRY PI 3', Dec 2016](https://devsidestory.com/build-a-64-bit-kernel-for-your-raspberry-pi-3/)
        - blog article: ['Tutorial: How (and why!) to set up a 64-bit kernel, 32-bit Raspbian host OS, 64-bit nspawn Debian guest OS RPi3 system', Jan 2019](https://www.raspberrypi.org/forums/viewtopic.php?f=56&t=232415&sid=146ea82cef8d235c1eaf64a12a8c3427)
        - On GitHub: ['Bootable RPi3 image with 64-bit kernel, 32-bit Raspbian Stretch host OS, 64-bit Debian Buster guest OS in nspawn container', Sakaki, Apr 2019](https://github.com/sakaki-/raspbian-nspawn-64)
        - Interesting, an [Ubuntu 18.04 server on Aarch64 Raspberry Pi](https://wiki.ubuntu.com/ARM/RaspberryPi); simplest way to get a 64-bit Raspberry Pi kernel and rootfs in a single image which can be burned onto the SD card

## Chapter 4, Writing your first Kernel Module-LKMs Part 1 - Further Reading

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

## Chapter 5, Writing your first Kernel Module-LKMs Part 2 - Further Reading

- Blog article: [WORKING ON THE CONSOLE WITH THE RASPBERRY PI, kaiwanTECH](https://kaiwantech.wordpress.com/2018/12/16/working-on-the-console-with-the-raspberry-pi/)

- Licensing
    - [Being honest with MODULE_LICENSE, Jon Corbet, LWN, Apr 2004](https://lwn.net/Articles/82305/)
    - [Multi-licensing](https://en.wikipedia.org/wiki/Multi-licensing)
    - [Dual Licensing: Having Your Cake and Eating It Too](https://www.linuxinsider.com/story/38172.html), LinuxInsider
    - [GPL License FAQs](https://www.gnu.org/licenses/gpl-faq.html)
    - Selecting a license
        - [Open Source Licenses Comparison -Guide](https://itsfoss.com/open-source-licenses-explained/), Leroux, Nov 2019
        - [Choose an open source license](https://choosealicense.com/)

- Arithmetic Overflow
    - An excellent must-read whitepaper: ['Understanding Integer Overflow in C/C++', Dietz, et al, June 2012](http://www.cs.utah.edu/~regehr/papers/overflow12.pdf)
    - ['Catch and compute overflow during multiplication of two large integers', StackOverflow](https://stackoverflow.com/questions/1815367/catch-and-compute-overflow-during-multiplication-of-two-large-integers)
    - An example of checking for arithmetic overflow in 'real-world' code can be found [in the git codebase](https://github.com/git/git); see the [wrapper.c code](https://github.com/git/git/blob/master/wrapper.c) for usage examples
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

## Chapter 6, Essentials of Kernel and Memory Management Internals - Further Reading
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


## Ch 7 and Ch 8, Kernel Memory Allocation for Module Authors - Further Reading

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

## Ch 9 and Ch 10 : The CPU Scheduler - Further Reading

- Detailed PDF: ['A complete guide to Linux process scheduling', Nikita Ishkov, Feb 2015](https://trepo.tuni.fi/bitstream/handle/10024/96864/GRADU-1428493916.pdf)
- Ftrace
    - ['ftrace: trace your kernel functions!', Julia Evans, Mar 2017](https://jvns.ca/blog/2017/03/19/getting-started-with-ftrace/)

- Control Groups (cgroups)
    - Good introductory article (focussed on cgroups1): ['Control Groups in Linux', Feb 2017](http://blog.brew.com.hk/control-groups-in-linux/)
    - ['Understanding the new control groups API', Rami Rosen, LWN, Mar 2016](https://lwn.net/Articles/679786/)
    - ['Control Group v2' : Linux kernel 'official' documentation](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#control-group-v2)
    - man page (informational): ['cgroups - Linux control groups'](http://man7.org/linux/man-pages/man7/cgroups.7.html) (man 7 cgroups)
    - RedHat System Design Guide (RHEL 8); focus on cgroups v1: ['SETTING LIMITS FOR APPLICATIONS'](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/system_design_guide/setting-limits-for-applications_system-design-guide)
    - FB: Includes an interesting case study on how Facebook uses Linux's cgroups2 to perform equitable resource distribution on (some of) it's servers: ['Maximizing Resource Utilization with cgroup2'](https://facebookmicrosites.github.io/cgroup2/docs/overview.html)
        - ['Creating and organizing cgroups'](https://facebookmicrosites.github.io/cgroup2/docs/create-cgroups.html)
    - Blog article, examples of using cgroups v1 for CPU and memory bandwidth control: ['Linux Virtualization : Resource throttling using cgroups'](https://www.geeksforgeeks.org/linux-virtualization-resource-throttling-using-cgroups/)

- Hard Real-Time and Linux as an RTOS
-   - ['Inside Real-Time Linux', Feb 2017](https://www.linux.com/news/event/elce/2017/2/inside-real-time-linux)
-     - ['The Road to Real Time Linux', Steven Rostedt, Mar 2017](https://blogs.vmware.com/opensource/2017/03/16/road-real-time-linux/)
-     - [VDC Survey on real-time response : The Embedded Muse, 341, 03 Jan 2018, Jack Ganssle](http://www.ganssle.com/tem/tem341.html#article3)
-     - Blog article: ['Algorithm time complexity and big O notation'](https://medium.com/@StueyGK/algorithm-time-complexity-and-big-o-notation-51502e612b4d)
-     - Book: *Automate This: How Algorithms Came to Rule Our World, Christopher Steiner*
-     - Older wiki site on *preempt_rt*; it's in the process of being migrated to the new documentation for RTL; it still has very useful HOWTO guides, articles, etc: ['Real-Time Linux Wiki'](https://rt.wiki.kernel.org/index.php/Main_Page) 
-     - [RTL – Real-Time Linux, using Linux as an RTOS:
The RTL Collaborative Project](https://wiki.linuxfoundation.org/realtime/rtl/start)
-   - [RTL Real-Time documentation](https://wiki.linuxfoundation.org/realtime/documentation/start)
-   - [HOWTO: RTOS and RT Applications](https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/application_base)
-   - Interesting: "... compares the load results of the Ubuntu 18.04 LTS generic kernel 4.15.0-22-generic, the Ubuntu low-latency kernel 4.15.0-22-lowlatency and an Ubuntu Linux kernel patched to be fully preemptive 4.16.0-rt4-PREEMPT_RT_FULL as well as preemptive with low-latency 4.16.12-rt4+REEMPT_LL" : ['Low latency and real-time kernels for telco and NFV', Ubuntu, Oct 2018](https://ubuntu.com/blog/low-latency-real-time-kernels-telco-nfv)
-   - ['Real-time Linux communications - An evaluation of the Linux communication stack for real-time robotic applications', Sept 2018](https://hackernoon.com/real-time-linux-communications-2faabf31cf5e)
-   - ['The real-time linux kernel: A survey on Preempt_RT', Feb 2019](https://www.researchgate.net/publication/331290349_The_real-time_linux_kernel_A_survey_on_Preempt_RT)
-   - Detailed slides on cyclictest, good for understanding latency and it's measurement: ['Using and Understanding the Real-Time Cyclictest Benchmark', Rowand, Oct 2013](https://events.static.linuxfound.org/sites/events/files/slides/cyclictest.pdf)
-   - ['Intro to Real-Time Linux for Embedded Developers', an interview with Steven Rostedt](https://www.linuxfoundation.org/blog/2013/03/intro-to-real-time-linux-for-embedded-developers/)


- The still amazing and very relevant book on software engineering: *'The Mythical Man Month: Essays on Software Engineering'*, Frederick P Brooks, 1975, 1995
    - [On Amazon (Anniversary Edition)](https://www.amazon.com/Mythical-Man-Month-Anniversary-Software-Engineering-ebook/dp/B00B8USS14/ref=sr_1_1?crid=3R2CNHTX5LYIC&keywords=mythical+man+month&qid=1562226712&s=digital-text&sprefix=mythical+%2Cdigital-text%2C376&sr=1-1)
    - [Quotes](https://en.wikiquote.org/wiki/Fred_Brooks)




## Ch 11 : Writing a simple Misc Character Device Driver - Further Reading

- Linux Device Drivers
    - Books:
        - ['Essential Linux Device Drivers', Sreekrishnan Venkateswaran, Pearson](https://www.amazon.in/Essential-Drivers-Prentice-Software-Development/dp/0132396556/ref=tmm_hrd_swatch_0?_encoding=UTF8&qid=&sr=); simply excellent!
        - ['Linux Driver Development for Embedded Processors - Second Edition: Learn to develop embedded Linux drivers with kernel 4.9 LTS', Alberto Liberal de los Rios](https://www.amazon.in/Linux-Driver-Development-Embedded-Processors-ebook/dp/B07L512BHG/ref=sr_1_6?crid=3RLFFZQXGAMF4&keywords=linux+driver+development+embedded&qid=1555486342&s=books&sprefix=linux+driver+%2Cstripbooks%2C270&sr=1-6-catcorr); very good, very recent (as of this writing)
        - ['Linux Device Drivers Development', John Madieu, Packt, Oct 2017](https://www.amazon.in/Linux-Device-Drivers-Development-Madieu/dp/1785280007/ref=sr_1_2?keywords=linux+device+driver&qid=1555486515&s=books&sr=1-2); recent (4.13 kernel)
        -  ['Linux Device Drivers', Rubini, Hartmann, Corbet, 3rd Ed](https://www.amazon.in/Linux-Device-Drivers-Kernel-Hardware/dp/8173668493/ref=sr_1_1?keywords=linux+device+driver&qid=1555486515&s=books&sr=1-1); venerable (but) old - the famous LDD3 book
    -  *The kernel driver API manual*; this is one of the PDF documents generated by doing '`make pdfdocs`' within a recent Linux kernel source tree
    -  Practical tutorials:
        - ['Device Drivers, Part 8: Accessing x86-Specific I/O-Mapped Hardware', Anil K Pugalia, OpenSourceForU, July 2011](https://opensourceforu.com/2011/07/accessing-x86-specific-io-mapped-hardware-in-linux/)


## Ch 15 and Ch 16 : Kernel Synchronization Primitives and How to Use Them - Further Reading

- ['What every systems programmer should know about concurrency', M Kline, May 2018](https://assets.bitbashing.io/papers/concurrency-primer.pdf)
- This presentation covers deadlock scenarios: ['Lockdep: how to read it's cryptic output', Steve Rostedt, Linux Plumbers Conf 2011](https://blog.linuxplumbersconf.org/2011/ocw/sessions/153)

-  ['Mutexes and Semaphores Demystified', Micheal Barr, May 2016](https://barrgroup.com/Embedded-Systems/How-To/RTOS-Mutex-Semaphore)
- The Mars Pathfinder mission and Priority Inversion
	- ['What really happened on Mars ?' Glenn Reeves](https://cs.unc.edu/~anderson/teach/comp790/papers/mars_pathfinder_long_version.html)
	- A must-read (PDF): [*What the Media Couldn't Tell You About Mars Pathfinder*](http://people.cs.ksu.edu/~hatcliff/842/Docs/Course-Overview/pathfinder-robotmag.pdf)
	- Detailed paper (PDF): [*Mars Pathfinder: Priority Inversion Problem*, R.M. Pathan](http://www.cse.chalmers.se/edu/year/2015/course/EDA222/Documents/Misc/Report_MarsPathFinder.pdf)
	- More recently (May 2015), with respect to the NASA Curiosity Rover: [*13 engineering truths proved by NASA's Curiosity Rover*, EDN](https://www.edn.com/electronics-blogs/now-hear-this/4439386/13-engineering-truths-proved-by-NASA-s-CuriosityRover?mc=NL_EDN_EDT_EDN_funfriday_20150508&cid=NL_EDN_EDT_EDN_funfriday_20150508&elq=75871e4437784010b51f12e4c3be00a2&elqCampaignId=22919&elqaid=25798&elqat=1&elqTrackId=8e848e14bf3e41b58e660c804b335b07)
 - OSFY magazine: ['How to Avoid Priority Inversion and Enable Priority Inheritance in Linux Kernel Programming', Thangaraju & Warade, Apr 2019](https://opensourceforu.com/2019/04/how-to-avoid-priority-inversion-and-enable-priority-inheritance-in-linux-kernel-programming/)
 - Spinlock internal implementation
    - ['Peeking Under the Hood', John Sloan, Chip Overclock blog, May 2012](https://coverclock.blogspot.com/2012/05/peeking-under-hood.html)
    - ['spin_lock implementation in ARM linux', linuxforthenew blog, Apr 2013](http://linuxforthenew.blogspot.com/2013/04/spinlock-implementation-in-arm-linux.html)
- False Sharing
    - ['Avoiding and Identifying False Sharing Among Threads', Intel, Nov 2011](https://software.intel.com/en-us/articles/avoiding-and-identifying-false-sharing-among-threads)
    - Blog article: ['Understanding False Sharing', Mar 2017](https://parallelcomputing2017.wordpress.com/2017/03/17/understanding-false-sharing/)
- PDF: ['Memory Barriers: a Hardware View for Software Hackers', 
Paul E. McKenney, IBM LTC, June 2010](http://www.rdrop.com/~paulmck/scalability/paper/whymb.2010.06.07c.pdf)

 - Lockless or Lock-Free programming
    - PDF: ['What every systems programmer should know about concurrency', Matt Kline, May 2018](https://assets.bitbashing.io/papers/concurrency-primer.pdf)
    - Lock-Free programming concepts 
        - ['Fear and Loathing in Lock-Free Programming', T Neely, Medium](https://medium.com/@tylerneely/fear-and-loathing-in-lock-free-programming-7158b1cdd50c) : an excellent 'Lock-free 101' article
        - Good article series: ['Introduction to Lock-free Algorithms'](http://www.1024cores.net/home/lock-free-algorithms/introduction)
        - Presentation: ['Lock-Free Programming', Geoff Langdale](https://www.cs.cmu.edu/~410-s05/lectures/L31_LockFree.pdf)
        - Blog article: ['Some notes on lock-free and wait-free algorithms'](http://www.rossbencina.com/code/lockfree?q=~rossb/code/lockfree/)
        - ['CppCon 2014: Herb Sutter "Lock-Free Programming (or, Juggling Razor Blades), Part I', Herb Sutter, YouTube video](https://www.youtube.com/watch?v=c1gO9aB9nbs)

    - Percpu
        - Blog: ['A brief introduction to per-cpu variables', Chen, May 2014](http://thinkiii.blogspot.com/2014/05/a-brief-introduction-to-per-cpu.html)
        - ['How are percpu pointers implemented in the Linux kernel?', StackOverflow, June 2013](https://stackoverflow.com/questions/16978959/how-are-percpu-pointers-implemented-in-the-linux-kernel)

    - RCU in the Linux kernel
        - The excellent and extensive kernel community documentation: [RCU](https://www.kernel.org/doc/Documentation/RCU/00-INDEX)
        - ['RCU Usage In the Linux Kernel: One Decade Later', McKenny et al (PDF), circa 2013](http://www2.rdrop.com/~paulmck/techreports/RCUUsage.2013.02.24a.pdf)
        - A 3 series article to learn RCU: ['What is RCU, Fundamentally?', LWN, Paul McKenney, Dec 2007, Part 1 of 3 ](https://lwn.net/Articles/262464/)
        - ['Make any algorithm lock-free with this one crazy trick', May 2016](https://www.the-paper-trail.org/post/2016-05-25-make-any-algorithm-lock-free-with-this-one-crazy-trick/)
    - ['The lockless page cache', LWN, Jon Corbet, July 2008](https://lwn.net/Articles/291826/) (an interesting article)
    - Lockdep
        - ['Runtime locking correctness validator', kernel documentation](https://www.kernel.org/doc/Documentation/locking/lockdep-design.txt)
        - ['The kernel lock validator', LWN, J Corbet, May 2006](https://lwn.net/Articles/185666/)
        - ['Lockdep: how to read it's cryptic output', Steve Rostedt, Linux Plumbers Conf 2011](https://blog.linuxplumbersconf.org/2011/ocw/sessions/153)
        - ['LOCKDEP, AN INSIDE OUT PERSPECTIVE', Nahim El Atmani, Nov 2016](https://www.lse.epita.fr/data/lt/2016-11-08/lt-2016-11-08-Nahim_El_Atmani-lockdep-an-inside-out-perspective.pdf)
        - (until it gets merged :-) : ['[PATCH 16/28] locking/lockdep: Add explanation to lock usage rules in lockdep design doc', Yuyang Du](https://lkml.org/lkml/2019/4/24/333), Apr 2019


[\[Top\]](https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook/blob/master/Further_Reading.md#further-reading)

[End of Doc]
