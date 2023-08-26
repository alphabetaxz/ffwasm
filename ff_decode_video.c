#include<stdio.h>
#include<libavutil/avutil.h>
#include<libavcodec/avcodec.h>

typedef void(*VideoCallback)(unsigned char* data_y,
                            unsigned char* data_u,
                            unsigned char* data_v,
                            int line1,
                            int line2,
                            int line3,
                            int width,
                            int height,
                            long pts);

typedef struct _DecoderContext {
    const AVCodec *pCodec;
    AVCodecContext *pCodecCtx;

    AVPacket *pkt;
    AVFrame *frame;

    AVCodecParser *pParser;
    AVCodecParserContext *pParaerCtx;

    VideoCallback cb;
} DecoderContext;

typedef enum _CodecType {
    CODEC_TYPE_H264 = 0,
    CODEC_TYPE_H265
} CodecType;

typedef enum _ErrorCode {
    ERROR_CODE_OK = 0,
    ERROR_CODE_NOMEMORY = -1,
    ERROR_CODEC_NOSUPPORT = -2
} ErrorCode;

DecoderContext *pDecoderCtx = NULL;

int init_decoder_ctx(DecoderContext *ctx) {
    if(!ctx) {
        return -1;
    }
    ctx->pCodec = NULL;
    ctx->pCodecCtx=NULL;
    ctx->pkt = NULL;
    ctx->frame = NULL;
    ctx->pParser = NULL;
    ctx->pParaerCtx = NULL;
    ctx->cb = NULL;
    return 0;
}


int open_decoder(int type, long callback) {
    printf("open decoder ...\n");
    CodecType ct = (CodecType)type;

    ErrorCode ec = ERROR_CODE_OK;
    pDecoderCtx = (DecoderContext*)malloc(sizeof(DecoderContext));
    if (!pDecoderCtx) {
        ec = ERROR_CODE_NOMEMORY;
        goto __ERROR;
    }
    init_decoder_ctx(pDecoderCtx);

    // 1. find decoder
    if(ct == CODEC_TYPE_H264) {
        pDecoderCtx->pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    } else if (ct == CODEC_TYPE_H265) {
        pDecoderCtx->pCodec = avcodec_find_decoder(AV_CODEC_ID_H265);
    } else {
        ec = ERROR_CODEC_NOSUPPORT;
        goto __ERROR;
    }
    pDecoderCtx->pParaerCtx = av_parser_init(pDecoderCtx->pCodec->id);

    // 2. allocate decoder context
    pDecoderCtx->pCodecCtx = avcodec_alloc_context3(pDecoderCtx->pCodec);
    // if (!pDecoderCtx->pCodecCtx) {
    // }

    // 3. bind context and decoder
    if(avcodec_open2(pDecoderCtx->pCodecCtx, pDecoderCtx->pCodec, NULL) < 0) {
        // 错误处理
    }

    // 4. allocate avpacket
    pDecoderCtx->pkt = av_packet_alloc();

    // 5. allocate avframe
    pDecoderCtx->frame = av_frame_alloc();

    pDecoderCtx->cb = (VideoCallback)callback;

    goto __RET;

__ERROR:
    // release source ...
__RET:
    return 0;
}

int decode() {
    int ret = avcodec_send_packet(pDecoderCtx->pCodecCtx, pDecoderCtx->pkt);
    if (ret < 0) {
        return ret;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(pDecoderCtx->pCodecCtx, pDecoderCtx->frame);
        if(ret < 0) {
            break;
        }
        pDecoderCtx->cb(pDecoderCtx->frame->data[0],
                        pDecoderCtx->frame->data[1],
                        pDecoderCtx->frame->data[2],
                        pDecoderCtx->frame->linesize[0],
                        pDecoderCtx->frame->linesize[1],
                        pDecoderCtx->frame->linesize[2],
                        pDecoderCtx->frame->width,
                        pDecoderCtx->frame->height,
                        pDecoderCtx->frame->pts);
    }
    return ret;
}

int decode_data(unsigned char* data, size_t data_size) {
    if (!pDecoderCtx) {
        goto __ERROR;
    }

    while(data_size > 0) {
        int size = av_parser_parse2(
            pDecoderCtx->pParaerCtx,
            pDecoderCtx->pCodecCtx,
            &pDecoderCtx->pkt->data,
            &pDecoderCtx->pkt->size,
            data,
            data_size,
            AV_NOPTS_VALUE,
            AV_NOPTS_VALUE,
            0);
        if (size < 0) {
            break;
        }
        // data 指针向后移动
        data += size;
        data_size -= size;

        if(pDecoderCtx->pkt->size) {
            int t = decode();
            if (t < 0) {
                if (t == AVERROR(EAGAIN) || t == AVERROR(AVERROR_EOF)) {
                    continue;
                } else {
                    break;
                }
            }
        }
    }

__ERROR:

__RET:
    return 0;
}

int main(int argc, char const *argv[])
{
    // open_decoder();
    // decode_data();
    printf("hello world!\n");
    return 0;
}
