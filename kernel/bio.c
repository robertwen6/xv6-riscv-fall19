// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13  //13个哈希表

struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf hashbucket[NBUCKETS];
} bcache;

void
binit(void)
{
  int i;
  struct buf *b;
  for(i=0; i<NBUCKETS; i++){
    initlock(&bcache.lock[i], "bcache");
    //链表构造
    bcache.hashbucket[i].prev = &bcache.hashbucket[i];
    bcache.hashbucket[i].next = &bcache.hashbucket[i];
  }
  
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.hashbucket[0].next;
    b->prev = &bcache.hashbucket[0];
    initsleeplock(&b->lock, "buffer");
    bcache.hashbucket[0].next->prev = b;
    bcache.hashbucket[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int pos;
  pos = blockno % NBUCKETS;  //对13取余计算位置
  acquire(&bcache.lock[pos]);

  // Is the block already cached?
  for(b = bcache.hashbucket[pos].next; b != &bcache.hashbucket[pos]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[pos]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  int pos_next;
  pos_next = (pos + 1) % NBUCKETS;    //下一位置
  while(pos_next != pos){
    acquire(&bcache.lock[pos_next]);
    for(b = bcache.hashbucket[pos_next].prev; b != &bcache.hashbucket[pos_next]; b = b->prev){
      if(b->refcnt == 0) {  //找到
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        //断开
        b->next->prev=b->prev;
        b->prev->next=b->next;
        release(&bcache.lock[pos_next]);
        //头插
        b->next=bcache.hashbucket[pos].next;
        b->prev=&bcache.hashbucket[pos];
        bcache.hashbucket[pos].next->prev=b;
        bcache.hashbucket[pos].next=b;

        release(&bcache.lock[pos]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    //没找到，释放当前锁，跳到下一个
    release(&bcache.lock[pos_next]);
    pos_next = (pos_next + 1) % NBUCKETS;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int pos;
  pos = b->blockno % NBUCKETS;
  acquire(&bcache.lock[pos]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbucket[pos].next;
    b->prev = &bcache.hashbucket[pos];
    bcache.hashbucket[pos].next->prev = b;
    bcache.hashbucket[pos].next = b;
  }

  release(&bcache.lock[pos]);
}

void
bpin(struct buf *b) {
  int pos;
  pos = b->blockno % NBUCKETS;
  acquire(&bcache.lock[pos]);
  b->refcnt++;
  release(&bcache.lock[pos]);
}

void
bunpin(struct buf *b) {
  int pos;
  pos = b->blockno % NBUCKETS;
  acquire(&bcache.lock[pos]);
  b->refcnt--;
  release(&bcache.lock[pos]);
}


