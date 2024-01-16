# mon_self

Example of application monitoring its own UM stats.

# Table of contents

<!-- mdtoc-start -->
&bull; [mon_self](#mon_self)  
&bull; [Table of contents](#table-of-contents)  
&bull; [Copyright And License](#copyright-and-license)  
&bull; [Repository](#repository)  
&bull; [Introduction](#introduction)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Quickstart](#quickstart)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Size of Data Set](#size-of-data-set)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Architecture](#architecture)  
&bull; [Coding Notes](#coding-notes)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [C Error Handling](#c-error-handling)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Java Statistics Fields](#java-statistics-fields)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Delay Before Terminate](#delay-before-terminate)  
<!-- TOC created by '/home/sford/bin/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->

# Copyright And License

All of the documentation and software included in this and any
other Informatica Ultra Messaging GitHub repository
(C) Copyright 2023,2024 Informatica Inc. All Rights Reserved.

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

The programs in this repository are not intended to be used as-is.
They are intended as coding demonstrations;
possibly a source of cut-and-paste code.

## Quickstart

You can build and run the enclosed programs using the
"tst.sh" script.

The "mon_self.c" program creates two contexts and a source in each one.
It creates a receiver in the first context only.
It runs two separate statistics threads,
one for each context.

Note that for the purposes of the demonstration,
statistics are sampled and printed by each thread
every 2 seconds (the stats interval).
In a large production deployment,
this would produce an unreasonable amount of statistical data.
A stats interval of 10 minutes or more is much more common.

## Size of Data Set

One goal of this repository is to demonstrate a "minimal" set of statistics.
I.e. only a subset of all statistics are printed.
Informatica suggests that you not reduce the included set further.
For example, some customers will determine that some of their subscribers
are joined to many hundreds of transport sessions,
resulting in many hundreds of statistics records printed with each sample.
It may be tempting to aggregate the transport sessions and produce a single
summary line.
This would be useful for detecting a problem,
but may not be sufficient to diagnose and treat the problem.
If the volume of data generated per hour is too large,
Informatica recommends decreasing the frequency of sampling
rather than decreasing the amount of data per sample.

A few customers have an existing application monitoring infrastructure
that cannot easily handle multiple per-transport-session records.
These customers aggregated all receiver transport stats into a single
receiver record and included that in their monitoring infrastructure.
However, they wrote the individual transport session records to a
local disk file to make the detailed information available,
albeit somewhat less conveniently.

One exception to the "no aggregation" guideline are
the "drop" counters due to malformed packets.
It is rarely useful to differentiate between the different
drop counters.
If any of them are non-zero,
it typically means that a security port scanner or a
misconfigured application is sending unrecognized packets to UM.

## Architecture

A separate thread is created to sample stats and print them to STDOUT.
This is preferred over the method used by the UM example applications
which frequently use a UM timer to trigger sampling and printing of statistics.
The problem with using a UM timer is that it can interfere with the
reception of time-critical messages,
introducing undesired latency outliers and jitter.

The stats thread is separated into its own module
(stats_thread.c, StatsThread.java).
Public methods allow creation, starting,
terminating, and in C, deleting the thread.

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

## Java Statistics Fields

Note that when printing the monitoring data,
the C field names are used.
This was done to make the field naming common,
and to make it easier to find the C documentation for the fields
(the C doc is generally better than the Java/.NET doc).

WARNING!
The Java getters for transport statistics fields do not perform error checking
against the transport type.
For example, you might use the method "srcStats.messagesSent(i)" to get the
number of datagrams sent.
However, if the transport type is TCP, that field is not supported,
and it simply returns 0.
You should only print fields that are valid for the transport type.
Unfortunately, the Java API documentation does not list the valid
fields per transport type.
You need to use the C documentation for that.
Here are appropriate links:

Source transport statistics:
<ul>
<li>[TRANSPORT_STAT_LBTRM](https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtrm__t__stct.html)
<li>[TRANSPORT_STAT_LBTRU](https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtru__t__stct.html)
<li>[TRANSPORT_STAT_TCP](https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__tcp__t__stct.html)
<li>[TRANSPORT_STAT_LBTIPC](https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtipc__t__stct.html)
<li>[TRANSPORT_STAT_LBTSMX](https://ultramessaging.github.io/currdoc/doc/API/structlbm__src__transport__stats__lbtsmx__t__stct.html)
</ul>

Receiver transport statistics:
<ul>
<li>[TRANSPORT_STAT_LBTRM](https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtrm__t__stct.html)
<li>[TRANSPORT_STAT_LBTRU](https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtru__t__stct.html)
<li>[TRANSPORT_STAT_TCP](https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__tcp__t__stct.html)
<li>[TRANSPORT_STAT_LBTIPC](https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtipc__t__stct.html)
<li>[TRANSPORT_STAT_LBTSMX](https://ultramessaging.github.io/currdoc/doc/API/structlbm__rcv__transport__stats__lbtsmx__t__stct.html)
</ul>

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
