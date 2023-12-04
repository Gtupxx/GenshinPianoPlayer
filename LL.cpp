/*
 * @Author: 柚岩龙蜥
 * @Date: 2023-11-20 12:44:45
 * @LastEditors: 柚岩龙蜥
 * @LastEditTime: 2023-11-20 12:48:13
 * @FilePath: \ccpp\MIDI\LL.cpp
 * @Description:
 *
 */
#include <iostream>
#include <fstream>

int main()
{
    std::string name = "flowerdance";

    std::fstream f;
    std::string song = "./song/";
    f.open(song + name + "/melody.txt");
    int num = 0;
    std::cin >> num;
    for (int i = 0; i < num; i++)
    {
        f << "LL" << '\t' << i + 1 << std::endl;
    }
    return 0;
}