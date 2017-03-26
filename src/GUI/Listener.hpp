#pragma once

class ResizeListener {
public:
    virtual ~ResizeListener() = default;
    virtual void onResize(int width, int height) = 0;
};