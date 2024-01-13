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

# Coding Notes

The stats thread's main loop checks the "running" flag and has a 1-second sleep.
When main() wants to shut down, it clears the "running" flag and joins the
stats thread.
Main will have to wait up to 1 full second for the stats thread to exit.

This could be made event-driven using a semaphore with a timed wait,
or with a condition variable with a timed wait,
or with a pipe select.
