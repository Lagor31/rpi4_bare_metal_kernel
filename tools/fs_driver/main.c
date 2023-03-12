#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PACKED __attribute((__packed__))

#define BOOT_SIGNATURE 0xAA55
#define BLOCK_SIZE 4096

typedef struct PACKED {
  uint8_t head;
  uint8_t sector : 6;
  uint8_t cylinder_hi : 2;
  uint8_t cylinder_lo;
} chs_address;

typedef struct PACKED {
  uint8_t status;
  chs_address first_sector;
  uint8_t type;
  chs_address last_sector;
  uint32_t first_lba_sector;
  uint32_t num_sectors;
} partition_entry;

typedef struct PACKED {
  uint8_t bootCode[0x1BE];
  partition_entry partitions[4];
  uint16_t bootSignature;
} master_boot_record;

// 12 * 4 bytes
typedef struct Superblock_T {
  uint8_t signature[5];
  uint32_t inodes_total;
  uint32_t blocks_total;
  uint32_t inodes_free;
  uint32_t blocks_free;
  uint32_t blocks_in_group;
  uint32_t inodes_in_group;
  uint32_t inode_size;
  uint32_t block_size;
  uint32_t total_pools;
  uint64_t last_mount_time;
  uint64_t last_write_time;

} SuperBlock;

#define FILE 1
#define DIR 2
// 12 * 4 bytes
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
} INode;

typedef struct DirEntry_T {
  uint32_t signature;
  uint16_t name_length;
  // uint8_t *name;
} DirEntry;

