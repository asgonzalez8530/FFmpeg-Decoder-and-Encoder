/*
 * BMP image format decoder
 * Copyright (c) 2005 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: entered nice_decode_frame *** \n");
    
    
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
    
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: Check that we at least have header size *** \n");

    // Nice file format contains NICE then the width and height as integers
    // necessary buffer size must be at least 12 bytes
    if (buf_size < 12) 
    {
        av_log(avctx, AV_LOG_ERROR, "buf size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }

    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: Check Header Begins with NICE *** \n");
    // Check to make sure our file has correct starting header
    if (bytestream_get_byte(&buf) != 'N' ||
        bytestream_get_byte(&buf) != 'I' ||
        bytestream_get_byte(&buf) != 'C' ||
        bytestream_get_byte(&buf) != 'E' ) 
    {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }

    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: Header begins with NICE *** \n");
    
    
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: read width and height from header *** \n");
    // read 32 bits and set as the width
    width  = bytestream_get_le32(&buf);
    // read 32 bits and set as the height
    height = bytestream_get_le32(&buf);


    
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: Set dimentions *** \n");
    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }
    
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: Set Format to RGB8 *** \n");
    
    avctx->pix_fmt = AV_PIX_FMT_RGB8;

    
    
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: Get Buffer *** \n");
    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret; 
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;


    
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: begin read image bits *** \n");        
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
            
    // used to print info to the console for debugging 
    av_log(NULL, AV_LOG_INFO, "\n *** IN NICEDEC.C: complete read image bits *** \n");       

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
