# mon_self

Example of application monitoring its own UM stats.

# Table of contents

<!-- mdtoc-start -->
&bull; [mon_self](#mon_self)  
&bull; [Table of contents](#table-of-contents)  
&bull; [Copyright And License](#copyright-and-license)  
&bull; [Repository](#repository)  
&bull; [Introduction](#introduction)  
&bull; [Coding Notes](#coding-notes)  
<!-- TOC created by '/home/sford/bin/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->

# Copyright And License

All of the documentation and software included in this and any
other Informatica Ultra Messaging GitHub repository
Copyright (C) Informatica, 2024. All rights reserved.

Permission is granted to licensees to use
or alter this software for any purpose, including commercial applications,
according to the terms laid out in the Software License Agreement.

This source code example is provided by Informatica for educational
and evaluation purposes only.

THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR
PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE
UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
BE LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR
INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE
TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF
THE LIKELIHOOD OF SUCH DAMAGES.

# Repository

See https://github.com/UltraMessaging/mon_self for code and documentation.

# Introduction

Informatica recommends that customers implement a centralized monitoring
collector to record UM statistics from all UM components (publishers,
subscribers, and infrastructure components like Stores and DRO).
However, we understand that implementing a new monitoring infrastructure
is time-consuming and can be difficult for customers to prioritize.

This repository demonstrates a method whereby applications can be modified
to monitor themselves, without the need for a centralized infrastructure.
Often this form of self-monitoring can leverage the user's existing
application monitoring infrastructure, making it much easier to implement,
deploy, and operate with existing systems and staff.

One goal of this repository is to demonstrate a "minimal" set of statistics.

A separate thread is created to sample stats and print them to STDOUT.



# Coding Notes

## C Error Handling

A very simple error handling convention is used so as not to obscure the
algorithms being demonstrated.
A set of three code macros is used to wrap function calls:
<ul>
<li>E - For UM API calls.
<li>ENZ - For non-UM functions that return non-zero for error with
"errno" holding extended error information.
<li>ENL - For non-UM functions that return NULL for error with
"errno" holding extended error information.
</ul>
These macros write error information to STDERR and exit the program
with a non-zero status.

## Delay Before Terminate

The stats thread's main loop checks the "running" flag and has a 1-second sleep.
Whuen main() wants to shut down, it clears the "running" flag and joins the
stats thread.
Main will have to wait up to 1 full second for the stats thread to exit.

This could be made event-driven using:
<ul>
<li>A semaphore with a timed wait,
<li>A condition variable with a timed wait,
<li>A pipe with select/epoll with a timeout.
</ul>
