# FAT12Parser
A parser written in C that does minimal read, copy, and write operations on a FAT12 disk image.

It's uses quite a bit of wacky bit shifting magic, and basically works by `fseek()`-ing by specific
bytes to traverse the file. If you really want to figure out whats going on, you can look at the FAT12 specs in this
repo, and try to understand why things are happenning the way they are. `./diskput` is definitely not
bulletproof as it probably doesn't handle very typical cases of overwritting and disk size overflow, etc.

But this is definitely one of the more intense C projects I've done in terms of work, even though the solution
isn't particularly esoteric. Implementing something by directly reading off a
specification was an interesting experience.
 
## Description
The tool implements the following operations:
  - ./diskinfo disk_image             - which lists information about the disk
  - ./disklist disk_image             - which lists the contents of the root directory
  - ./diskget disk_image file_to_get  - copies a file from the disk to the current directory
  - ./diskput disk_image file_to_put  - puts foo into the disk

## Build
  - You can use make to build the executables
  - You can then run the commands described above to run it
