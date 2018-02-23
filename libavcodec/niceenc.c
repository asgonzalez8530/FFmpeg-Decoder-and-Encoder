/*
 * BMP image format encoder
 * Copyright (c) 2006, 2007 Michel Bardiaux
 * Copyright (c) 2009 Daniel Verkamp <daniel at drv.nu>
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

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "nice.h"
#include "internal.h"

static const uint32_t monoblack_pal[] = { 0x000000, 0xFFFFFF };
static const uint32_t rgb565_masks[]  = { 0xF800, 0x07E0, 0x001F };
static const uint32_t rgb444_masks[]  = { 0x0F00, 0x00F0, 0x000F };

static av_cold int nice_encode_init(AVCodecContext *avctx)
{    
    /* there is only one answer to this question may remove in future */
    avctx->bits_per_coded_sample = 8;
    return 0;
}

static int nice_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    const AVFrame * const p = pict;
    int n_bytes_image, n_bytes_per_row, n_bytes, i, n, hsize, ret;
    const uint32_t *pal = NULL;
    uint32_t palette256[256];
    int pad_bytes_per_row, pal_entries = 0, compression = BMP_RGB;
    int bit_count = avctx->bits_per_coded_sample;
    uint8_t *ptr, *buf;

/* Don't think we need this
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->coded_frame->key_frame = 1;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
*/
    /*switch (avctx->pix_fmt) {
    case AV_PIX_FMT_RGB444:
        compression = BMP_BITFIELDS;
        pal = rgb444_masks; // abuse pal to hold color masks
        pal_entries = 3;
        break;
    case AV_PIX_FMT_RGB565:
        compression = BMP_BITFIELDS;
        pal = rgb565_masks; // abuse pal to hold color masks
        pal_entries = 3;
        break;
    case AV_PIX_FMT_RGB8:*/
    /*case AV_PIX_FMT_BGR8:
    case AV_PIX_FMT_RGB4_BYTE:
    case AV_PIX_FMT_BGR4_BYTE:
    case AV_PIX_FMT_GRAY8: */
        av_assert1(bit_count == 8);
        avpriv_set_systematic_pal2(palette256, AV_PIX_FMT_RGB8);
        pal = palette256;
        /*break;
        
    case AV_PIX_FMT_PAL8:
        pal = (uint32_t *)p->data[1];
        break;
    case AV_PIX_FMT_MONOBLACK:
        pal = monoblack_pal;
        break;
    }*/
    
    // MAY NEED TO BRING BACK
    //if (pal && !pal_entries) pal_entries = 1 << bit_count;
    //n_bytes_per_row = ((int64_t)avctx->width * (int64_t)bit_count + 7LL) >> 3LL;
    //pad_bytes_per_row = (4 - n_bytes_per_row) & 3;
    //n_bytes_image = avctx->height * (n_bytes_per_row + pad_bytes_per_row);

    // STRUCTURE.field refer to the MSVC documentation for BITMAPFILEHEADER
    // and related pages.


    hsize = 12;
    n_bytes = n_bytes_image + hsize;
    if ((ret = ff_alloc_packet2(avctx, pkt, n_bytes, 0)) < 0)
        return ret;
    buf = pkt->data;
    // Write NICE to file as part of header
    bytestream_put_byte(&buf, 'N');                   
    bytestream_put_byte(&buf, 'I');
    bytestream_put_byte(&buf, 'C'); 
    bytestream_put_byte(&buf, 'E');
    // Write width and height to file                   
    bytestream_put_le32(&buf, avctx->width);      
    bytestream_put_le32(&buf, avctx->height);         
    
    /* Used to put pallet in file, not using pallet
    for (i = 0; i < pal_entries; i++)
        bytestream_put_le32(&buf, pal[i] & 0xFFFFFF); */
    
    
    // BMP files are bottom-to-top so we start from the end...
    ptr = p->data[0] + (avctx->height - 1) * p->linesize[0];
    buf = pkt->data + hsize;
    for(i = 0; i < avctx->height; i++) 
    {
        
        memcpy(buf, ptr, n_bytes_per_row);
        
        buf += n_bytes_per_row;
        memset(buf, 0, pad_bytes_per_row);
        buf += pad_bytes_per_row;
        ptr -= p->linesize[0]; // ... and go back
    }

    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

AVCodec ff_nice_encoder = {
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("NICE image (a project for CS 3505)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .init           = nice_encode_init,
    .encode2        = nice_encode_frame,
    .pix_fmts       = (const enum AVPixelFormat[]){ AV_PIX_FMT_RGB8 },
};