void main(int argc, char **args, char **env) {
  char *disk_name = args[1];
  int disk_fd = open(disk_name, O_RDWR, 0777);
  if (disk_fd <= 0) {
    printf("Error!");
    exit(1);
  }

  master_boot_record mbr;
  partition_entry lagor_part_start;
  read(disk_fd, &mbr, sizeof(master_boot_record));

  if (mbr.bootSignature != BOOT_SIGNATURE) {
    printf("BAD BOOT SIGNATURE ON DISK %s: %X\n", disk_name, mbr.bootSignature);
  }

  for (int i = 0; i < 4; i++) {
    if (mbr.partitions[i].type == 0) {
      break;
    }

    printf("Partition %d:\n", i);
    printf("\t Type: %x\n", mbr.partitions[i].type);
    printf("\t NumSecs: %d\n", mbr.partitions[i].num_sectors);
    printf("\t Status: %d\n", mbr.partitions[i].status);
    printf("\t Start: %d\n", mbr.partitions[i].first_lba_sector);
    if (mbr.partitions[i].type == 0x31) {
      printf("Found Lagor 0x31 partition starting at LBA: %d Size: %d\n",
             mbr.partitions[i].first_lba_sector, mbr.partitions[i].num_sectors);
      lagor_part_start = mbr.partitions[i];
    }
  }

  // fread(data, 512, 1, disk_file);

  uint32_t total_size = lagor_part_start.num_sectors * 512;
  // Remove superblock, rest divide
  total_size -= 512;

  SuperBlock *sb = calloc(1, 512);
  sb->block_size = BLOCK_SIZE;
  sb->inode_size = 128;
  sb->inodes_in_group = 1024;

  uint32_t block_pool_size = BLOCK_SIZE << 12;
  uint32_t inode_table_size = sb->inode_size * sb->inodes_in_group;
  uint32_t pool_size = inode_table_size + block_pool_size;

  uint32_t tot_units = total_size / pool_size;
  sb->total_pools = tot_units;
  sb->blocks_free = (tot_units * block_pool_size) - 2;
  sb->blocks_in_group = block_pool_size;
  sb->blocks_total = sb->blocks_free + 2;
  sb->inodes_free = (sb->inodes_in_group * tot_units) - 3;
  sb->inodes_total = sb->inodes_in_group * tot_units;
  sb->signature[0] = 'L';
  sb->signature[1] = 'a';
  sb->signature[2] = 'g';
  sb->signature[3] = 'o';
  sb->signature[4] = 'r';

  INode *inode = (INode *)calloc(1, sb->inode_size);
  printf(
      "Total Part Size: %d TotUnits: %d BlockPoolSize: %d UnitSize: "
      "%d\nSuperblock: "
      "block_size=%d, "
      "blocks_free=%d, "
      "blocks_grp=%d\n"
      "blocks_tot=%d, inodes_free=%d, inodes_total=%d, inodes_group=%d",
      total_size, tot_units, block_pool_size, pool_size, sb->block_size,
      sb->blocks_free, sb->blocks_in_group, sb->blocks_total, sb->inodes_free,
      sb->inodes_total, sb->inodes_in_group);
  printf("\n");

  // inode->signature = (uint32_t)getchar();
  printf("Writing Superblock...");
  getchar();

  // getchar();
  //  lseek(disk_fd, 0, SEEK_SET);
  printf("Lseeking to: %d\n", lagor_part_start.first_lba_sector * 512);
  int l = lseek(disk_fd, lagor_part_start.first_lba_sector * 512, SEEK_SET);
  unsigned char *d = (char *)calloc(1, 512);
  read(disk_fd, d, 10);

  for (int i = 0; i < 10; ++i) printf("%x ", d[i]);
  printf("\n");
  getchar();
  unsigned char data[5] = {'L', 'a', 'g', 'o', 'r'};
  lseek(disk_fd, lagor_part_start.first_lba_sector * 512, SEEK_SET);
  // if (write(disk_fd, &data, 5) == -1) goto errors;

  if (write(disk_fd, sb, 512) == -1) goto errors;
  // Skip just written Superblock
  printf("Superblock Done!\n");
  printf("Writing Inodes...\n");
  getchar();
  lseek(disk_fd, lagor_part_start.first_lba_sector * 512 + 512, SEEK_SET);

  // inode->signature = 0xfedefede;

  for (int u = 0; u < tot_units; ++u) {
    for (int i = 0; i < sb->inodes_in_group; ++i) {
      if (write(disk_fd, inode, sb->inode_size) == -1) goto errors;
      // printf("Written U:%d I:%d \n", u, i);
    }
    // if (u == tot_units - 1) break;
    lseek(disk_fd, block_pool_size, SEEK_CUR);
  }
  printf("Table reset!");

  getchar();
  lseek(disk_fd, lagor_part_start.first_lba_sector * 512 + 512, SEEK_SET);
  inode->signature = 0xEEEE;
  inode->block0 = 1;
  inode->length0 = 1;
  inode->permissions = 0x777;
  inode->type = DIR;
  inode->size = 4096;
  if (write(disk_fd, inode, sb->inode_size) == -1) goto errors;
  printf("First 1  done!\n");

  char *content = "First ever file\n";

  inode->signature = 0xEEEE;
  inode->block0 = 2;
  inode->length0 = 1;
  inode->permissions = 0x777;
  inode->type = FILE;
  inode->size = strlen(content);

  if (write(disk_fd, inode, sb->inode_size) == -1) goto errors;

  DirEntry *dir;
  char *root_name = "/";
  dir = calloc(sizeof(DirEntry) + strlen(root_name), 1);
  dir->name_length = strlen(root_name);
  dir->signature = 0xDDDD;
  printf("First 2 indodes done!\n");

  uint32_t block_num = 1;
  lseek(disk_fd,
        (lagor_part_start.first_lba_sector * 512) +
            (512 + inode_table_size + (block_num * sb->block_size)),
        SEEK_SET);
  if (write(disk_fd, dir, sizeof(DirEntry)) == -1) goto errors;
  if (write(disk_fd, root_name, strlen(root_name)) == -1) goto errors;

  uint32_t child = 1;
  if (write(disk_fd, &child, 4) == -1) goto errors;
  printf("Done Directory!\n");

  getchar();

  block_num = 2;

  char *filename = "lagor.conf";
  free(dir);
  dir = calloc(sizeof(DirEntry) + strlen(filename), 1);
  dir->name_length = strlen(filename);
  dir->signature = 0xDDDD;
  lseek(disk_fd,
        (lagor_part_start.first_lba_sector * 512) +
            (512 + inode_table_size + (block_num * sb->block_size)),
        SEEK_SET);
  if (write(disk_fd, dir, sizeof(DirEntry)) == -1) goto errors;
  if (write(disk_fd, filename, strlen(filename)) == -1) goto errors;
  if (write(disk_fd, content, strlen(content)) == -1) goto errors;

  free(dir);

  printf("Inodes Done!");
  printf("All done!");
  exit(0);

errors:
  printf("Error!\n");
  close(disk_fd);

  exit(-1);
  /*
    DIR *d;
    struct dirent *dir;
    d = opendir(args[2]);
    if (d) {
      while ((dir = readdir(d)) != NULL) {
        printf("%s\n", dir->d_name);
      }
      closedir(d);
    } */
}
