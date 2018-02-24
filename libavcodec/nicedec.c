/**
* Author: Aaron Bellis and  Anastasia Gonzalez
* UID: u0981638 and u0985898
* Date: 2/23/18
* Class: 3505
* Purpose:
* How to run: 
*/
#include <inttypes.h>

#include "avcodec.h"
#include "bytestream.h"
#include "nice.h"
#include "internal.h"
#include "msrledec.h"

static int nice_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data; // store data as an AVFrame
    unsigned int fsize, hsize;
    int width, height;
    unsigned int depth;
    BiCompression comp;
    unsigned int ihsize;
    int i, j, n, linesize, ret;
    uint32_t rgb[3] = {0};
    uint32_t alpha = 0;
    uint8_t *ptr;
    int dsize;
    const uint8_t *buf0 = buf;
    GetByteContext gb;

    // Nice file format contains NICE then the width and height as integers
    // necessary buffer size must be at least 12 bytes
    if (buf_size < 12) 
    {
        av_log(avctx, AV_LOG_ERROR, "buf size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }

    // Check to make sure our file has correct starting header
    if (bytestream_get_byte(&buf) != 'N' ||
        bytestream_get_byte(&buf) != 'I' ||
        bytestream_get_byte(&buf) != 'C' ||
        bytestream_get_byte(&buf) != 'E' ) 
    {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }


    // read 32 bits and set as the width
    width  = bytestream_get_le32(&buf);
    // read 32 bits and set as the height
    height = bytestream_get_le32(&buf);

    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }
        
    avctx->pix_fmt = AV_PIX_FMT_RGB8;


    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret; 
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;


           
    // our own copy loop for each pixel
    // nice format is represented as top to bottom!
    ptr = p->data[0];
    for(i = 0; i < avctx->height; i++) 
    {
      for (int j = 0; j < avctx->width; j++)
      {
        // places current pixel inside file and advances destination buf
        bytestream_put_byte(&ptr, buf[0]); 
        buf++; // advance the pointer to new data
      }
    }       

    *got_frame = 1;

    return buf_size;
}

AVCodec ff_nice_decoder = {
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("NICE image (a project for CS 3505)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .decode         = nice_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .pix_fmts       = (const enum AVPixelFormat[]){ AV_PIX_FMT_RGB8, AV_PIX_FMT_NONE },
};
