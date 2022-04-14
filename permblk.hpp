#ifndef PERMBLK_HPP
#define PERMBLK_HPP
#pragma once
#include <vector>

// 容许段，左闭右开区间
template <typename _SType = unsigned long long>
struct permblk
{
    // 开始处（含）
    _SType sta;
    // 结束处（斥）
    _SType end;

    permblk(_SType sta, _SType end) : 
    sta(sta), end(end) { }

    _SType len()
    {
        return end - sta;
    }

    bool is_includes(permblk &a)
    {
        return a.sta >= sta && a.end <= end;
    }

    bool is_overlap(permblk &a)
    {
        return (a.sta >= sta && a.sta <= end) || (sta >= a.sta && sta <= a.end);
    }

    int merge(permblk &a)
    {
        if(!is_overlap(a)) return -1;
        if(sta > a.sta) sta = a.sta;
        if(end < a.end) end = a.end;
        return 0;
    }

    // 擦除一段。必须确保a是重叠而不包含的。
    int erase(permblk &a)
    {
        if(is_includes(a)) return -1;
        if(!is_overlap(a)) return -1;

        if(a.end < end && a.end > sta)
        {
            sta = a.end;
        }
        else
        {
            end = a.sta;
        }

        return 0;
    }
};

template <typename _SType = unsigned long long>
class permblkman
{
private:
    std::vector<permblk<_SType> > blocks;
public:
    permblkman() {}
    ~permblkman() {}

    void mkblk(_SType sta, _SType end)
    {
        permblk<_SType> t(sta, end);
        for(permblk<_SType> i : blocks)
        {
            if(i.is_overlap(t))
            {
                i.merge(t);
                return;
            }
        }
        blocks.push_back(t);
    }

    void rmblk(_SType sta, _SType end)
    {
        permblk<_SType> t(sta, end);
        for(permblk<_SType> i : blocks)
        {
            if(i.is_includes(t))
            {
                permblk<_SType> x(end, i.end);
                blocks.push_back(x);
            }
            if(i.is_overlap(t))
            {
                i.erase(t);
            }
            if(i.len == 0) blocks.erase(i);
        }
    }
};

#endif