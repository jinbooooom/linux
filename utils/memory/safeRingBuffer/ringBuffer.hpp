#ifndef __RING_BUFFER_HPP__
#define __RING_BUFFER_HPP__

/**
 * Description: This file is used to implement the ring buffer,
 * which is thread-safe.
 * Author: Jinboom
 * Date: 2021/8
 */

#include <mutex>
#include <thread>

using byte_t = unsigned char;

class RingBuffer
{
  public:
    RingBuffer(int id, int len);
    ~RingBuffer();

  private:
    int mId;
    byte_t *mBuf;
    int mFront;
    int mRear;
    int mAllocSize;

    std::mutex mMutex;

  public:
    /**
     * std::vector style interface
     */
    int size();
    int capacity();
    void resize(int len);
    bool empty();
    void clear();
    int push(byte_t *data, int len);
    int get(byte_t *data, int len);

    int free_size();
    void show();  // for debug
    int get_id() const;
};

#endif