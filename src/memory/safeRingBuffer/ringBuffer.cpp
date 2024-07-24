/**
 * Description: This file is used to implement the ring buffer,
 * which is thread-safe.
 * Author: Jinboom
 * Date: 2021/8
 */

#include "ringBuffer.hpp"
#include "../../log/log.hpp"
#include "string.h"

RingBuffer::RingBuffer(int id, int len)
{
    mId        = id;
    mFront     = 0;        // index read next time.
    mRear      = 0;        // index written next time.
    mAllocSize = len + 1;  // not index, but mAllocSize - 1 is index
    mBuf       = new byte_t[mAllocSize];
}

RingBuffer::~RingBuffer()
{
    if (mBuf)
    {
        delete[] mBuf;
        mBuf = nullptr;
    }
}

/**
 * Number of bytes currently used.
 * That is, the size of unprocessed data.
 */
int RingBuffer::size()
{
    std::lock_guard<std::mutex> lock(mMutex);
    /**
     * when mRear > mFront:
     * size = mRear - mFront
     *
     * when mRear < mFront:
     * size = (mAllocSize - 1) - (mFront - mRear - 1)
     *      = mRear - mFront + mAllocSize
     *
     * when mRear == mFront:
     * size = 0
     *
     * So in the above cases, the formula can be combined into:
     * size = (mRear - mFront + mAllocSize) % mAllocSize
     */
    return (mRear - mFront + mAllocSize) % mAllocSize;
}

/**
 * Maximum number of bytes that ring buffer can store.
 */
int RingBuffer::capacity()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mAllocSize - 1;
}

/**
 * Resetting the buf size will also reset the data. Not recommended.
 */
void RingBuffer::resize(int len)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (len + 1 == mAllocSize)
    {
        return;
    }

    byte_t *tmp = new byte_t[len + 1];

    /**
     * When len < mAllocSize, some data will be discarded!
     */
    // int cpy_size = len + 1 > mAllocSize ? mAllocSize : len + 1;
    // memcpy(tmp, mBuf, cpy_size);
    delete[] mBuf;
    mBuf       = tmp;
    mAllocSize = len + 1;

    mFront = mRear = 0;
}

/**
 * Determine if ring buffer is empty.
 */
bool RingBuffer::empty()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mFront == mRear;
}
/**
 * clear ring buffer.
 */
void RingBuffer::clear()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mFront = 0;
    mRear  = 0;
}

/**
 * Get len bytes from data and store them in the ring buffer.
 * If the space available in the ring buffer is less than len, no data is written.
 */
int RingBuffer::push(byte_t *data, int len)
{
    std::lock_guard<std::mutex> lock(mMutex);  // 这里可以

    int ret = 0;
    /**
     * If the data is too late to be processed, the latest data will be
     * discarded instead of the new data overwriting the old data.
     */
    if (mRear >= mFront && (mAllocSize - 1) - (mRear - mFront) >= len)
    {
        /**
         * The right half has enough space for writing data.
         */
        if (mAllocSize - mRear >= len)
        {
            memcpy(mBuf + mRear, data, len);
            mRear += len;
            mRear %= mAllocSize;
        }
        else
        {
            /**
             * lr: The length of data that should be written in the right half
             * ll: The length of data that should be written in the left half
             */
            int lr = mAllocSize - mRear;
            int ll = len - lr;
            memcpy(mBuf + mRear, data, lr);
            memcpy(mBuf, data + lr, ll);
            mRear = ll;
        }
        ret = len;
    }
    /**
     * The left half has enough space for writing data.
     */
    else if (mRear < mFront && (mFront - mRear) - 1 >= len)
    {
        // pvrw("show");
        memcpy(mBuf + mRear, data, len);
        mRear += len;
        mRear %= mAllocSize;

        ret = len;
    }
    else
    {
        logw("RingBuffer%d: New data cannot be written, please process the data in time.", mId);
        ret = 0;
    }

    // show();
    return ret;
}

/**
 * Get len bytes from ring buffer and store them in data.
 * If the data length in the ring is less than len, no data will be written.
 */
int RingBuffer::get(byte_t *data, int len)
{
    std::lock_guard<std::mutex> lock(mMutex);

    int ret = 0;

    if (mRear > mFront && mRear - mFront >= len)
    {
        memcpy(data, mBuf + mFront, len);
        mFront += len;
        mFront %= mAllocSize;

        ret = len;
    }
    /**
     * The left half + the right half has enough data.
     */
    else if (mRear < mFront && (mAllocSize - mFront) + mRear - 1 >= len)
    {
        /**
         * There is enough data available in the right half.
         * So there is no need to use the data in the left half.
         */
        if (mAllocSize - mFront >= len)
        {
            memcpy(data, mBuf + mFront, len);
            mFront += len;
            mFront %= mAllocSize;

            ret = len;
        }
        else
        {
            /**
             * lr: The length of data that can be obtained in the right half
             * ll: The length of data that can be obtained in the left half
             */
            int lr = mAllocSize - mFront;
            int ll = len - lr;
            memcpy(data, mBuf + mFront, lr);
            memcpy(data + lr, mBuf, ll);
            mFront = ll;

            ret = len;
        }
    }
    else
    {
        logw("RingBuffer%d is empty or data is not enough.", mId);
        ret = 0;
    }

    logi("RingBuffer%d get %d data", mId, ret);
    // show();
    return ret;
}

/**
 * Number of bytes that can be used.
 */
int RingBuffer::free_size()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mRear >= mFront)
    {
        /**
         * Available size MINUS used size
         */
        return (mAllocSize - 1) - (mRear - mFront);
    }
    else
    {
        return (mFront - mRear) - 1;
    }
}

void RingBuffer::show()
{
    logi("ringbuffer data: ");
    // index, for debug
    for (int i = 0; i < mAllocSize; ++i)
    {
        logi("%2d ", i);
    }
    logi("\n");

    // real data
    for (int i = 0; i < mAllocSize; ++i)
    {
        logi("%2d ", (int)mBuf[i]);
    }
    logi("\n");

    logi("mFront = %d, mRear = %d", mFront, mRear);
}

int RingBuffer::get_id() const
{
    return mId;
}