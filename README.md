This is my third try to make OS.
branch master is not for real hardware, use ONLY on qemu/bochs,
 it can (and will) format your ATA disk (if you have one)


**ONLY FOR LINUX** (Can't use Windows tools),
you can compile to Windows, but **DO NOT USE MAKE IN WINDOWS**


`make`        - Compile
`make hda`    - Create disk for QEMU (/dev/urandom)
`make qemu`   - Run with QEMU (recommended)
`make bochs`  - Run with BOCHS
`make clean`  - Remove all compiled stuff, not HDA


You can switch to branch posix_rew to check i rewrite it to POSIX standars
 and compatibility with GCC and standart libs.

 
 Branch posix_rew is TOO RAW to be main branch, i didn't even fix input
 but when it's ready, it will contain tools for compiling
 real software (simple, just stdandart libs) and run on real hardware.


I use Arch, btw :)
