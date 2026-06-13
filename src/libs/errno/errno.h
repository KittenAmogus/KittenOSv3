#ifndef ERRNO_H
#define ERRNO_H

typedef enum {
  // POSIX
  SUCCESS = 0,
  EPERM   = 1,
  ENOENT  = 2,
  EIO     = 5,
  ENOMEM  = 12,
  EACCES  = 13,
  EEXIST  = 17,
  ENOTDIR = 20,
  EISDIR  = 21,
  EINVAL  = 22,
  ENOSPC  = 28,

  // Custom
  ENODEV = 30,
} error_t;

#endif // ERRNO_H

