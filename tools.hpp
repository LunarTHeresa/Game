#pragma once
#include<easyx.h>

void drawImg(int x, int y, IMAGE *src)
{
    // 变量初始化
    DWORD* pwin = GetImageBuffer();         // 窗口缓冲区指针
    DWORD* psrc = GetImageBuffer(src);     // 图片缓冲区指针
    int win_w = getwidth();
    int win_h = getheight();
    int src_w = src->getwidth();
    int src_h = src->getheight();

    // 计算贴图的实际长宽
    int real_w = (x + src_w > win_w) ? win_w - x : src_w;
    int real_h = (y + src_h > win_h) ? win_h - y : src_h;
    if (x < 0) { psrc += -x; real_w -= -x; x = 0; }
    if (y < 0) { psrc += (src_w * -y); real_h -= -y; y = 0; }

    // 修正贴图起始位置
    pwin += (win_w * y + x);

    // 透明/混合贴图
    for (int iy = 0; iy < real_h; iy++)
    {
        for (int ix = 0; ix < real_w; ix++)
        {
            DWORD s = psrc[ix];
            BYTE a = (BYTE)(s >> 24); // 源 alpha (0..255)
            if (a == 0) continue;     // 完全透明跳过
            if (a == 255)             // 完全不透明直接覆盖
            {
                pwin[ix] = s;
                continue;
            }
            // 半透明：标准 alpha 混合
            BYTE sr = (BYTE)(s >> 16);
            BYTE sg = (BYTE)(s >> 8);
            BYTE sb = (BYTE)(s);
            DWORD d = pwin[ix];
            BYTE dr = (BYTE)(d >> 16);
            BYTE dg = (BYTE)(d >> 8);
            BYTE db = (BYTE)(d);

            int invA = 255 - a;
            BYTE rr = (BYTE)((sr * a + dr * invA + 127) / 255);
            BYTE rg = (BYTE)((sg * a + dg * invA + 127) / 255);
            BYTE rb = (BYTE)((sb * a + db * invA + 127) / 255);

            pwin[ix] = (0xFFu << 24) | (rr << 16) | (rg << 8) | rb;
        }
        // 换到下一行
        pwin += win_w;
        psrc += src_w;
    }
}

void drawImg(int x, int y, int dstW, int dstH, IMAGE* src, int srcX, int srcY)
{
	// 变量初始化
	DWORD* pwin = GetImageBuffer();			//窗口缓冲区指针
	DWORD* psrc = GetImageBuffer(src);		//图片缓冲区指针
	int win_w = getwidth();				//窗口宽高
	int win_h = getheight();
	int src_w = src->getwidth();				//图片宽高
	int src_h = src->getheight();


	// 计算贴图的实际长宽
	int real_w = (x + dstW > win_w) ? win_w - x : dstW;			// 处理超出右边界
	int real_h = (y + dstH > win_h) ? win_h - y : dstH;			// 处理超出下边界
	if (x < 0) { psrc += -x;			real_w -= -x;	x = 0; }	// 处理超出左边界
	if (y < 0) { psrc += (dstW * -y);	real_h -= -y;	y = 0; }	// 处理超出上边界

	//printf("realw,h(%d,%d)\n", real_w, real_h);
	// 修正贴图起始位置
	pwin += (win_w * y + x);

	// 实现透明贴图
	for (int iy = 0; iy < real_h; iy++)
	{
		for (int ix = 0; ix < real_w; ix++)
		{
			byte a = (byte)(psrc[ix + srcX + srcY * src_w] >> 24);//计算透明通道的值[0,256) 0为完全透明 255为完全不透明
			if (a > 100)
			{
				pwin[ix] = psrc[ix + srcX + srcY * src_w];
			}
		}
		//换到下一行
		pwin += win_w;
		psrc += src_w;
	}
}
