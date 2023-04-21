#include "ringBuffer.hpp"
#include "../../log/log.hpp"

#include <unistd.h>
#include <thread>

static const int MAX = 18;
static const int BUF_SIZE = 12;

byte_t data[MAX]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

RingBuffer *buf = new RingBuffer(0, BUF_SIZE);

void write_data()
{
    int data_w_pos = 0;

    int w_cnt = 5;

    byte_t *packet = new byte_t[w_cnt];

    while (true)
    {
        for (int i = 0; i < w_cnt; ++i)
        {
            packet[i] = data[(data_w_pos + i) % MAX];
        }

        if (buf->push(packet, w_cnt))
        {
            logd("[thread write_data] ring buf size = %d", buf->size());
        }
        else
        {
            //loge("[thread write_data] don't write data.");
        }

        data_w_pos += w_cnt;
        data_w_pos %= MAX;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (time(nullptr) % 15 == 0)
        {
            buf->resize(BUF_SIZE * 2);
            logw("resize ring buffer, size = %d", buf->capacity());
        }
    }
}

void read_data1()
{
    int r_cnt = 2;
    byte_t *arr = new byte_t[r_cnt];

    while (true)
    {
        logi("[th1] get %d byte_t.", r_cnt);
        if (buf->get(arr, r_cnt))
        {
            if (buf->empty())
            {
                logw("[th1] after get data, ring buf is empty.");
            }
            logd("[th1] ring buf size = %d, free_size = %d",
                 buf->size(), buf->free_size());
        }
        else
        {
            logi("[th1] th1 don't get data.");
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    delete[] arr;
    arr = nullptr;
}

void read_data2()
{
    int r_cnt = 3;
    byte_t *arr = new byte_t[r_cnt];

    while (true)
    {
        logi("[th2] get %d byte_t.", r_cnt);
        if (buf->get(arr, r_cnt))
        {
            if (buf->empty())
            {
                logw("[th2] after get data, ring buf is empty.");
            }
            logd("[th2] ring buf size = %d, free_size = %d",
                 buf->size(), buf->free_size());
        }
        else
        {
            logi("[th2] th2 don't get data.");
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    delete[] arr;
    arr = nullptr;
}

int main()
{
    std::thread write_thread(write_data);
    write_thread.detach();
    usleep(50 * 1000);

    std::thread read_thread1(read_data1);
    read_thread1.detach();
    usleep(50 * 1000);

    std::thread read_thread2(read_data2);
    read_thread2.detach();

    while (true)
        ;

    return 0;
}
