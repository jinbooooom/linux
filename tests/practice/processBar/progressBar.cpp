#include <iomanip>
#include <iostream>

void ProgressBar(int progress, int total)
{
    float percentage = static_cast<float>(progress) / total * 100;
    int barWidth     = 100;

    std::cout << "[";

    int pos = barWidth * percentage / 100;
    for (int i = 0; i < barWidth; ++i)
    {
        if (i <= pos)
        {
            std::cout << "=";
        }
        else
        {
            std::cout << " ";
        }
    }

    // \r
    // 可以将光标移动到当前行的起始位置，从而实现在终端上覆盖之前输出的效果。这样可以使得进度条的更新看起来更加平滑和连续
    std::cout << "] " << std::fixed << std::setprecision(2) << percentage << "%\r";

    if (total <= progress)
    {
        std::cout << std::endl;
    }
    std::cout.flush();
}

int main()
{
    int total = 99;
    for (int i = 0; i <= total; i++)
    {
        ProgressBar(i, total);
        // 模拟处理时间
        // 建议使用实际任务的逻辑来替代这里的延迟，例如文件的读写或计算等
        for (int j = 0; j < 10000000; ++j)
        {}
    }

    return 0;
}
