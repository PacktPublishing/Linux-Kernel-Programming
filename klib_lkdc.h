/* 
 * klib_lkdc.h
 * Our klib_lkdc 'library' header.
 ****************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 ****************************************************************
 * This kernel (module) header code is meant to serve as a 'library' of sorts.
 * Other kernel modules in our codebase might wish to link into it and use
 * it's code.
 *
 * For details, please refer the book.
 */
#ifndef __KLIB_LKDC_H__
#define __KLIB_LKDC_H__

#include <linux/init.h>
#include <linux/module.h>

u64 powerof(int base, int exponent);
void show_phy_pages(const void *kaddr, size_t len, bool contiguity_check);
void show_sizeof(void);

#endif
