#ifndef REDFS_H
#define REDFS_H

#include <stdint.h>

#define BLOCK_SIZE 4096

// 12 * 4 bytes
typedef struct Superblock_T {
  uint8_t signature[5];

  uint32_t inodes_total;
  uint32_t blocks_total;
  uint32_t inodes_free;
  uint32_t blocks_free;
  uint32_t blocks_in_pool;
  uint32_t inodes_in_pool;
  uint32_t inode_size;
  uint32_t block_size;
  uint32_t total_pools;
  uint64_t last_mount_time;
  uint64_t last_write_time;

} SuperBlock;

#define FILE 1
#define DIR 2
// 4 * 10 + 1 + 2 * 4
// Needs to be 128 bytes
typedef struct Inode_T {
  uint32_t signature;
  uint8_t type;
  uint32_t parent;
  uint32_t block0;
  uint16_t length0;
  uint32_t block1;
  uint16_t length1;
  uint32_t block2;
  uint16_t length2;
  uint32_t block3;
  uint16_t length3;
  uint32_t permissions;
  uint32_t uid;
  uint32_t gid;
  uint64_t ts_created;
  uint64_t ts_mod;
  uint64_t ts_read;
  uint32_t size;
  uint8_t padding[40];
} INode;

typedef struct DirEntry_T {
  uint32_t signature;
  uint16_t name_length;
  // uint8_t *name;
} DirEntry;

#endif